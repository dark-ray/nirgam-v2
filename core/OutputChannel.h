
/*
 * OutputChannel.h
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
/// \file OutputChannel.h
/// \brief Defines module OutputChannel
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#ifndef	_OP_CHANNEL_
#define	_OP_CHANNEL_

#include "systemc.h"
#include "switch_reg.h"
#include "flit.h"
#include "credit.h"
#include "rr_arbiter.h"
#include <string>
#include <fstream>
#include <iostream>

using namespace std;

//////////////////////////////////////////////////////////////////////////
/// \brief Module to represent an Output Channel
///
/// This module defines an Output Channel in a network tile
/// Template parameters:
/// - num_ip: Number of input channels
//////////////////////////////////////////////////////////////////////////
template<UI num_ip = NUM_IC>
struct OutputChannel : public sc_module {
	
	// PORTS ////////////////////////////////////////////////////////////////////////////////////
    sc_in_clk           switch_cntrl;		            ///< clock input port
	sc_in<flit>         inport[num_ip];		            ///< input data/flit ports (one for each IC)
    sc_in<creditLine>   credit_in[NUM_VCS];	            ///< input port to recieve credit info (buffer status) from ICs of neighbor tiles
    sc_in<bool>         congestion_status_in[NUM_VCS];  ///< congestion status from local ICs
	sc_out<bool>        inReady[num_ip];		        ///< output port to send ready signal to IC
	sc_out<flit>        outport;			            ///< output data/flit port	
    // PORTS END ////////////////////////////////////////////////////////////////////////////////////
	
	SC_CTOR(OutputChannel); ///< constructor
    
    // SUBMODULES ////////////////////////////////
    rr_arbiter<NUM_VCS> rr_arbiter_out; ///< RR arbiter for output
    // SUBMODULES END ////////////////////////////
	
	// PROCESSES ///////////////////////////////////////////////////////////////////////////////////////////////
	void entry();			///< reads and processes incoming flit
	void closeLogs();		///< closes logfiles at the end of simulation and computes performance stats
	/// \brief sets tile ID and id corresponding to port directions
	void setTileID(UI tileID, UI portN, UI portS, UI portE, UI portW);
    void processSimCount(); ///< process track of clock cycles
    void setFail();         ///< set fail state of this output channel
    void setWorking();      ///< set working state of this output channel
	// PROCESSES END //////////////////////////////////////////////////////////////////////////////////////////
	
	// VARIABLES //////////////////////////////////////////////////////////////////////////////////////////
	UI tileID;	                ///< unique tile ID
	UI cntrlID;	                ///< control ID to identify channel direction (N, S, E, W, C)
	UI portN;	                ///< port number representing North direction
	UI portS;	                ///< port number representing South direction
	UI portE;	                ///< port number representing East direction
	UI portW;	                ///< port number representing West direction
		
	switch_reg	r_in[num_ip];	///< registers to store flit from inport, one for each inport
	switch_reg	r_vc[NUM_VCS];	///< registers, one for each next VCID

	ULL latency;			    ///< total latency
	ULL num_pkts;			    ///< total number of packets
	ULL num_flits;			    ///< total number of flits
	ULL input_time[NUM_VCS];	///< generation timestamp of head flit of a packet
	ULL sim_count;	            ///< keep track of clock cycles
	
	ULL beg_cycle;			    ///< clock cycle in which first flit is recieved in the channel
	ULL total_cycles;		    ///< total number of clock cycles
	ULL end_cycle;			    ///< clock cycle in which last flit leaves the channel	
    
    bool not_empty_rvc[NUM_VCS];///< not empty local VC state array
    ULL timewait_rvc[NUM_VCS];  ///< wait time of VCs
    ULL timewait_rin[num_ip];   ///< wait time of input registers
	
	double avg_latency;		    ///< average latency (in clock cycles) per packet
	double avg_latency_flit;	///< average latency (in clock cycles) per flit
	double avg_throughput;		///< average throughput (in Gbps)   
    
    bool isFail;                ///< output channel fail condition
	// VARIABLES END //////////////////////////////////////////////////////////////////////////////////////////
    
    // SIGNALS /////////
    // arb out
    sc_signal<bool> req_o[NUM_VCS]; ///< arbiter input VCs request
    sc_signal<bool> arbRequest_o;   ///< main arbitration request
    sc_signal<UI> grant_o;          ///< arbitration result
    sc_signal<bool> arbReady_o;     ///< arbitration ready
    // SIGNALS END //////////
};
#endif
