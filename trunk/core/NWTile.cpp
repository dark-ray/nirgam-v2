
/*
 * NWTile.cpp
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
/// \file NWTile.cpp
/// \brief Creates a network tile.
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#include "systemc.h"
#include "NWTile.h"
#include <string>
#include <fstream>
#include "../config/extern.h"

/// array to store library name of application attached to ipcore on each tile
extern string app_libname[MAX_NUM_TILES];

//////////////////////////////////////////////////////////////////////////////////////////////
/// Constructor to create a network tile.
/// Creates and connects the following submodules:
/// - Input Channels
/// - Output Channels
/// - Virtual Channel Allocator
/// - IP Core
/// - Controller
/// .
/// Template parameters:
/// - num_nb: Number of neighbors
/// - num_ic: Number of input channels
/// - num_oc: Number of output channels
///
/// \param NWTile module name
/// \param id unique tile ID
/////////////////////////////////////////////////////////////////////////////////////////////
template <UI num_nb, UI num_ic, UI num_oc>
NWTile<num_nb, num_ic, num_oc>::NWTile(sc_module_name NWTile, UI id): BaseNWTile(NWTile, id), vcAlloc("VA"), ctr("Controller") {
    resetCounts();
    
    //Adaptive routing out ports init
    for(UI i = 0; i < num_nb; i++) {
        stress_value_out[i].initialize(0);
        congestion_flag_out[i].initialize(false);
        
        for (UI j = 0; j < NUM_VCS; j++)
            congestion_status_out[i][j].initialize(false);
    }
    for (UI i = 0; i < NUM_VCS; i++)
        congestion_status_false[i].write(false);
    
	for(UI i = 0; i < num_ic; i++) {
		char name[4];
		sprintf(name, "IC%d", i);
		Ichannel[i] = new InputChannel<num_ic>(name);	// instantiate ICs
    }
    
    for(UI i = 0; i < num_oc; i++) {
        char name[4];
		sprintf(name, "OC%d", i);
		Ochannel[i] = new OutputChannel<num_oc>(name);	// instantiate OCs
	}

	// attach requested library to ipcore
	if(app_libname[id] == "NULL")
		ip = NULL;
	else {
		void *hndl;
		void *mkr;
		string libname = string("./application/lib/");
		libname = libname + app_libname[id];
		hndl = dlopen(libname.c_str(), RTLD_NOW);
		if(hndl == NULL) {
			cerr << dlerror() << endl;
			exit(-1);
		}
		mkr = dlsym(hndl, "maker");
		ip = ((ipcore*(*)())(mkr))();
        
        if (ADDITIONAL_INFO)
            cout<<"Attaching application "<<app_libname[id]<<endl;
	}
	
	// set tile ID and map port numbers to directions
	setID(id);

	// Process sensitive to clock (writes buf utilization info at each clock)
	SC_THREAD(entry);
	sensitive << switch_cntrl.pos();
    
    //
    SC_METHOD(stress_value_out_calc);
    for(UI i = 0; i < num_ic; i++)
        sensitive << stress_value_internal[i].value_changed_event();

	// Interconnections between submodules //////////////////////////////////////////////

	// VCA //////////////////////////////////////////////////////////////////////////////
	vcAlloc.switch_cntrl(*nw_clock);	// clock

	for(UI i = 0; i < num_ic; i++) {
		vcAlloc.vcRequest[i](vcReq[i]);	// VC Request from IC
		vcAlloc.opRequest[i](opReq[i]);	// Request for OC from IC
		vcAlloc.nextVCID[i](nextvc[i]);	// next VCID from VCA to IC
		vcAlloc.vcReady[i](vcReady[i]);	// vcReady signal to IC
		// credit info from IC
		for(UI j = 0; j < NUM_VCS; j++) {
			if(i == num_ic - 1)	{ // Core channel
				vcAlloc.Icredit[i][j](creditIC_CS[j]);
                vcAlloc.congestion_status_in[i][j](congestion_status_false[j]);
            }
			else {
				vcAlloc.Icredit[i][j](credit_in[i][j]);
                vcAlloc.congestion_status_in[i][j](congestion_status_in[i][j]);
            }
		}
	}
	// END VCA //////////////////////////////////////////////////////////////////////////

	// IC ///////////////////////////////////////////////////////////////////////////////
	for(UI i = 0; i < num_ic; i++) {
		Ichannel[i]->switch_cntrl(*nw_clock);	// clock
		
		switch(TOPO) {
		
		case TORUS:
			Ichannel[i]->cntrlID = i;
			break;
	
		case MESH:
			if(i == portN)
				Ichannel[i]->cntrlID = N;
			else if(i == portS)
				Ichannel[i]->cntrlID = S;
			else if(i == portE)
				Ichannel[i]->cntrlID = E;
			else if(i == portW)
				Ichannel[i]->cntrlID = W;
			else if(i == num_ic - 1)
				Ichannel[i]->cntrlID = C;
			break;

		}
		
		if(i == num_ic - 1) 
			Ichannel[i]->inport(flit_CS_IC);	// data input port of core IC
		else
			Ichannel[i]->inport(ip_port[i]);	// data input port of IC

		for(UI j = 0; j < num_oc; j++) {
			Ichannel[i]->outport[j](flit_sig[i][j]); // data output port of IC
			Ichannel[i]->outReady[j](rdy[i][j]);	// ready signal form OC
		}
		Ichannel[i]->vcRequest(vcReq[i]);	// vcRequest to VC
		Ichannel[i]->opRequest(opReq[i]);	// request for o/p to VC
		Ichannel[i]->nextVCID(nextvc[i]);	// next VCID from VC
		Ichannel[i]->vcReady(vcReady[i]);	// ready signal from VC
		Ichannel[i]->rtRequest(rtReq[i]);	// route request to Ctr
		Ichannel[i]->destRequest(destReq[i]);	// dest to Ctr
		Ichannel[i]->sourceAddress(srcAddr[i]);	// source address to Ctr
        Ichannel[i]->faultInfoIn(faultInfoFromCTR[i]);
        Ichannel[i]->faultInfoOut(faultInfoToCTR[i]);
		Ichannel[i]->rtReady(rtReady[i]);	// ready signal from Ctr
		Ichannel[i]->nextRt(nextRt[i]);		// next hop from Ctr
		// send credit info and congestion statuses to OC and VCA
		for(UI j = 0; j < NUM_VCS; j++) {
			if(i == num_ic - 1) {
				Ichannel[i]->credit_out[j](creditIC_CS[j]);	// output credit info to ipcore
                Ichannel[i]->congestion_status_out[j](congestion_status_loop[j]);
            }
			else {
				Ichannel[i]->credit_out[j](credit_out[i][j]);	// output credit info to neighbor tiles
                Ichannel[i]->congestion_status_out[j](congestion_status_out[i][j]);
            }
		}
        
        //stress part
        Ichannel[i]->stress_value_int_out(stress_value_internal[i]);
        
        //congestion part
        if (i != (num_ic - 1))
            Ichannel[i]->congestion_flag(congestion_flag_out[i]);
        else
            Ichannel[i]->congestion_flag(congestion_flag_loop); // Not attach to anywhere!!!
            
	}
	// END IC ////////////////////////////////////////////////////////////////////////////////////

/* example to generate vcdtrace, commented */
/*	if(id == 0) {
		sc_trace(tracefile, Ichannel[num_ic - 1]->inport, "flitin");
		sc_trace(tracefile, Ichannel[num_ic - 1]->vc[0].vcQ.pntr, "size");
		sc_trace(tracefile, Ichannel[num_ic - 1]->vc[0].vcQ.regs[0], "reg0");
		sc_trace(tracefile, Ichannel[num_ic - 1]->vc[0].vcQ.regs[1], "reg1");
		sc_trace(tracefile, Ichannel[num_ic - 1]->vc[0].vcQ.regs[2], "reg2");
		sc_trace(tracefile, Ichannel[num_ic - 1]->vc[0].vcQ.regs[3], "reg3");
		sc_trace(tracefile, Ichannel[num_ic - 1]->vc[0].vcQ.regs[4], "reg4");
		sc_trace(tracefile, Ichannel[num_ic - 1]->vc[0].vcQ.regs[5], "reg5");
		sc_trace(tracefile, Ichannel[num_ic - 1]->vc[0].vcQ.regs[6], "reg6");
	}
*/

	// OC ////////////////////////////////////////////////////////////////////////////////////////
	for(UI i = 0; i < num_oc; i++) {
		Ochannel[i]->switch_cntrl(*nw_clock);	// clock
		
		switch(TOPO) {
			
		case TORUS:
			Ochannel[i]->cntrlID = i;
			break;
	
		case MESH:
			if(i == portN)
				Ochannel[i]->cntrlID = N;
			else if(i == portS)
				Ochannel[i]->cntrlID = S;
			else if(i == portE)
				Ochannel[i]->cntrlID = E;
			else if(i == portW)
				Ochannel[i]->cntrlID = W;
			else if(i == num_oc - 1)
				Ochannel[i]->cntrlID = C;
			break;
		}

		for(UI j = 0; j < num_ic; j++) {
			Ochannel[i]->inport[j](flit_sig[j][i]);	// data (flit) input from IC
			Ochannel[i]->inReady[j](rdy[j][i]);	// ready signal to IC
		}
		if(i == num_oc - 1)
			Ochannel[i]->outport(flit_OC_CR);	// data (flit) output to core
		else
			Ochannel[i]->outport(op_port[i]);	// data (flit) output to output port (connects to neighbor tile)
		// credit info and congestion statuses from IC 
		for(UI j = 0; j < NUM_VCS; j++) {
			if(i == num_oc - 1) {
				Ochannel[i]->credit_in[j](creditIC_CS[j]);	// credit info from core IC
                Ochannel[i]->congestion_status_in[j](congestion_status_false[j]);
            }
			else {
				Ochannel[i]->credit_in[j](credit_in[i][j]);	// credit info from IC
                Ochannel[i]->congestion_status_in[j](congestion_status_in[i][j]);
            }
        }
	}
	// END OC ///////////////////////////////////////////////////////////////////////////////////////////

	// IPcore ///////////////////////////////////////////////////////////////////////////////////////////
	if(app_libname[id] != "NULL") {
		ip->clock(*nw_clock);
	
		// data (flit) input from core OC
		ip->flit_inport(flit_OC_CR);
	
		// data (flit) output to core IC
		ip->flit_outport(flit_CS_IC);
	
		// credit info from core IC
		for(UI i = 0; i < NUM_VCS; i++)
			ip->credit_in[i](creditIC_CS[i]);
	}
	// END IPcore /////////////////////////////////////////////////////////////////////////////////////

	// Controller ///////////////////////////////////////////////////////////////////////////////////////
	// clock
	ctr.switch_cntrl(*nw_clock);

	for(UI i = 0; i < num_ic; i++) {
		ctr.rtRequest[i](rtReq[i]);		// route request from IC
		ctr.destRequest[i](destReq[i]);		// dest address from IC
		ctr.sourceAddress[i](srcAddr[i]);	// source address from IC
        ctr.faultInfoIn[i](faultInfoToCTR[i]);
        ctr.faultInfoOut[i](faultInfoFromCTR[i]);
		ctr.rtReady[i](rtReady[i]);		// ready signal to IC
		ctr.nextRt[i](nextRt[i]);		// next hop to IC
		// credit info from IC
		for(UI j = 0; j < NUM_VCS; j++) {
			if(i == num_ic - 1)
				ctr.Icredit[i][j](creditIC_CS[j]);	// credit info from core IC
			else
				ctr.Icredit[i][j](credit_in[i][j]);	// credit info from IC
        }
    
        if (i != (num_ic - 1)) {
            ctr.stress_value[i](stress_value_in[i]);
            ctr.congestion_flag[i](congestion_flag_in[i]);
        }
    }
	// END Controller ///////////////////////////////////////////////////////////////////////////////////
	//sc_trace(tracefile, rtReady[0], "buf_reg");
} // end constructor

