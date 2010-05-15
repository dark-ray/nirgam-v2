
/*
 * InputChannel.h
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
/// \file InputChannel.h
/// \brief Defines module InputChannel (reads and processes incoming flits)
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#ifndef	_IP_CHANNEL_
#define	_IP_CHANNEL_

#include "fifo.h"
#include "systemc.h"
#include "rng.h"
#include "credit.h"
#include "flit.h"
#include "rr_arbiter.h"
#include <string>
#include <iostream>
#include <fstream>

/// required for stl
using namespace std;

///////////////////////////////////////////////
/// \brief Virtual channel (VC) data structure
///////////////////////////////////////////////
struct  VC {
	UI	                id;		        ///< unique identifier
	UI	                vc_next_id;	    ///< virtual channel id on next tile to which flit is supposed to go
	UI	                num_buf;	    ///< number of buffers in this VC
	UI		            vc_route;	    ///< routing decision (next hop) for the flits stored in this VC
    routing_fault_info  new_rfi;        ///< routing fault info structure (routing_fault_info)
	fifo			    vcQ;		    ///< buffer (fifo queue)
};

//////////////////////////////////////////////////////////////////////////
/// \brief Module to represent an Input Channel
///
/// This module defines an Input Channel in a network tile
/// Template parameters:
/// - num_op: Number of output channels
//////////////////////////////////////////////////////////////////////////
template<UI num_op = NUM_OC>
struct InputChannel : public sc_module {

	// PORTS //////////////////////////////////////////////////////////////////////////////////
    //input ports
	sc_in_clk                   switch_cntrl;	                ///< input clock port
	sc_in<flit>                 inport;		                    ///< input data/flit port
    sc_in<bool>                 outReady[num_op];	            ///< input ports for ready signal from OCs
    sc_in<bool>                 vcReady;		                ///< input port for ready signal from VCA
    sc_in<UI>	                nextVCID;	                    ///< input port to receive next VCID from VCA
    sc_in<bool>                 rtReady;		                ///< input port to receive ready signal from Controller
    sc_in<UI>                   nextRt;			                ///< input port to receive routing decision (next hop) from Controller
    sc_in<routing_fault_info>   faultInfoIn;                    ///< input port to receive fault information for routing
    
	sc_out<flit>                outport[num_op];	            ///< ouput data/flit ports (one for each output channel)
	sc_out<bool>                vcRequest;		                ///< output port for sending request to VCA
	sc_out<UI>                  opRequest;	                    ///< output port for sending OC requested to VCA
	sc_out<creditLine>          credit_out[NUM_VCS];		    ///< output ports to send credit info (buffer status) to OC, VCA and Ctr
	sc_out<request_type>        rtRequest;			            ///< output port to send request to Controller
	sc_out<UI>                  destRequest;	                ///< output port to send destination address to Controller
	sc_out<UI>                  sourceAddress;	                ///< output port to send source address to Controller
    sc_out<routing_fault_info>  faultInfoOut;                   ///< output port to send fault information for routing 
	sc_out<UI>                  stress_value_int_out;           ///< output port to send stress value state
    sc_out<bool>                congestion_flag;                ///< output port to send congestion flag 
    sc_out<bool>                congestion_status_out[NUM_VCS]; ///< output port to send congestion status for each VC
	// PORTS END //////////////////////////////////////////////////////////////////////////////
    
    // SUBMODULES ////
    rr_arbiter<NUM_VCS> rr_arbiter_route;       ///< round-robin arbiter for route selection
    rr_arbiter<NUM_VCS> rr_arbiter_transmit;    ///< round-robin arbiter for Xbar transmit selection
    /////////////////
	
	SC_CTOR(InputChannel); ///< Constructor

	// FUNCTIONS /////////////////////////////////////////////////////////////////////////////
	void read_flit();		    ///< reads flit from i/p port and calls function to store it in buffer
	void store_flit_VC(flit*);	///< stores flit in buffer
	void route_flit();		    ///< routes the flit at the front of fifo buffer
	void routing_src(flit*);	///< routing function for algorithms containing entire path in header (source routing)
	void routing_dst(flit*);	///< routing function for algorithms containing destination address in header
	void transmit_flit();		///< transmits flit at the front of fifo to output port
	void setTileID(UI tileID, UI portN, UI portS, UI portE, UI portW); ///< sets tile ID and id corresponding to port directions
	void resetCounts();		    ///< resets buffer counts to zero
    void processIntLogic();     ///< track clocks count and update router's stress value
	UI   reverse_route(UI);		///< reverses route (to be used in future)
	void inc_vcs_num_waits();   ///< increment number of wait clocks in all waiting flits
	// FUNCTIONS END /////////////////////////////////////////////////////////////////////////
	
	// VARIABLES /////////////////////////////////////////////////////////////////////////////
	VC	    vc[NUM_VCS];	        ///< Virtual channels
    ULL     timewait_r[NUM_VCS];    ///< Time waits for routing at buffers 
    ULL     timewait_t[NUM_VCS];    ///< Time waits for transmiting at buffers 
    bool    served_r[NUM_VCS];      ///< state array for served VC for routing selection
    bool    not_empty_r[NUM_VCS];   ///< state array for not empty VC for routing
    bool    not_empty_t[NUM_VCS];   ///< state array for not empty VC for transmit to output channel
	UI	    cntrlID;	            ///< Control ID to identify channel direction
	UI	    tileID;		            ///< Tile ID
	UI	    portN;		            ///< port number representing North output direction
	UI	    portS;		            ///< port number representing South output direction
	UI	    portE;		            ///< port number representing East output direction
	UI	    portW;		            ///< port number representing West output direction
	UI	    numBufReads;	        ///< number of buffer reads in the channel
	UI	    numBufWrites;	        ///< number of buffer writes in the channel
	UI	    numBufsOcc;	            ///< number of occupied buffers
	UI	    numVCOcc;	            ///< number of occupied virtual channels
	ULL     sim_count;	            ///< keeps track of number of clock cycles
    UI      stress_value;           ///< stress value of current router
	// VARIABLES END /////////////////////////////////////////////////////////////////////////
    
    // SIGNALS //////////////////////////////////////////////////////////////////////////////
    
    // route
    sc_signal<bool>     req_r[NUM_VCS]; ///< arbitration info signals for routing
    sc_signal<bool>     arbRequest_r;   ///< arbitration request signal for routing
    sc_signal<UI>       grant_r;        ///< arbitration result for routing
    sc_signal<bool>     arbReady_r;     ///< arbitration ready signal for routing
    
    // transmit
    sc_signal<bool>     req_t[NUM_VCS]; ///< arbitration info signals for transmit
    sc_signal<bool>     arbRequest_t;   ///< arbitration request signal for transmit
    sc_signal<UI>       grant_t;        ///< arbitration result for transmit
    sc_signal<bool>     arbReady_t;     ///< arbitration ready signal for transmit
    // SIGNALS END //////////////////////////////////////////////////////////////////////////
};

#endif

