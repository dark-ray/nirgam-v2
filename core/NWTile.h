
/*
 * NWTile.h
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
/// \file NWTile.h
/// \brief Defines a network tile
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#ifndef __NW_TILE__
#define __NW_TILE__

#include "systemc.h"
#include "../config/constants.h"
#include "flit.h"
#include "credit.h"
#include "InputChannel.h"
#include "OutputChannel.h"
#include "VCAllocator.h"
#include "RouteInfo.h"
#include "ipcore.h"
#include "Controller.h"
#include "BaseNWTile.h"

#define ptr   (NWTile<NUM_NB, NUM_IC, NUM_OC> *)
#define ptr_b (NWTile<NUM_NB_B, NUM_IC_B, NUM_OC_B> *)
#define ptr_c (NWTile<NUM_NB_C, NUM_IC_C, NUM_OC_C> *)

//////////////////////////////////////////////////////////////////////////
/// \brief Module to represent a tile in NoC
///
/// This module defines a network tile and submodules within it.
/// It is derived from abstract class BaseNWTile.
/// Template parameters:
/// - num_nb: Number of neighbors
/// - num_ic: Number of input channels
/// - num_oc: Number of output channels
//////////////////////////////////////////////////////////////////////////
template <UI num_nb = NUM_NB, UI num_ic = NUM_IC, UI num_oc = NUM_OC>
struct NWTile : public BaseNWTile {

	// PORTS ////////////////////////////////////////////////////////////////////////////////////
	sc_in_clk	         switch_cntrl;		                    ///< input clock port
	sc_in<flit>	         ip_port[num_nb];	                    ///< input data/flit ports
	sc_out<flit>         op_port[num_nb];	                    ///< output data/flit ports

	sc_in<creditLine>	 credit_in[num_nb][NUM_VCS];	        ///< input ports for credit line (buffer status)
	sc_out<creditLine>	 credit_out[num_nb][NUM_VCS];           ///< output ports for credit line (buffer status)
    
    sc_in<UI>  stress_value_in[num_nb];                         ///< input ports for stress values
    sc_out<UI> stress_value_out[num_nb];                        ///< output posts for stress values    
    
    sc_in<bool>          congestion_flag_in[num_nb];            ///< input ports for congestion flags
    sc_out<bool>         congestion_flag_out[num_nb];           ///< output ports for congestion flags
    
    sc_in<bool>          congestion_status_in[num_nb][NUM_VCS]; ///< input ports for congestion statuses
    sc_out<bool>         congestion_status_out[num_nb][NUM_VCS];///< output ports for congestion statuses
	// PORTS END ////////////////////////////////////////////////////////////////////////////////

	/// \brief constructor
	SC_HAS_PROCESS(NWTile);
	NWTile(sc_module_name NWTile, UI tileID);

	// PROCESSES //////////////////////////////////////////////////////////////////////////////////////////
	void    entry();		                                    ///< Writes buffer utilization information at the tile, at each clock cycle
	void    setID(UI);		                                    ///< sets unique tile id and associates ports with directions
	double  return_latency(UI port_dir);		                ///< returns average latency per packet for a channel
	double  return_latency_flit(UI port_dir);	                ///< returns average latency per flit for a channel
	double  return_avg_tput(UI port_dir);		                ///< returns average throughput for a channel
    void    closeLogs();                                        ///< close logs
	ULL     return_total_latency();			                    ///< returns total latency across all channels in the tile
    ULL     return_total_packets();                             ///< returns total packets across all channels in the tile
	ULL     return_total_flits();			                    ///< returns total flits across all channels in the tile
	UI      getportid(UI port_dir);	                            ///< returns id corresponding to a port direction (N, S, E, W)
    UI      getportdir(UI port_id);                             ///< returns port direction (N, S, E, W) corresponding to a port id
    void    stress_value_out_calc();                            ///< calculate local stress value (based on fifo's occupancy)
    bool    set_router_fail_dir(UI dir, bool fail);             ///< set router output channel fail state
    bool    get_router_fail_dir(UI dir);                        ///< get router output channel fail state
    bool    is_router_shutdown();                               ///< check is router turned off
    bool    set_creating_flits_state(UI toTileID, bool grant);  ///< set state of creating flits to certain tile
    
    //core based stats
    double  return_latency_core();		    ///< returns average latency per packet for a core
	double  return_latency_flit_core();	    ///< returns average latency per flit for a core
	double  return_wc_latency_flit_core();	///< returns worst-case latency per flit for a core
	double  return_avg_tput_core();		    ///< returns average throughput for a core
	ULL     return_total_latency_core();	///< returns total latency for a core
	
	//additional core info
	ULL     return_wc_num_waits();          ///< returns worst-case number of clocks waited by flit
	ULL     return_wc_num_sw();             ///< returns worst-case number of switch travelled by flit
	double  return_avg_num_waits();         ///< returns average number of clocks waited by flit
	double  return_avg_num_sw();            ///< returns average number of switch travelled by flit
    double  return_bufs_util();             ///< returns buffers utilization by current tile
    double  return_vcs_util();              ///< returns VCs utilization by current tile
    double  return_avr_latency_unrouted();  ///< returns average latency of unrouted flits
    ULL     return_wc_latency_unrouted();   ///< returns worst-case latency of unrouted flits
    
    //core work info
    ULL     return_send_packets_number();   ///< returns send packet number by current tile
    ULL     return_send_flits_number();     ///< returns send flits number by current flit
    ULL     return_recv_packets_number();	///< returns received packets number by current flit
    ULL     return_recv_flits_number();		///< returns received flits number by current flit
    
    void resetCounts();                     ///< reset statistics
	// PROCESS END /////////////////////////////////////////////////////////////////////////////////////

	// SUBMODULES /////////////////////////////////////////////////////////////////
	InputChannel<num_oc>	        *Ichannel[num_ic];	///< Input channels
	OutputChannel<num_ic>	        *Ochannel[num_oc];	///< Output channels
	VCAllocator<num_ic> 	        vcAlloc;		    ///< Virtual Channel Allocator
	ipcore			                *ip;			    ///< IP Core
	Controller<num_nb, num_ic>      ctr;			    ///< Controller
	// SUBMODULES END //////////////////////////////////////////////////////////////////////////////////

	// SIGNALS ///////////////////////////////////////////////////////////////////////////////////
    /// \brief signals to connect data outport of ICs to the data inport of the OCs
	sc_signal<flit>	flit_sig[num_ic][num_oc];
	/// \brief data line from ipcore to input channel
	sc_signal<flit>	flit_CS_IC;
	/// \brief data line from output channel to ipcore
	sc_signal<flit> flit_OC_CR;

	/// \brief ready signals from ICs to OCs of neighboring tiles
	sc_signal<bool>	rdy[num_ic][num_oc];

	/// \brief Request signal for virtual channel allocation from IC to VCA
	sc_signal<bool> vcReq[num_ic];
	/// \brief Output port requested from IC to VCA
	sc_signal<UI>	opReq[num_ic];

	/// \brief Ready signal from VCA to IC
	sc_signal<bool> vcReady[num_ic];
	/// \brief Virtual channel id allocated from VCA to IC
	sc_signal<UI>	nextvc[num_ic];

	/// \brief credit line from core channel to VCA and ipcore
	sc_signal<creditLine>	creditIC_CS[NUM_VCS];
	//sc_signal<creditLine>	creditCR_OC[NUM_VCS];

	/// \brief Routing request signal from IC to Ctr
	sc_signal<request_type> rtReq[num_ic];
	/// \brief Destination address from IC to Ctr
	sc_signal<UI> destReq[num_ic];
	/// \brief Source address from IC to Ctr
	sc_signal<UI> srcAddr[num_ic];
    /// \brief fault info signals from adjancert routers to Ctr
    sc_signal<routing_fault_info> faultInfoToCTR[num_ic];
    /// \brief fault info signals from Ctr to adjancent routers
    sc_signal<routing_fault_info> faultInfoFromCTR[num_ic];

	/// \brief Ready signal from Ctr to IC
	sc_signal<bool> rtReady[num_ic];
	/// \brief Route (output port) signal from Ctr to IC
	sc_signal<UI> nextRt[num_ic];
    
    /// \brief stress values signals from IC to Tile
    sc_signal<UI> stress_value_internal[num_oc];
    /// \brief stress value signal loop IC to IC (if IC attach to Core)
    sc_signal<bool> congestion_flag_loop;  
    /// \brief congestion status signal loop
    sc_signal<bool> congestion_status_loop[NUM_VCS]; 
    /// \brief congestion status signal false
    sc_signal<bool> congestion_status_false[NUM_VCS]; 
	// SIGNALS END ///////////////////////////////////////////////////////////////////////////////////////////////////
    
    // VARIABLES //////////////////////////////////////////////////
    ULL     totBufsOcc;             ///< total number of buffers occupated
    ULL     totVCOcc;               ///< total number of VCs occupated
    double  avr_latency_unrouted;   ///< average latency of unrouted flits
    ULL     wc_latency_unrouted;    ///< worst-case latency of unrouted flits
    double  bufUtil;                ///< buffers utilization
    double  vcUtil;                 ///< VCs utilization
    // VARIABLES END ///////////////////////////////////////////
};

#endif