/////////////////////////////////////////
/// Method to reset statistics
////////////////////////////////////////
template <UI num_nb, UI num_ic, UI num_oc>
void NWTile<num_nb, num_ic, num_oc>::resetCounts() {
    totBufsOcc = 0;
    totVCOcc = 0;
    avr_latency_unrouted = 0.0;
    wc_latency_unrouted = 0.0;
    bufUtil = 0.0;
    vcUtil = 0.0;
}

///////////////////////////////////////////////////////////////////////////
/// Process sensitive to clock.
/// Writes buffer utilization info at each clock
///////////////////////////////////////////////////////////////////////////
template <UI num_nb, UI num_ic, UI num_oc>
void NWTile<num_nb, num_ic, num_oc>::entry() {
	while(true) {
		wait();

		//writing out instantaneous buffer utilization
 		ULL totBufReads = 0;
        ULL totBufWrites = 0;
        
		for(UI i = 0; i < num_ic; i++) {
			totBufReads += Ichannel[i]->numBufReads;
			totBufWrites += Ichannel[i]->numBufWrites;
			totBufsOcc += Ichannel[i]->numBufsOcc;
			totVCOcc += Ichannel[i]->numVCOcc;
		}

		if(LOG >= 3) {
			eventlog<<"\ntime: "<<sc_time_stamp()<<" name: "<<this->name()<<" tile: "<<tileID<<" NumBufReads = "<<totBufReads
            <<" NumBufWrites = "<<totBufWrites;
		}
	}
}

