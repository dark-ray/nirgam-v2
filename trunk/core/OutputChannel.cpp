
/*
 * OutputChannel.cpp
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
/// \file OutputChannel.cpp
/// \brief Implements module OutputChannel
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#include "OutputChannel.h"
#include "../config/extern.h"

////////////////////////
/// constructor
////////////////////////
template<UI num_ip>
OutputChannel<num_ip>::OutputChannel(sc_module_name OutputChannel): sc_module(OutputChannel), rr_arbiter_out("RR_O") {

    sim_count = 0;
    isFail = false;
    
	// process to read and process incoming flits
	SC_THREAD(entry);
	for(UI i = 0; i < num_ip; i++)
		sensitive << inport[i]; 
	sensitive << switch_cntrl.pos() << arbReady_o;
    
    SC_METHOD(processSimCount);
    sensitive << switch_cntrl.pos();

	// initialize ready signal to true
	for(UI i = 0; i < num_ip; i++)
		inReady[i].initialize(true);
        
    //init wait times and states for r_in
    for(UI i = 0; i < num_ip; i++)
        timewait_rin[i] = 0;
    
    //init wait times and states for r_vc
    for(UI i = 0; i < NUM_VCS; i++) {
        not_empty_rvc[i] = false;
        timewait_rvc[i] = 0;
    }
    
    // arbiter out
    arbRequest_o.write(false);
    
    for(UI i = 0; i < NUM_VCS; i++) 
        req_o[i].write(false);
    
    for(UI i = 0; i < NUM_VCS; i++) 
        rr_arbiter_out.req[i](req_o[i]);
    rr_arbiter_out.arbRequest(arbRequest_o);
    rr_arbiter_out.grant(grant_o);
    rr_arbiter_out.arbReady(arbReady_o);
    
	// initialize performance stats to zero
	latency             = 0;
	num_pkts            = 0;
	num_flits           = 0;
	avg_latency         = 0.0;
	avg_latency_flit    = 0.0;
	avg_throughput      = 0.0;
	
	beg_cycle           = 0;
	end_cycle           = 0;
	total_cycles        = 0;
}

///////////////////////////////////////////////////////////////////////////
/// Process sensitive to inport event and clock event
/// - clock event:
///   - send flit from register r_vc to output port
///   - move any waiting flits from register r_in to r_vc
///   .
/// - inport event:
///   - read flit from inport and store in register r_in
///   .
///////////////////////////////////////////////////////////////////////////
template<UI num_ip>
void OutputChannel<num_ip>::entry() {
    UI cur_vc = 0;
	while(sim_count < SIM_NUM) {
		wait();
		if(switch_cntrl.event() && !isFail) {	// clock event

            for(UI i = 0; i < NUM_VCS; i++) {
                //update rr_arbiter_out reqs
                if(r_vc[i].free)
                    req_o[i].write(false);
                else
                    req_o[i].write(true);
			}
            
            switch (IAT_TYPE) {       // output arbitration type
                case SEQUENCE: cur_vc = (sim_count+NUM_VCS)%NUM_VCS; break; // sequence alike
                case RR: {                                                  // round-robin
                    arbRequest_o.write(true);
                    if(LOG >= 6)
                        eventlog<<"\ntime: "<<sc_time_stamp()<<" name: "<<this->name()<<" OC: arbRequest_o sent!"<<endl;
                    wait();
                    if(arbReady_o.event()) {
                        if(LOG >= 6)
                            eventlog<<"\ntime: "<<sc_time_stamp()<<" name: "<<this->name()<<" OC: arbReady_r event..."<<endl;
                    }
                    else if(switch_cntrl.event()) {
                        if(LOG >= 4)
                            eventlog<<"\ntime: "<<sc_time_stamp()<<" name: "<<this->name()<<" OC: unknown clock event..."<<endl;
                    }
                    cur_vc = grant_o.read();
                    arbRequest_o.write(false);
                }; break;
                case AA:  {                       // adaptive arbitration
                        UI choise = 0;
                        
                        for(UI i = 0; i < NUM_VCS; i++) {  // increase wait times of all stored flits
                            if (!r_vc[i].free) {
                                not_empty_rvc[i] = true;
                                timewait_rvc[i]++;
                            }
                            else
                                not_empty_rvc[i] = false;
                        }
                        
                        for (UI i = 1; i < NUM_VCS; i++) {  // for all VCs
                            if (not_empty_rvc[i] && !not_empty_rvc[choise]) // only 1 VC has flit
                                choise = i;
                            
                            if (not_empty_rvc[i] && not_empty_rvc[choise]) { // more than 2 VC has flit
                                UI hopcount_1 = 0;
                                UI hopcount_2 = 0;
                                
                                if (HOP_USE) {      // use hop statistics
                                    hopcount_1 = r_vc[i].val.pkthdr.nochdr.hopcount;
                                    hopcount_2 = r_vc[choise].val.pkthdr.nochdr.hopcount;
                                    if (hopcount_1 <= HOP_LEVEL)
                                        hopcount_1 = 0;
                                    else
                                        hopcount_1 = hopcount_1 - HOP_LEVEL;
                                    if (hopcount_2 <= HOP_LEVEL)
                                        hopcount_2 = 0;
                                    else
                                        hopcount_2 = hopcount_2 - HOP_LEVEL;
                                }
                                
                                if (CONGESTION_USE) {   // use congestion status
                                    if (!congestion_status_in[i])
                                        hopcount_1 = hopcount_1 + CONGESTION_PRIORITY;
                                    if (!congestion_status_in[choise])
                                        hopcount_2 = hopcount_2 + CONGESTION_PRIORITY;
                                }
                                    
                                if ((timewait_rvc[i] + hopcount_1) > (timewait_rvc[choise] + hopcount_2)) // selection
                                    choise = i;
                            }
                        }
                        
                        timewait_rvc[choise] = 0;
                        cur_vc = choise;
                         
                    }; break; 
                default:  
                        eventlog<<"\ntime: "<<sc_time_stamp()<<" name: "<<this->name()<<" Not set input arbitry!"<<endl;
                        break;
            }
            if(LOG >= 6)
                eventlog<<"\ntime: "<<sc_time_stamp()<<" name: "<<this->name()<<" OC: cur_vc = "<<cur_vc<<endl;
            
           	if(!r_vc[cur_vc].free) {	// flit in register r_vc
            
				// local channel, send flit from r_vc to outport, no need to check credit info
				if(cntrlID == C) {
					
					r_vc[cur_vc].val.simdata.ctime = sc_time_stamp();
                    
                    //updates hop counts
                    r_vc[cur_vc].val.pkthdr.nochdr.hopcount++;
                    
					outport.write(r_vc[cur_vc].val);
					r_vc[cur_vc].free = true;
					
					if(r_vc[cur_vc].val.pkthdr.nochdr.flittype == TAIL || r_vc[cur_vc].val.pkthdr.nochdr.flittype == HDT) {
						latency += sim_count - 1 - input_time[cur_vc];
						num_pkts++;
						end_cycle = sim_count - 1;
					}
					num_flits++;
					
					if(LOG >= 2)
						eventlog<<"Time: "<<sc_time_stamp()<<" name: "<<this->name()<<" tile: "<<tileID<<" cntrlID: "<<cntrlID<<" Sending out flit from OC "<<r_vc[cur_vc].val;
					
				}
				else {	// send flit to outport on basis of credit info, if free space in buf at IC of next tile
					if(credit_in[r_vc[cur_vc].val.vcid].read().freeBuf) {

						r_vc[cur_vc].val.simdata.ctime = sc_time_stamp();
                        
                        //updates hop counts
                        r_vc[cur_vc].val.pkthdr.nochdr.hopcount++;
                
						outport.write(r_vc[cur_vc].val);
						
						if(r_vc[cur_vc].val.pkthdr.nochdr.flittype == TAIL || r_vc[cur_vc].val.pkthdr.nochdr.flittype == HDT) {
							latency += sim_count - 1 - input_time[cur_vc];
							num_pkts++;
                            if(LOG >= 6) {
                                eventlog<<"Time: "<<sc_time_stamp()<<" name: "<<this->name()<<" tile: "<<tileID<<" cntrlID: "<<cntrlID<<" Calc VCid: "<<cur_vc<<endl;
							    eventlog<<"!!Values +lat: "<<(sim_count - 1 - input_time[cur_vc])<<" st_time: "<<input_time[cur_vc]<<" cur_time: "<<sim_count<<endl;
                            }
                            end_cycle = sim_count - 1;
						}
						num_flits++;
							
						if(LOG >= 2)
							eventlog<<"Time: "<<sc_time_stamp()<<" name: "<<this->name()<<" tile: "<<tileID<<" cntrlID: "<<cntrlID<<" Buf at next tile is free, Sending out flit from OC "<<r_vc[cur_vc].val;
						
						r_vc[cur_vc].free = true;
					}
					else {
						if(LOG >= 4)
							eventlog<<"Time: "<<sc_time_stamp()<<" name: "<<this->name()<<" tile: "<<tileID<<" cntrlID: "<<cntrlID<<" Buf at next tile is not free for VC "<<cur_vc<<endl;
					}
				}
			}

			// if any r_vc got free, move any waiting flits from r_in to r_vc
            switch (IAT_TYPE) {   // output arbitry type 
                case SEQUENCE: case RR: {
                    for(UI i = 0; i < num_ip; i++) {
                        if(!r_in[i].free) {
                            if(r_vc[r_in[i].val.vcid].free) {
                                r_vc[r_in[i].val.vcid].val = r_in[i].val;
                                if(r_in[i].val.pkthdr.nochdr.flittype == HDT || r_in[i].val.pkthdr.nochdr.flittype == HEAD)
                                    input_time[r_in[i].val.vcid] = r_in[i].val.simdata.ICtimestamp;
                            
                                r_vc[r_in[i].val.vcid].free = false;
                                r_in[i].free = true;
                                inReady[i].write(true);
                            }
                        }
                    }
                }; break;
                case AA: {         // only adaptive different
                    //BEGIN LONG LOGIC
                    UI r_in_choise      = -1;
                    UI r_in_choise_vcid = -1;
                    for (UI i = 0; i < num_ip; i++) { // increase wait times at all input regs
                        if (!r_in[i].free) {
                            timewait_rin[i]++;
                            if ((r_vc[r_in[i].val.vcid].free) && (r_in_choise == -1))
                                r_in_choise = i;
                        }
                    }
                               
                    if (r_in_choise != -1) {
                        r_in_choise_vcid = r_in[r_in_choise].val.vcid;
                        for (UI i = 0; i < num_ip; i++) {
                            if (r_in[i].free || (i == r_in_choise))     //free slot or our choise
                                continue;
                                
                            if (!r_vc[r_in[i].val.vcid].free)           //not free VC
                                continue;
                               
                            if (r_in[i].val.vcid != r_in_choise_vcid) { //don't compete with our choise
                                r_vc[r_in[i].val.vcid].val = r_in[i].val;
                                if(r_in[i].val.pkthdr.nochdr.flittype == HDT || r_in[i].val.pkthdr.nochdr.flittype == HEAD)
                                    input_time[r_in[i].val.vcid] = r_in[i].val.simdata.ICtimestamp;
                            
                                r_vc[r_in[i].val.vcid].free = false;
                                r_in[i].free = true;
                                inReady[i].write(true);
                                continue;
                            }
                           
                            // compete with our choise
                            UI hopcount_1 = 0;
                            UI hopcount_2 = 0;
                            
                            if (HOP_USE) {  // use hop statistics
                                hopcount_1 = r_in[i].val.pkthdr.nochdr.hopcount;
                                hopcount_2 = r_in[r_in_choise].val.pkthdr.nochdr.hopcount;
                                if (hopcount_1 <= HOP_LEVEL)
                                hopcount_1 = 0;
                                else
                                    hopcount_1 = hopcount_1 - HOP_LEVEL;
                                if (hopcount_2 <= HOP_LEVEL)
                                    hopcount_2 = 0;
                                else
                                    hopcount_2 = hopcount_2 - HOP_LEVEL;
                            }
                                
                            if (CONGESTION_USE) { // use congestion status
                                if (!congestion_status_in[r_in[i].val.vcid])
                                    hopcount_1 = hopcount_1 + CONGESTION_PRIORITY;
                                if (!congestion_status_in[r_in[r_in_choise].val.vcid])
                                    hopcount_2 = hopcount_2 + CONGESTION_PRIORITY;
                            }
                            
                            if ((timewait_rin[i] + hopcount_1) > (timewait_rin[r_in_choise] + hopcount_2)) // selection
                                r_in_choise = i;
                                
                        }
                        timewait_rin[r_in_choise] = 0;
                        r_vc[r_in[r_in_choise].val.vcid].val = r_in[r_in_choise].val;
                        if(r_in[r_in_choise].val.pkthdr.nochdr.flittype == HDT || r_in[r_in_choise].val.pkthdr.nochdr.flittype == HEAD)
                            input_time[r_in[r_in_choise].val.vcid] = r_in[r_in_choise].val.simdata.ICtimestamp;
                    
                        r_vc[r_in[r_in_choise].val.vcid].free = false;
                        r_in[r_in_choise].free = true;
                        inReady[r_in_choise].write(true);
                    } 
                    //END LONG LOGIC
                }; break;
                default:  
                        eventlog<<"\ntime: "<<sc_time_stamp()<<" name: "<<this->name()<<" Not set input arbitry!"<<endl;
                        break;
            }
           
		} //end if switch_cntrl

		// inport event, store the flit in corresponding register
		for(UI i = 0; i < num_ip; i++) {
			if(inport[i].event() && !isFail) {
				r_in[i].val = inport[i].read();
				r_in[i].free = false;
					
				if(beg_cycle == 0)
					beg_cycle = r_in[i].val.simdata.ICtimestamp;

				if(LOG >= 4)
					eventlog<<"Time: "<<sc_time_stamp()<<" name: "<<this->name()<<" tile: "<<tileID<<" cntrlID: "<<cntrlID<<" Recvd flit at port "<<i<<": "<<r_in[i].val<<endl;
				
				// if r_vc is not full, write r_in into it, else set ready signal to false
				if(!r_vc[r_in[i].val.vcid].free || !WRITE_THROUGH_OUTPORT)
					inReady[i].write(false);
				else {
					r_vc[r_in[i].val.vcid].val = r_in[i].val;
					if(r_in[i].val.pkthdr.nochdr.flittype == HDT || r_in[i].val.pkthdr.nochdr.flittype == HEAD)
						input_time[r_in[i].val.vcid] = r_in[i].val.simdata.ICtimestamp;
					
					r_vc[r_in[i].val.vcid].free = false;
					r_in[i].free = true;
					UI vc_id = r_in[i].val.vcid;
					if(LOG >= 4)
						eventlog<<"Time: "<<sc_time_stamp()<<" name: "<<this->name()<<" tile: "<<tileID<<" cntrlID: "<<cntrlID<<" VC "<<vc_id<<" is free, putting in flit"<<endl;
					inReady[i].write(true);
				}
			} //end inport event
		}
	} //end while
} //end entry

///////////////////////////////////////////////////////////////////////////
/// Method to assign tile IDs and port IDs
/// \param is tile ID
/// \param port_N id corresponding to North direction
/// \param port_S id corresponding to South direction
/// \param port_E id corresponding to East direction
/// \param port_W id corresponding to West direction
///////////////////////////////////////////////////////////////////////////
template<UI num_ip>
void OutputChannel<num_ip>::setTileID(UI id, UI port_N, UI port_S, UI port_E, UI port_W){
	tileID = id;
	portN = port_N;
	portS = port_S;
	portE = port_E;
	portW = port_W;
}

////////////////////////////////////////////
/// Method to set fail state of output channel
////////////////////////////////////////////
template<UI num_ip>
void OutputChannel<num_ip>::setFail() {
    isFail = true;
    #ifdef DEBUG_NOC
        eventlog<<"!!Output channel fail: "<<tileID<<endl;
    #endif
}

////////////////////////////////////////////
/// Method to set working state of output channel
////////////////////////////////////////////
template<UI num_ip>
void OutputChannel<num_ip>::setWorking() {
    isFail = false;
    #ifdef DEBUG_NOC
        eventlog<<"!!Output channel work: "<<tileID<<endl;
    #endif
}

///////////////////////////////////////////////////////////////////////////
/// Cleaning method
/// - close logfiles
/// - compute preformance stats (latency and throughput)
/// - dump results
///////////////////////////////////////////////////////////////////////////
template<UI num_ip>
void OutputChannel<num_ip>::closeLogs(){
	if(num_pkts != 0)
		avg_latency = (double)latency/num_pkts;
	if(num_flits != 0)
		avg_latency_flit = (double)latency/num_flits;
	total_cycles = end_cycle - beg_cycle;
	if(total_cycles != 0)
		avg_throughput = (double)(num_flits * FLITSIZE * 8) / (total_cycles * CLK_PERIOD);	// Gbps
	//cout<<tileID<<" || "<<beg_cycle<<" "<<end_cycle<<" "<<num_flits<<" || "<<sim_count<<" || "<<latency<<endl;
	if(cntrlID != C) {
		results_log<<tileID<<"\t";
		switch(cntrlID) {
			case N: results_log<<"North\t\t"; break;
			case S: results_log<<"South\t\t"; break;
			case E: results_log<<"East\t\t"; break;
			case W: results_log<<"West\t\t"; break;
		}
		results_log<<num_pkts<<"\t\t"<<num_flits<<"\t\t"<<avg_latency<<"\t\t"<<avg_latency_flit<<"\t\t"<<avg_throughput<<endl;
	}
}

///////////////////////////////////////////////////////////////////////////
/// Method to process track of clock cycles
///////////////////////////////////////////////////////////////////////////
template<UI num_ip>
void OutputChannel<num_ip>::processSimCount() {
	sim_count++;
}

template struct OutputChannel<NUM_IC>;
template struct OutputChannel<NUM_IC_B>;
template struct OutputChannel<NUM_IC_C>;
