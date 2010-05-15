
/*
 * InputChannel.cpp
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License,
 * version 2, as published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License along
 * with this program; if not, write to the Free Software Foundation, Inc.,
 * 59 Temple Place, Suite 330, Boston, MA 02111-1307, USA.
 *
 * Author: Lavina Jain
 *
 */
/*
 * Portions of changes by Alexander Rumyanthev (darkstreamray@gmail.com) SPbSU ITMO 2010.
 */
  
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/// \file InputChannel.cpp
/// \brief Implements module InputChannel (reads and processes incoming flits)
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#include "InputChannel.h"
#include "../config/extern.h"

////////////////////////
/// Constructor
////////////////////////
template<UI num_op>
InputChannel<num_op>::InputChannel(sc_module_name InputChannel): sc_module(InputChannel) {

    sim_count = 0;
    
	// process sensitive to inport event, reads in flit and stores in buffer
	SC_THREAD(read_flit);
	sensitive << inport;

	// transmit flit at the front of fifo to output port at each clock cycle
	SC_THREAD(transmit_flit);
	sensitive << switch_cntrl.pos() << vcReady;

	// route flit at the front of fifo if required
	SC_THREAD(route_flit);
	sensitive << switch_cntrl.pos() << rtReady;
    
    //processing sim_count variable (simulation ticks) and output stress value
    SC_METHOD(processIntLogic);
    sensitive << switch_cntrl.pos();
	
	// initialize VC request to false
	vcRequest.initialize(false);

	// initialize route request to NONE
	rtRequest.initialize(NONE);
    
    // initialize stress value of IC to 0
    stress_value_int_out.initialize(0);

	// initialize virtual channels and buffers
	for(UI i=0; i < NUM_VCS ; i++) {
		/*vc[i].vcQ.num_bufs = NUM_BUFS;
		vc[i].vcQ.pntr = 0;
		vc[i].vcQ.full = false;
		vc[i].vcQ.empty = true;*/
		vc[i].vc_route = 5;
		vc[i].vc_next_id = NUM_VCS + 1;
	}

	// reset buffer counts to zero
	resetCounts();

	// Send initial credit info (buffer status)
    creditLine temp_cl; temp_cl.freeVC = true; temp_cl.freeBuf = true;
	for(int i=0; i < NUM_VCS ; i++){
		credit_out[i].initialize(temp_cl);
	}
    
    // Send initial congestion flags info
    congestion_flag.initialize(false);
    
    // Initial stress value
    stress_value = 0;
}

///////////////////////////////////////////////////////////////////////////
/// Process sensitive to inport event
/// Reads flit from input port and calls function to store in buffer
///////////////////////////////////////////////////////////////////////////
template<UI num_op>
void InputChannel<num_op> :: read_flit() {
	//flit that is read into the input channel
	flit flit_in;
	while(true) {
		wait();
        // inport event
		if(inport.event()) {
			
			flit_in = inport.read();	// read flit
           
			flit_in.simdata.ICtimestamp = sim_count - 1;	// set input timestamp (required for per channel latency stats)

			if(LOG >= 2)
				eventlog<<"\ntime: "<<sc_time_stamp()<<" name: "<<this->name()<<" tile: "<<tileID<<" cntrl: "<<cntrlID <<" Inport event!"<<" flitID: "<<flit_in;
			
			switch(flit_in.pkttype) {

			case ANT:
				break;

			case NOC:
				store_flit_VC(&flit_in);	// store flit in buffer
				break;
	
			} // end switch pkt type
			// find buffers and VCs occupied
			numBufsOcc = 0; numVCOcc = 0;
			for(int i = 0; i < NUM_VCS; i++) {
				numBufsOcc += vc[i].vcQ.pntr;
				if(vc[i].vc_next_id != NUM_VCS+1) numVCOcc++;
			}
            //congestion flags update
            bool cong_flag = false;
            if (numBufsOcc > HALF_NUM_BUFS)
                cong_flag = true;
            else
                cong_flag = false;
            congestion_flag.write(cong_flag);
            if(LOG >= 4)
                eventlog<<"\ntime: "<<sc_time_stamp()<<" name: "<<this->name()<<" tile: "<<tileID<<" cntrl: "<<cntrlID <<" Congestion flag: "<<cong_flag<<endl;
                    
		} // end if inport
	} // end while
} //end entry()