///////////////////////////////////////////////////////////////////////////
/// Method to set tile ID
/// \param id tile ID
///
/// - set unique tile ID
/// - map port number to port direction
//////////////////////////////////////////////////////////////////////////
template <UI num_nb, UI num_ic, UI num_oc>
void NWTile<num_nb, num_ic, num_oc>::setID(UI id) {
	tileID = id;
	
	switch(TOPO) {
	
	case TORUS:
		portN = 0;
		portS = 1;
		portE = 2;
		portW = 3;
		break;
		
	case MESH:
		// South border
		if(borderS(id)) {
			portS = ND;
		}
		else if(borderN(id)) {
			portS = 0;
		}
		else portS = 1;

		// East border
		if(borderE(id)) {
			portE = ND;
		}
		else if(borderN(id) || borderS(id)) {
			portE = 1;
		}
		else portE = 2;

		//West border
		if(borderW(id)) {
			portW = ND;
		}
		else if(corner(id)) {
			portW = 1;
		}
		else if(border(id)){
			portW = 2;
		}
		else portW = 3;

		// North border
		if(borderN(id)) {
			portN = ND;
		}
		else portN = 0;
		break;
	}

	// set id and port mapping for ICs
	for(UI i = 0; i < num_ic; i++)
		Ichannel[i]->setTileID(id, portN, portS, portE, portW);

	// set id and port mapping for OCs
	for(UI i = 0; i < num_oc; i++)
		Ochannel[i]->setTileID(id, portN, portS, portE, portW);

	// set id and port mapping for VCA
	vcAlloc.setTileID(id, portN, portS, portE, portW);

	// set id for IP core
	if(app_libname[id] != "NULL")
		ip->setID(id);

	// set id and port mapping for ctr
	ctr.setTileID(id, portN, portS, portE, portW);
}

