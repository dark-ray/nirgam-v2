
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

//////////////////////////////////
/// constructor for input channel
/////////////////////////////////
template<UI num_op>
InputChannel<num_op>::InputChannel(sc_module_name InputChannel): sc_module(InputChannel), rr_arbiter_route("RR_R"), rr_arbiter_transmit("RR_T") {

    sim_count = 0;
    
	// process sensitive to inport event, reads in flit and stores in buffer
	SC_THREAD(read_flit);
	sensitive << inport;

	// transmit flit at the front of fifo to output port at each clock cycle
	SC_THREAD(transmit_flit);
	sensitive << switch_cntrl.pos() << vcReady << arbReady_t;

	// route flit at the front of fifo if required
	SC_THREAD(route_flit);
	sensitive << switch_cntrl.pos() << rtReady << arbReady_r;
    
    //processing sim_count variable (simulation ticks) and output stress value
    SC_METHOD(processIntLogic);
    sensitive << switch_cntrl.pos();
    
	// initialize VC request to false
	vcRequest.initialize(false);

	// initialize route request to NONE
	rtRequest.initialize(NONE);
    
    // arbiters
    arbRequest_r.write(false);
    arbRequest_t.write(false);
    
    for(UI i = 0; i < NUM_VCS; i++) {
        req_r[i].write(false);
        req_t[i].write(false);
    }
    
    for(UI i = 0; i < NUM_VCS; i++) 
        rr_arbiter_route.req[i](req_r[i]);
    rr_arbiter_route.arbRequest(arbRequest_r);
    rr_arbiter_route.grant(grant_r);
    rr_arbiter_route.arbReady(arbReady_r);
    
    for(UI i = 0; i < NUM_VCS; i++) 
        rr_arbiter_transmit.req[i](req_t[i]);
    rr_arbiter_transmit.arbRequest(arbRequest_t);
    rr_arbiter_transmit.grant(grant_t);
    rr_arbiter_transmit.arbReady(arbReady_t);
    
    // initialize stress value of IC to 0
    stress_value_int_out.initialize(0);

	// initialize virtual channels and buffers
	for(UI i=0; i < NUM_VCS ; i++) {
		vc[i].vc_route = 5;
		vc[i].vc_next_id = NUM_VCS + 1;
	}
    
    //init timewaits
    for(UI i = 0; i < NUM_VCS; i++) {
        timewait_r[i] = 0;
        timewait_t[i] = 0;
        served_r[i] = false;
        not_empty_r[i] = false;
        not_empty_t[i] = false;
    }

	// reset buffer counts to zero
	resetCounts();

	// Send initial credit info (buffer status)
    creditLine temp_cl; temp_cl.freeVC = true; temp_cl.freeBuf = true;
	for (UI i = 0; i < NUM_VCS ; i++)
		credit_out[i].initialize(temp_cl);
    
    // Send initial congestion flags info
    congestion_flag.initialize(false);
    
    // Initial congestion statuses
    for (UI i = 0; i < NUM_VCS; i++)
        congestion_status_out[i].initialize(false);
    
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
			for(UI i = 0; i < NUM_VCS; i++) {
				numBufsOcc += vc[i].vcQ.pntr;
				if(vc[i].vc_next_id != NUM_VCS+1) numVCOcc++;
                
                //update rr_arbiter_route reqs
                if(vc[i].vcQ.empty)
                    req_r[i].write(false);
                else
                    req_r[i].write(true);
                    
                //update congestion statuses
                if (vc[i].vcQ.pntr > CONGESTION_LEVEL)
                    congestion_status_out[i].write(true);
                else
                    congestion_status_out[i].write(false);
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
			ULL vc_to_serve;
			if(cntrlID == C)	// assuming only 1 VC at IchannelC
				vc_to_serve = 0;
			else {
                switch (IAT_TYPE) {         // selection depends on input arbitry type
                    case SEQUENCE: vc_to_serve = (sim_count-1) % NUM_VCS; break;	// serving VCs in sequence manner
                    case RR:  {                                                     // real round-robin manner
                            arbRequest_r.write(true);
                            if(LOG >= 4)
                                eventlog<<"\ntime: "<<sc_time_stamp()<<" name: "<<this->name()<<" IC: arbRequest_r sent!"<<endl;
                            wait();
                            if(arbReady_r.event()) {
                                if(LOG >= 4)
                                    eventlog<<"\ntime: "<<sc_time_stamp()<<" name: "<<this->name()<<" IC: arbReady_r event..."<<endl;
                            }
                            else if(switch_cntrl.event()) {
                                if(LOG >= 4)
                                    eventlog<<"\ntime: "<<sc_time_stamp()<<" name: "<<this->name()<<" IC: unknown clock event..."<<endl;
                            }
                            vc_to_serve = grant_r.read();
                            arbRequest_r.write(false);
                            if(LOG >= 4)
                                eventlog<<"\ntime: "<<sc_time_stamp()<<" name: "<<this->name()<<" IC: vc_to_serve = "<<vc_to_serve<<endl;
                        }; break;                                                   
                    case AA:  {                                                     // adapting arbitry
                            for (UI i = 0; i < NUM_VCS; i++)  // increased wait times for serving
                                if (!vc[i].vcQ.empty) {
                                    not_empty_r[i] = true;
                                    if (!served_r[i])
                                        timewait_r[i]++;
                                    else
                                        timewait_r[i] = 0;
                                }
                                else
                                    not_empty_r[i] = false;
                                    
                            vc_to_serve = 0;
                            
                            for (UI i = 1; i < NUM_VCS; i++) {     // process all VCs
                                if (not_empty_r[i] && !served_r[i] && (!not_empty_r[vc_to_serve] || served_r[vc_to_serve]))
                                    vc_to_serve = i;     // only 1 VC has flit
                                    
                                if (not_empty_r[i] && !served_r[i] && not_empty_r[vc_to_serve] && !served_r[vc_to_serve]) { // at least 2 VC has flit
                                    UI hopcount_1 = 0;
                                    UI hopcount_2 = 0;
                                    
                                    if (HOP_USE) {   // use hop statistics to selection
                                        hopcount_1 = vc[i].vcQ.flit_read(true).pkthdr.nochdr.hopcount;
                                        hopcount_2 = vc[vc_to_serve].vcQ.flit_read(true).pkthdr.nochdr.hopcount;
                                        if (hopcount_1 <= HOP_LEVEL)
                                        hopcount_1 = 0;
                                        else
                                            hopcount_1 = hopcount_1 - HOP_LEVEL;
                                        if (hopcount_2 <= HOP_LEVEL)
                                            hopcount_2 = 0;
                                        else
                                            hopcount_2 = hopcount_2 - HOP_LEVEL;
                                    }
                                    
                                    if ((timewait_r[i] + hopcount_1) >  (timewait_r[vc_to_serve] + hopcount_2)) // selection
                                        vc_to_serve = i;
                                }
                            }
                            
                            if(LOG >= 4)
                                eventlog<<"\ntime: "<<sc_time_stamp()<<" name: "<<this->name()<<" IC:r: vc_to_serve = "<<vc_to_serve<<endl;
                        }; break; 
                    default:  
                        eventlog<<"\ntime: "<<sc_time_stamp()<<" name: "<<this->name()<<" Error in input arbitry!"<<endl;
                        break;
                }            
            }
			
			if(vc[vc_to_serve].vc_route == 5) {	// route not set, require routing
				if(vc[vc_to_serve].vcQ.empty == false) {	// fifo not empty
					// read flit at front of fifo
					flit_out = vc[vc_to_serve].vcQ.flit_read();
                    
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
                    
                    served_r[vc_to_serve] = true;
					
					if(LOG >= 2)
						eventlog<<"\ntime: "<<sc_time_stamp()<<" name: "<<this->name()<<" tile: "<<tileID<<" cntrl: "<<cntrlID<<" Routing flit: "<<flit_out;
				}
			}
            
            // update rr_arbiter_transmit reqs
            for(UI i = 0; i < NUM_VCS; i++) {
                if(vc[i].vc_route == 5)
                    req_t[i].write(false);
                else
                    req_t[i].write(true);
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
			// find buffers and VCs occupied
			ULL vc_to_serve;
            
            if(cntrlID == C)	// assuming only 1 VC at IchannelC
				vc_to_serve = 0;
			else  {
                switch (IAT_TYPE) {
                    case SEQUENCE: vc_to_serve = (sim_count-1) % NUM_VCS; break; // sequence manner
                    case RR: {                                                   // round-robin manner
                            arbRequest_t.write(true);
                            if(LOG >= 4)
                                eventlog<<"\ntime: "<<sc_time_stamp()<<" name: "<<this->name()<<" IC: arbRequest_t sent!"<<endl;
                            wait();
                            if(arbReady_t.event()) {
                                if(LOG >= 4)
                                    eventlog<<"\ntime: "<<sc_time_stamp()<<" name: "<<this->name()<<" IC: arbReady_t event..."<<endl;
                            }
                            else if(switch_cntrl.event()) {
                                if(LOG >= 4)
                                    eventlog<<"\ntime: "<<sc_time_stamp()<<" name: "<<this->name()<<" IC: unknown clock event..."<<endl;
                            }
                            vc_to_serve = grant_t.read();
                            arbRequest_t.write(false);
                            if(LOG >= 4)
                                eventlog<<"\ntime: "<<sc_time_stamp()<<" name: "<<this->name()<<" IC: vc_to_serve = "<<vc_to_serve<<endl;
                        }; break;
                    case AA:  {                                                 // adaptive arbitry
                            for (UI i = 0; i < NUM_VCS; i++)                    // increase wait times of all VCs
                                if (!vc[i].vcQ.empty) {
                                    not_empty_t[i] = true;
                                    timewait_t[i]++;
                                }
                                else
                                    not_empty_t[i] = false;
                                    
                            vc_to_serve = 0;
                            
                            for (UI i = 1; i < NUM_VCS; i++) {                  // for all VCs
                                if (not_empty_t[i] && !not_empty_t[vc_to_serve])  // only 1 VC has flit
                                    vc_to_serve = i;
                                    
                                if (not_empty_t[i] && not_empty_t[vc_to_serve])  {   // 2 or more VCs has flit
                                    UI hopcount_1 = 0;
                                    UI hopcount_2 = 0;
                                    
                                    if (HOP_USE) {                                  // use hop staticstics
                                        hopcount_1= vc[i].vcQ.flit_read(true).pkthdr.nochdr.hopcount;
                                        hopcount_2 = vc[vc_to_serve].vcQ.flit_read(true).pkthdr.nochdr.hopcount;
                                        if (hopcount_1 <= HOP_LEVEL)
                                            hopcount_1 = 0;
                                        else
                                            hopcount_1 = hopcount_1 - HOP_LEVEL;
                                        if (hopcount_2 <= HOP_LEVEL)
                                            hopcount_2 = 0;
                                        else
                                            hopcount_2 = hopcount_2 - HOP_LEVEL;
                                    }
                                    
                                    if ((timewait_t[i] + hopcount_1) > (timewait_t[vc_to_serve] + hopcount_2)) // selection
                                        vc_to_serve = i;
                                }
                            }
                            
                            if(LOG >= 4)
                                eventlog<<"\ntime: "<<sc_time_stamp()<<" name: "<<this->name()<<" IC:t: vc_to_serve = "<<vc_to_serve<<endl;
                        }; break; 
                    default:  
                        eventlog<<"\ntime: "<<sc_time_stamp()<<" name: "<<this->name()<<" Error in input arbitry!"<<endl;
                        break;
                }            
            }
            
            timewait_t[vc_to_serve] = 0;
            
			if(vc[vc_to_serve].vc_route == 5) {	// routing decision pending, before transmission
				inc_vcs_num_waits();
				continue;
			}
			
			// Routing decision has been made, proceed to transmission
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
					flit_out = vc[vc_to_serve].vcQ.flit_read();	// read flit from fifo
					numBufReads++;		// increase buffer read count

					if(LOG >= 4) {
						eventlog<<"time: "<<sc_time_stamp()<<" name: "<<this->name()<<" IC: Attempting to forward  flit: "<<flit_out<<" To: "<<i<<" num_op-1: "<<(num_op -1)<<endl;
                        eventlog<<"time: "<<sc_time_stamp()<<" name: "<<this->name()<<" IC: vc["<<vc_to_serve<<"].vc_next_id: "<<vc[vc_to_serve].vc_next_id;
                    }
					
					if(i != num_op - 1) {	// not to be done for core OC
						if( vc[vc_to_serve].vc_next_id == NUM_VCS + 1 ) { //&& cntrlID != C ) {
							if(flit_out.pkttype == NOC && (flit_out.pkthdr.nochdr.flittype == DATA || flit_out.pkthdr.nochdr.flittype == TAIL)) {
								//should have been a head, need to clean out the fifo Q
								if(LOG >= 0)
									eventlog<<"\ntime: "<<sc_time_stamp()<<" name: "<<this->name()<<" IC: flit is not a head..Error"<<endl;
								vc[vc_to_serve].vcQ.pntr = 0;
								vc[vc_to_serve].vcQ.empty = true;
								vc[vc_to_serve].vcQ.full = false;
                                served_r[vc_to_serve] = false; 
								inc_vcs_num_waits();
								continue;
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
                                
                                if(vc[vc_to_serve].vc_next_id == NUM_VCS + 1) {	// VC not granted
                                    if(LOG >= 4) 
                                        eventlog<<"\ntime: "<<sc_time_stamp()<<" name: "<<this->name()<<" IC: No free next vc, pushing flit in Q" <<endl;
                                    // push flit back in fifo
                                    vcRequest.write(false);
									inc_vcs_num_waits();
                                    continue;
                                }
                            }
                        }
					} // end if

                    //flit ready for send to OC
					flit_out = vc[vc_to_serve].vcQ.flit_out();	// read flit from fifo
					if(i != num_op - 1) {	// not to be done for core OC
						flit_out.vcid = vc[vc_to_serve].vc_next_id;                       
					}
                    
                    if(flit_out.pkttype == NOC && (flit_out.pkthdr.nochdr.flittype == HEAD || flit_out.pkthdr.nochdr.flittype == HDT))
                        flit_out.pkthdr.nochdr.flithdr.header.rtfi = vc[vc_to_serve].new_rfi;
        
                     //Stress value update - outgoing
                    stress_value--;
                    if(LOG >= 4)
                        eventlog<<"\ntime: "<<sc_time_stamp()<<" name: "<<this->name()<<" tile: "<<tileID<<" cntrlID: "<<cntrlID <<" Stress dec: "<<stress_value<<endl;
                    
					// write flit to output port
					flit_out.simdata.num_sw++;
					flit_out.simdata.ctime = sc_time_stamp();
					outport[i].write(flit_out);
					
					if(LOG >= 2)
						eventlog<<"\ntime: "<<sc_time_stamp()<<" name: "<<this->name()<<" tile: "<<tileID<<" cntrlID: "<<cntrlID<<" Transmitting flit to output port: "<<i<<" "<<flit_out<<endl;

					//if hdt/tail flit, put freeVC signal on creditLine
					//if head/data flit, if fifo buf is free, then put freeBuf signal creditLine
					// Update credit info
					if(LOG >= 4)
						eventlog<<"\ntime: "<<sc_time_stamp()<<" name: "<<this->name()<<" IC: Updating freeVC status for vcid: "<<vc_to_serve<<endl;
					
					// if hdt/tail flit, free VC
					// else free buffer
					if((flit_out.pkttype == NOC && (flit_out.pkthdr.nochdr.flittype == TAIL || flit_out.pkthdr.nochdr.flittype == HDT)) || flit_out.pkttype == ANT) {
						vc[vc_to_serve].vc_next_id = NUM_VCS + 1;
						creditLine t; t.freeVC = true; t.freeBuf = true;
						credit_out[vc_to_serve].write(t);
						//if(cntrlID == C)
                        vc[vc_to_serve].vc_route = 5;
                        //flit served!
                        served_r[vc_to_serve] = false; 
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
			inc_vcs_num_waits();
		} //end if switch_cntrl
	} // end while
} //end transmit_flit()

///////////////////////////////////////////////////////////////////////////
/// Method to assign tile IDs and port IDs
/// \param id tile ID
/// \param port_N id corresponding to North direction
/// \param port_S id corresponding to South direction
/// \param port_E id corresponding to East direction
/// \param port_W id corresponding to West direction
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
/// Method to resut buffer statistics to zero
///////////////////////////////////////////////////////////////////////////
template<UI num_op>
void InputChannel<num_op>::resetCounts(){
	numBufReads = 0;
    numBufWrites = 0;
	numBufsOcc = 0;
	numVCOcc = 0;
}

///////////////////////////////////////////////////////////////////////////
/// Method to store flit in fifo buffer of VC
/// \param flit_in input flit to VC
///////////////////////////////////////////////////////////////////////////
template<UI num_op>
void InputChannel<num_op>::store_flit_VC(flit *flit_in) {
	UI vc_id = flit_in->vcid;
	if(LOG >= 4)
		eventlog<<"\ntime: "<<sc_time_stamp()<<" name: "<<this->name()<<" Buffer: "<<vc[vc_id].vcQ.pntr;
	if(vc[vc_id].vcQ.full == true) {
		if(LOG >= 1)
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
/// \param flit_in HDT/HEAD flit to route
///////////////////////////////////////////////////////////////////////////
template<UI num_op>
void InputChannel<num_op>::routing_src(flit *flit_in) {
	UI vc_id = flit_in->vcid;
	rtRequest.write(ROUTE);
	sourceAddress.write(flit_in->src);
	destRequest.write(flit_in->pkthdr.nochdr.flithdr.header.rthdr.sourcehdr.route);
    faultInfoOut.write(flit_in->pkthdr.nochdr.flithdr.header.rtfi);
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
    vc[vc_id].new_rfi  = faultInfoIn.read();
	rtRequest.write(NONE);
}

///////////////////////////////////////////////////////////////////////////////////////
/// Method to call controller for routing algorithms that require destination address
/// \param flit_in HDT/HEAD flit to route
//////////////////////////////////////////////////////////////////////////////////////
template<UI num_op>
void InputChannel<num_op>::routing_dst(flit *flit_in) {
	UI vc_id = flit_in->vcid;
	rtRequest.write(ROUTE);
	sourceAddress.write(flit_in->src);
	destRequest.write(flit_in->pkthdr.nochdr.flithdr.header.rthdr.dsthdr.dst);
    faultInfoOut.write(flit_in->pkthdr.nochdr.flithdr.header.rtfi);
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
    vc[vc_id].new_rfi  = faultInfoIn.read();
	rtRequest.write(NONE);
}

///////////////////////////////////////////////////////////////////////////
/// Method to track clocks count and update router's stress value
///////////////////////////////////////////////////////////////////////////
template<UI num_op>
void InputChannel<num_op>::processIntLogic(){
   sim_count++;
   stress_value_int_out.write(stress_value);
}

///////////////////////////////////////////////////////////////////////////
/// Method to increment number of waited clocks of all stored flits in VCs
///////////////////////////////////////////////////////////////////////////
template<UI num_op>
void InputChannel<num_op>::inc_vcs_num_waits(){
   for (UI i = 0; i < NUM_VCS; i++)
		vc[i].vcQ.inc_flits_num_waits();
}

//////////////////////////////////////////////////
// Ant routines

///////////////////////////////////////////////////
/// Route table lookup on basis of interconnects
/// \param rt_code routing direction
/// \return reverse routing direction
template<UI num_op>
UI InputChannel<num_op>::reverse_route(UI rt_code) {
	UI rt_rev;
	switch(rt_code) {
		case 0: rt_rev = 1; break;
		case 1: rt_rev = 0; break;
		case 2: rt_rev = 3; break;
		case 3: rt_rev = 2; break;
	};
	return rt_rev;
}

// Ant routines end
////////////////////////////////////////////////////

template struct InputChannel<NUM_OC>;
template struct InputChannel<NUM_OC_B>;
template struct InputChannel<NUM_OC_C>;