///////////////////////////////////////////////////////////////////////////
/// Process sensitive to clock
/// Calls routing functions if head/hdt flit at the front of fifo
///////////////////////////////////////////////////////////////////////////
template<UI num_op>
void InputChannel<num_op> :: route_flit() {
	while(sim_count < SIM_NUM) {
		wait();		// wait for next clock cycle
		flit flit_out;
		if(switch_cntrl.event()) {
			//sim_count++;
			
			ULL vc_to_serve;
			if(cntrlID == C)	// assuming only 1 VC at IchannelC
				vc_to_serve = 0;
			else
				vc_to_serve = (sim_count-1) % NUM_VCS;	// serving VCs in round robin manner

			
			if(vc[vc_to_serve].vc_route == 5) {	// route not set, require routing
				if(vc[vc_to_serve].vcQ.empty == false) {	// fifo not empty
					// read flit at front of fifo
					//flit_out = vc[vc_to_serve].vcQ.flit_out();
					flit_out = vc[vc_to_serve].vcQ.flit_read();
					//numBufReads++;

					// call routing function depending on type of routing algorithm
					switch(flit_out.pkttype) {
						case NOC: if(flit_out.pkthdr.nochdr.flittype == HEAD || flit_out.pkthdr.nochdr.flittype == HDT) {
							routing_type rt = flit_out.pkthdr.nochdr.flithdr.header.rtalgo;
							routing_hdr *rt_hdr = &(flit_out.pkthdr.nochdr.flithdr.header.rthdr);

							if(rt == SOURCE) {
								routing_src(&flit_out);
							}
							else {
								routing_dst(&flit_out);
							}
						}
						break;
						case ANT:
							break;
					}
					
					// push back flit in fifo
					//vc[vc_to_serve].vcQ.flit_push(flit_out);
					
					if(LOG >= 2)
						eventlog<<"\ntime: "<<sc_time_stamp()<<" name: "<<this->name()<<" tile: "<<tileID<<" cntrl: "<<cntrlID<<" Routing flit: "<<flit_out;
				}
			}
		}
	}
}