//////////////////////////////////////////////////////////
/// Closes log files at the end of simulation
/////////////////////////////////////////////////////////
template <UI num_nb, UI num_ic, UI num_oc>
void NWTile<num_nb, num_ic, num_oc>::closeLogs() {
	for(UI i = 0; i < num_oc; i++)
		Ochannel[i]->closeLogs();
    ip->closeLogs();
    bufUtil = (double)totBufsOcc/(NUM_VCS * NUM_BUFS * num_ic *(SIM_NUM - WARMUP));
	vcUtil = (double)totVCOcc/(NUM_VCS * num_ic * (SIM_NUM - WARMUP));
    
    ULL unrouter_flits_number = 0;    // walk through the overall NoC and count unrouted flits
    ULL unrouted_wait_time = 0;
    ULL unrouted_wait_time_all = 0;
    ULL unrouted_wait_time_wc = 0;
    UI  pntr           = 0;
    
    for (UI i = 0; i < num_ic; i++)           // IChannles
        for (UI j = 0; j < NUM_VCS; j++)
            if (!Ichannel[i]->vc[j].vcQ.empty) {
                pntr = Ichannel[i]->vc[j].vcQ.pntr;
                for (UI k = 0; k < pntr; k++) {
                    unrouted_wait_time = SIM_NUM - Ichannel[i]->vc[j].vcQ.flit_debug_read(k).simdata.gtimestamp;
                    unrouted_wait_time_all += unrouted_wait_time;
                    if (unrouted_wait_time > unrouted_wait_time_wc)
                        unrouted_wait_time_wc = unrouted_wait_time;
                    unrouter_flits_number++;
                }
            }
    
    for (UI i = 0; i < num_oc; i++) {         // OChannels registers
        for (UI j = 0; j < num_ic; j++) 
            if (!Ochannel[i]->r_in[j].free) {
                unrouted_wait_time = SIM_NUM - Ochannel[i]->r_in[j].val.simdata.gtimestamp;
                unrouted_wait_time_all += unrouted_wait_time;
                if (unrouted_wait_time > unrouted_wait_time_wc)
                    unrouted_wait_time_wc = unrouted_wait_time;
                unrouter_flits_number++;
            }
            
        for (UI j = 0; j < NUM_VCS; j++)     // OChannel VCs
            if (!Ochannel[i]->r_vc[j].free) {
                unrouted_wait_time = SIM_NUM - Ochannel[i]->r_vc[j].val.simdata.gtimestamp;
                unrouted_wait_time_all += unrouted_wait_time;
                if (unrouted_wait_time > unrouted_wait_time_wc)
                    unrouted_wait_time_wc = unrouted_wait_time;
                unrouter_flits_number++;
            }
    }
    
    if (unrouter_flits_number > 0)
        avr_latency_unrouted = (double)unrouted_wait_time_all / unrouter_flits_number;
    else
        avr_latency_unrouted = 0.0;
    
    wc_latency_unrouted = unrouted_wait_time_wc;
}

/////////////////////////////////////////////////////////////////
/// Returns average latency per packet across a given channel
////////////////////////////////////////////////////////////////
template <UI num_nb, UI num_ic, UI num_oc>
double NWTile<num_nb, num_ic, num_oc>::return_latency(UI port_dir) {
	return Ochannel[getportid(port_dir)]->avg_latency;
}

/////////////////////////////////////////////////////////////////
/// Returns average throughput across a given channel
////////////////////////////////////////////////////////////////
template <UI num_nb, UI num_ic, UI num_oc>
double NWTile<num_nb, num_ic, num_oc>::return_avg_tput(UI port_dir) {
	return Ochannel[getportid(port_dir)]->avg_throughput;
}

/////////////////////////////////////////////////////////////////
/// Returns average latency per flit across a given channel
////////////////////////////////////////////////////////////////
template <UI num_nb, UI num_ic, UI num_oc>
double NWTile<num_nb, num_ic, num_oc>::return_latency_flit(UI port_dir) {
	return Ochannel[getportid(port_dir)]->avg_latency_flit;
}

/////////////////////////////////////////////////////////////////
/// Returns total latency across all channels in the tile
////////////////////////////////////////////////////////////////
template <UI num_nb, UI num_ic, UI num_oc>
ULL NWTile<num_nb, num_ic, num_oc>::return_total_latency() {
	ULL total_latency = 0;
	for(UI i = 0; i < num_oc; i++) {
		total_latency += Ochannel[i]->latency;
	}
	return total_latency;
}

//////////////////////////////////////////////////////////////////
/// Returns total number of packets across all channels in the tile
//////////////////////////////////////////////////////////////////
template <UI num_nb, UI num_ic, UI num_oc>
ULL NWTile<num_nb, num_ic, num_oc>::return_total_packets() {
	ULL total_packets = 0;
	for(UI i = 0; i < num_oc; i++) {
		total_packets += Ochannel[i]->num_pkts;
	}
	return total_packets;
}

/////////////////////////////////////////////////////////////////
/// Returns total number of flits across all channels in the tile
////////////////////////////////////////////////////////////////
template <UI num_nb, UI num_ic, UI num_oc>
ULL NWTile<num_nb, num_ic, num_oc>::return_total_flits() {
	ULL total_flits = 0;
	for(UI i = 0; i < num_oc; i++) {
		total_flits += Ochannel[i]->num_flits;
	}
	return total_flits;
}

/////////////////////////////////////////////////////////////////////
/// Returns id corresponding to a given port direction (N, S, E, W)
////////////////////////////////////////////////////////////////////
template <UI num_nb, UI num_ic, UI num_oc>
UI NWTile<num_nb, num_ic, num_oc>::getportid(UI port_dir) {
	switch(port_dir) {
		case N: return portN;
		case S: return portS;
		case E: return portE;
		case W: return portW;
	}
	return 0;
}