///////////////////////////////////////////////////////////////////////////
/// Process sensitive to clock
/// Transmits flit at front of fifo to OC
/// - If head/hdt flit, send VC request
/// - write flit to output port if ready signal from OC
///////////////////////////////////////////////////////////////////////////
template<UI num_op>
void InputChannel<num_op> :: transmit_flit() {
	while(sim_count < SIM_NUM) {
		wait();		// wait for next clock cycle
		flit flit_out;
		if(switch_cntrl.event()) {	// clock event
			//sim_count++;

			// find buffers and VCs occupied
/*			numBufsOcc = 0; numVCOcc = 0;
			for(int i = 0; i < NUM_VCS; i++) {
				numBufsOcc += vc[i].vcQ.pntr;
				if(vc[i].vc_next_id != NUM_VCS+1) numVCOcc++;
			}
*/
			// serve a VC in round robin manner
			ULL vc_to_serve;
			if(cntrlID == C)	// assuming only 1 VC at IchannelC
				vc_to_serve = 0;
			else
				vc_to_serve = (sim_count-1) % NUM_VCS;

			if(vc[vc_to_serve].vc_route == 5)	// routing decision pending, before transmission
				continue;
			
			// Routing decision has been made, proceed to transmission
			int oldvcid;

			UI i;
			switch(TOPO) {
				
			case TORUS:
				i = vc[vc_to_serve].vc_route;
				break;
	
			case MESH:
				UI dir = vc[vc_to_serve].vc_route;
				switch(dir) {
					case N: i = portN;
						break;
					case S: i = portS;
						break;
					case E: i = portE;
						break;
					case W: i = portW;
						break;
					case C: i = num_op - 1;
						break;
                    default:                   // Error in routing
                        i = num_op - 1;
                        eventlog<<"\ntime: "<<sc_time_stamp()<<" name: "<<this->name()<<" Error in routing algo! Get direction to route = "<<dir<<endl;
                        break;
				}
				break;
			}

			if(vc[vc_to_serve].vcQ.empty == false) {	// fifo not empty
				if(outReady[i].read() == true) {	// OC ready to recieve flit
					//flit_out = vc[vc_to_serve].vcQ.flit_out();	// read flit from fifo
					flit_out = vc[vc_to_serve].vcQ.flit_read();	// read flit from fifo
					numBufReads++;		// increase buffer read count

					oldvcid = flit_out.vcid;

					if(LOG >= 4)
						eventlog<<"\ntime: "<<sc_time_stamp()<<" name: "<<this->name()<<" IC: Attempting to forward  flit: "<<flit_out;
					
					if(i != num_op - 1) {	// not to be done for core OC
						if( vc[vc_to_serve].vc_next_id == NUM_VCS+1 && cntrlID != C) {
							if(flit_out.pkttype == NOC && (flit_out.pkthdr.nochdr.flittype == DATA || flit_out.pkthdr.nochdr.flittype == TAIL)) {
								//should have been a head, need to clean out the fifo Q
								if(LOG >= 4)
									eventlog<<"\ntime: "<<sc_time_stamp()<<" name: "<<this->name()<<" IC: flit is not a head..Error"<<endl;
								vc[vc_to_serve].vcQ.pntr = 0;
								vc[vc_to_serve].vcQ.empty = true;
								vc[vc_to_serve].vcQ.full = false;
								continue;
							}
						}

						if( (flit_out.pkttype == NOC && (flit_out.pkthdr.nochdr.flittype == HEAD || flit_out.pkthdr.nochdr.flittype == HDT)) || (flit_out.pkttype == ANT && flit_out.pkthdr.anthdr.anttype == FORWARD)) {
							
							// VC request
							if(LOG >= 4)
								eventlog<<"\ntime: "<<sc_time_stamp()<<" name: "<<this->name()<<" tile: "<<tileID<<" cntrlID: "<<cntrlID<<" vcRequest: "<<i;

							vcRequest.write(true);
							opRequest.write(i);

							wait();	// wait for ready event from VC
							if(vcReady.event()) {
								if(LOG >= 4)
									eventlog<<"\ntime: "<<sc_time_stamp()<<" name: "<<this->name()<<" IC: vcReady event..."<<endl;
							}
							else if(switch_cntrl.event()) {
								if(LOG >= 4)
									eventlog<<"\ntime: "<<sc_time_stamp()<<" name: "<<this->name()<<" IC: unknown clock event..."<<endl;
							}

							// read next VCid sent by VC
							vc[vc_to_serve].vc_next_id = nextVCID.read();
							
							if(vc[vc_to_serve].vc_next_id == NUM_VCS+1) {	// VC not granted
								if(LOG >= 4) 
									eventlog<<"\ntime: "<<sc_time_stamp()<<" name: "<<this->name()<<" IC: No free next vc, pushing flit in Q" <<endl;
								// push flit back in fifo
								//flit_out.simdata.num_waits++;
								//vc[vc_to_serve].vcQ.flit_push(flit_out);
								vcRequest.write(false);
								continue;
							}
						}

						oldvcid = flit_out.vcid;
					} // end if, this block not executed for core OC

					flit_out = vc[vc_to_serve].vcQ.flit_out();	// read flit from fifo
					if(i != num_op - 1) {	// not to be done for core OC
						flit_out.vcid = vc[vc_to_serve].vc_next_id;                       
					}
                    
                     //Stress value update - outgoing
                    stress_value--;
                    if(LOG >= 4)
                        eventlog<<"\ntime: "<<sc_time_stamp()<<" name: "<<this->name()<<" tile: "<<tileID<<" cntrl: "<<cntrlID <<" Stress dec: "<<stress_value<<endl;
                    
					// write flit to output port
					flit_out.simdata.num_sw++;
					flit_out.simdata.ctime = sc_time_stamp();
					outport[i].write(flit_out);
					
					if(LOG >= 2)
						eventlog<<"\ntime: "<<sc_time_stamp()<<" name: "<<this->name()<<" tile: "<<tileID<<" cntrl: "<<cntrlID<<" Transmitting flit to output port: "<<i<<" "<<flit_out<<endl;

					//if hdt/tail flit, put freeVC signal on creditLine
					//if head/data flit, if fifo buf is free, then put freeBuf signal creditLine
					// Update credit info
					if(LOG >= 4)
						eventlog<<"\ntime: "<<sc_time_stamp()<<" name: "<<this->name()<<" IC: Updating freeVC status for vc:"<<vc_to_serve<<endl;
					
					// if hdt/tail flit, free VC
					// else free buffer
					if((flit_out.pkttype == NOC && (flit_out.pkthdr.nochdr.flittype == TAIL || flit_out.pkthdr.nochdr.flittype == HDT)) || flit_out.pkttype == ANT) {
						vc[oldvcid].vc_next_id = NUM_VCS+1;
						creditLine t; t.freeVC = true; t.freeBuf = true;
						credit_out[vc_to_serve].write(t);
						//if(cntrlID == C)
							vc[vc_to_serve].vc_route = 5;
					}
					else {
						creditLine t; t.freeVC = false; t.freeBuf = true;
						credit_out[vc_to_serve].write(t);
					}
					vcRequest.write(false);
				} // end outReady == true
				else {
					if(LOG >= 4)
						eventlog<<"\ntime: "<<sc_time_stamp()<<" name: "<<this->name()<<" IC: OC cannot accept flit!"<<endl;
				}
			} // end serving VC
		} //end if switch_cntrl
	} // end while
} //end transmit_flit()