/////////////////////////////////////////////////////////////////////
/// Returns port direction (N, S, E, W) corresponding to a port id
////////////////////////////////////////////////////////////////////
template <UI num_nb, UI num_ic, UI num_oc>
UI NWTile<num_nb, num_ic, num_oc>::getportdir(UI port_id) {
    if(port_id == portN)
		return N;
	else if(port_id == portS)
		return S;
	else if(port_id == portE)
		return E;
	else if(port_id == portW)
		return W;
	return C;
}

//////////////////////////////////////////////////////////
/// Stress calculation for friends tiles
/////////////////////////////////////////////////////////
template <UI num_nb, UI num_ic, UI num_oc>
void NWTile<num_nb, num_ic, num_oc>::stress_value_out_calc() {
    UI tmp = 0;
	for(UI i = 0; i < num_nb; i++)
		tmp += stress_value_internal[i].read();
    
    if(LOG >= 4) {
			eventlog<<"\ntime: "<<sc_time_stamp()<<" name: "<<this->name()<<" tile: "<<tileID<<" Stress value = "<<tmp;
    }
    
    for(UI i = 0; i < num_nb; i++)
        stress_value_out[i].write(tmp);
}

///////////////////////////////////////////////////////////////////
/// changes router's output channel state to fail or working
/// \param dir output channel direction
/// \param fail new state (true - fail, false - working)
/// \return result
///////////////////////////////////////////////////////////////////
template <UI num_nb, UI num_ic, UI num_oc>
bool NWTile<num_nb, num_ic, num_oc>::set_router_fail_dir(UI dir, bool fail) {
    if (dir > C)
        return false;
        
    ctr.set_router_fail_dir(dir, fail);   
    if (fail)
        Ochannel[getportid(dir)]->setFail();
    else
        Ochannel[getportid(dir)]->setWorking();
    return true;
}

///////////////////////////////////////////////////////////////////
/// gets router's output channel state
/// \param dir output channel direction
/// \return state (true - fail, false - working)
//////////////////////////////////////////////////////////////////
template <UI num_nb, UI num_ic, UI num_oc>
bool NWTile<num_nb, num_ic, num_oc>::get_router_fail_dir(UI dir) {
    if (dir > C)
        return false;
    else
        return ctr.get_router_fail_dir(dir);
}

////////////////////////////////////////////////////
/// Method to check is router turned off
/// \return router turn off state (yes/no)
///////////////////////////////////////////////////
template <UI num_nb, UI num_ic, UI num_oc>
bool NWTile<num_nb, num_ic, num_oc>::is_router_shutdown() {
    return ctr.is_router_shutdown();
}

//////////////////////////////////////////////////////////////////////////
/// Method to change ip core state of generating flit
/// \param toTileID tile ID to which state changing
/// \param grant accept generation (true) or not (false)
/// \return state changed flag
/////////////////////////////////////////////////////////////////////////
template <UI num_nb, UI num_ic, UI num_oc>
bool NWTile<num_nb, num_ic, num_oc>::set_creating_flits_state(UI toTileID, bool grant) {
    return ip->set_creating_flits_state(toTileID, grant);
}

/////////////////////////////////////////////////////////////////
/// returns send packet number by current tile
////////////////////////////////////////////////////////////////
template <UI num_nb, UI num_ic, UI num_oc>
ULL NWTile<num_nb, num_ic, num_oc>::return_send_packets_number() {
	return ip->num_pkts_gen;
}

/////////////////////////////////////////////////////////////////
/// returns send flits number by current flit
////////////////////////////////////////////////////////////////
template <UI num_nb, UI num_ic, UI num_oc>
ULL NWTile<num_nb, num_ic, num_oc>::return_send_flits_number() {
	return ip->num_flits_gen;
}

/////////////////////////////////////////////////////////////////
/// returns received packets number by current flit
////////////////////////////////////////////////////////////////
template <UI num_nb, UI num_ic, UI num_oc>
ULL NWTile<num_nb, num_ic, num_oc>::return_recv_packets_number() {
	return ip->total_packets_recived;
}

/////////////////////////////////////////////////////////////////
/// returns received flits number by current flit
////////////////////////////////////////////////////////////////
template <UI num_nb, UI num_ic, UI num_oc>
ULL NWTile<num_nb, num_ic, num_oc>::return_recv_flits_number() {
	return ip->total_flits_recived;
}

/////////////////////////////////////////////////////////////////
/// returns average latency per packet for a core
////////////////////////////////////////////////////////////////
template <UI num_nb, UI num_ic, UI num_oc>
double NWTile<num_nb, num_ic, num_oc>::return_latency_core() {
	return ip->avg_latency;
}

/////////////////////////////////////////////////////////////////
/// returns average latency per flit for a core
////////////////////////////////////////////////////////////////
template <UI num_nb, UI num_ic, UI num_oc>
double NWTile<num_nb, num_ic, num_oc>::return_latency_flit_core() {
	return ip->avg_latency_flit;
}

/////////////////////////////////////////////////////////////////
/// returns worst-case latency per flit for a core
////////////////////////////////////////////////////////////////
template <UI num_nb, UI num_ic, UI num_oc>
double NWTile<num_nb, num_ic, num_oc>::return_wc_latency_flit_core() {
	return ip->wc_latency;
}

/////////////////////////////////////////////////////////////////
/// returns average throughput for a core
////////////////////////////////////////////////////////////////
template <UI num_nb, UI num_ic, UI num_oc>
double NWTile<num_nb, num_ic, num_oc>::return_avg_tput_core() {
	return ip->avg_throughput;
}

/////////////////////////////////////////////////////////////////
/// returns total latency for a core
////////////////////////////////////////////////////////////////
template <UI num_nb, UI num_ic, UI num_oc>
ULL NWTile<num_nb, num_ic, num_oc>::return_total_latency_core() {
	return ip->total_latency;
}

/////////////////////////////////////////////////////////////////
/// returns worst-case number of clocks waited by flit
////////////////////////////////////////////////////////////////
template <UI num_nb, UI num_ic, UI num_oc>
ULL NWTile<num_nb, num_ic, num_oc>::return_wc_num_waits() {
	return ip->wc_num_waits;
}

/////////////////////////////////////////////////////////////////
/// returns worst-case number of switch travelled by flit
////////////////////////////////////////////////////////////////
template <UI num_nb, UI num_ic, UI num_oc>
ULL NWTile<num_nb, num_ic, num_oc>::return_wc_num_sw() {
	return ip->wc_num_sw;
}

/////////////////////////////////////////////////////////////////
/// returns average number of clocks waited by flit
////////////////////////////////////////////////////////////////
template <UI num_nb, UI num_ic, UI num_oc>
double NWTile<num_nb, num_ic, num_oc>::return_avg_num_waits() {
	return ip->avg_num_waits;
}

/////////////////////////////////////////////////////////////////
/// returns average number of switch travelled by flit
////////////////////////////////////////////////////////////////
template <UI num_nb, UI num_ic, UI num_oc>
double NWTile<num_nb, num_ic, num_oc>::return_avg_num_sw() {
	return ip->avg_num_sw;
}

/////////////////////////////////////////////////////////////////
/// returns buffers utilization by current tile
////////////////////////////////////////////////////////////////
template <UI num_nb, UI num_ic, UI num_oc>
double NWTile<num_nb, num_ic, num_oc>::return_bufs_util() {
    return bufUtil;
}

/////////////////////////////////////////////////////////////////
/// returns VCs utilization by current tile
////////////////////////////////////////////////////////////////
template <UI num_nb, UI num_ic, UI num_oc>
double NWTile<num_nb, num_ic, num_oc>::return_vcs_util() {
    return vcUtil;
}

/////////////////////////////////////////////////////////////////
/// returns average latency of unrouted flits
////////////////////////////////////////////////////////////////
template <UI num_nb, UI num_ic, UI num_oc>
double NWTile<num_nb, num_ic, num_oc>::return_avr_latency_unrouted() {
    return avr_latency_unrouted;
}

/////////////////////////////////////////////////////////////////
/// returns worst-case latency of unrouted flits
////////////////////////////////////////////////////////////////
template <UI num_nb, UI num_ic, UI num_oc>
ULL NWTile<num_nb, num_ic, num_oc>::return_wc_latency_unrouted() {
    return wc_latency_unrouted;
}

template struct NWTile<NUM_NB, NUM_IC, NUM_OC>;
template struct NWTile<NUM_NB_B, NUM_IC_B, NUM_OC_B>;
template struct NWTile<NUM_NB_C, NUM_IC_C, NUM_OC_C>;