///////////////////////////////////////////////////////////////////////////
/// Method to assign tile IDs and port IDs
///////////////////////////////////////////////////////////////////////////
template<UI num_op>
void InputChannel<num_op>::setTileID(UI id, UI port_N, UI port_S, UI port_E, UI port_W) {
	tileID = id;
	portN = port_N;
	portS = port_S;
	portE = port_E;
	portW = port_W;
	resetCounts();
}

///////////////////////////////////////////////////////////////////////////
/// Method to resut buffer stats to zero
///////////////////////////////////////////////////////////////////////////
template<UI num_op>
void InputChannel<num_op>::resetCounts(){
	numBufReads = numBufWrites = 0;
	numBufsOcc = 0;
	numVCOcc = 0;
}

///////////////////////////////////////////////////////////////////////////
/// Method to store flit in fifo buffer
///////////////////////////////////////////////////////////////////////////
template<UI num_op>
void InputChannel<num_op>::store_flit_VC(flit *flit_in) {
	int vc_id = flit_in->vcid;
	if(LOG >= 4)
		eventlog<<"\ntime: "<<sc_time_stamp()<<" name: "<<this->name()<<" Buffer: "<<vc[vc_id].vcQ.pntr;
	if(vc[vc_id].vcQ.full == true) {
		if(LOG >= 4)
			eventlog<<"\ntime: "<<sc_time_stamp()<<" name: "<<this->name()<<" Error: DATA has arrived. vcQ is full!"<<endl;
	}
	else {
		vc[vc_id].vcQ.flit_in(*flit_in);
		numBufWrites++;
		creditLine t;
		t.freeVC = false; t.freeBuf = !vc[vc_id].vcQ.full;
		credit_out[vc_id].write(t);

        //Stress value update - incoming
        stress_value++;
        if(LOG >= 4)
            eventlog<<"\ntime: "<<sc_time_stamp()<<" name: "<<this->name()<<" tile: "<<tileID<<" cntrl: "<<cntrlID <<" Stress inc: "<<stress_value<<endl;
	}
}

///////////////////////////////////////////////////////////////////////////
/// Method to call Controller for source routing
///////////////////////////////////////////////////////////////////////////
template<UI num_op>
void InputChannel<num_op>::routing_src(flit *flit_in) {
	int vc_id = flit_in->vcid;
	rtRequest.write(ROUTE);
	sourceAddress.write(flit_in->src);
	destRequest.write(flit_in->pkthdr.nochdr.flithdr.header.rthdr.sourcehdr.route);
 	flit_in->pkthdr.nochdr.flithdr.header.rthdr.sourcehdr.route = flit_in->pkthdr.nochdr.flithdr.header.rthdr.sourcehdr.route >> 3; //Right shift
	if(LOG >= 4)
		eventlog<<"\ntime: "<<sc_time_stamp()<<" name: "<<this->name()<<" IC: rtRequest sent!"<<endl;
	wait();
	if(rtReady.event()) {
		if(LOG >= 4)
			eventlog<<"\ntime: "<<sc_time_stamp()<<" name: "<<this->name()<<" IC: rtReady event..."<<endl;
	}
	else if(switch_cntrl.event()) {
		if(LOG >= 4)
			eventlog<<"\ntime: "<<sc_time_stamp()<<" name: "<<this->name()<<" IC: unknown clock event..."<<endl;
	}
	else if(LOG >= 4)
		eventlog<<"\ntime: "<<sc_time_stamp()<<" name: "<<this->name()<<" IC: Unknown Event!"<<endl;
	
	vc[vc_id].vc_route = nextRt.read();
	rtRequest.write(NONE);
}

///////////////////////////////////////////////////////////////////////////////////////
/// Method to call controller for routing algorithms that require destination address
//////////////////////////////////////////////////////////////////////////////////////
template<UI num_op>
void InputChannel<num_op>::routing_dst(flit *flit_in) {
	int vc_id = flit_in->vcid;
	rtRequest.write(ROUTE);
	sourceAddress.write(flit_in->src);
	destRequest.write(flit_in->pkthdr.nochdr.flithdr.header.rthdr.dsthdr.dst);
	if(LOG >= 4)
		eventlog<<"\ntime: "<<sc_time_stamp()<<" name: "<<this->name()<<" IC: rtRequest sent!"<<endl;
	wait();
	if(rtReady.event()) {
		if(LOG >= 4)
			eventlog<<"\ntime: "<<sc_time_stamp()<<" name: "<<this->name()<<" IC: rtReady event..."<<endl;
	}
	else if(switch_cntrl.event()) {
		if(LOG >= 4)
			eventlog<<"\ntime: "<<sc_time_stamp()<<" name: "<<this->name()<<" IC: unknown clock event..."<<endl;
	}
	vc[vc_id].vc_route = nextRt.read();
	rtRequest.write(NONE);
}

///////////////////////////////////////////////////////////////////////////
/// Method to process sim_count
///////////////////////////////////////////////////////////////////////////
template<UI num_op>
void InputChannel<num_op>::processIntLogic(){
   sim_count++;
   stress_value_int_out.write(stress_value);
}

//////////////////////////////////////////////////
// Ant routines

// Route table lookup on basis of interconnects
template<UI num_op>
int InputChannel<num_op>::reverse_route(int rt_code) {
	int rt_rev;
	switch(rt_code) {
		case 0: rt_rev = 1; break;
		case 1: rt_rev = 0; break;
		case 2: rt_rev = 3; break;
		case 3: rt_rev = 2; break;
	};
	return rt_rev;
}

// Ant routines end

template struct InputChannel<NUM_OC>;
template struct InputChannel<NUM_OC_B>;
template struct InputChannel<NUM_OC_C>;
