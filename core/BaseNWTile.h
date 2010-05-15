
/*
 * BaseNWTile.h
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
/// \file BaseNWTile.h
/// \brief Defines abstract module for network tile
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#ifndef _BASE_NWTILE_
#define _BASE_NWTILE_

#include "systemc.h"
#include "../config/constants.h"
#include "flit.h"
#include "credit.h"

///////////////////////////////////////////////////////////////////
/// \brief Abstract class to represent network tile.
//////////////////////////////////////////////////////////////////
struct BaseNWTile : public sc_module {

	UI tileID;	        ///< unique tile id
	UI portN;	        ///< port representing North direction
	UI portS;	        ///< port representing South direction
	UI portE;	        ///< port representing East direction
	UI portW;	        ///< port representing West direction
	BaseNWTile() {	}	///< default constructor
	
	SC_HAS_PROCESS(BaseNWTile);
    /// systemC constructor
	/// \param BaseNWTile module name
    /// \param id tile id
	BaseNWTile(sc_module_name BaseNWTile, UI id) : sc_module(BaseNWTile) {
	}
	
    //channel based
	virtual double return_latency(UI)                 = 0;		///< returns average latency per packet for a channel
	virtual double return_latency_flit(UI)            = 0;	    ///< returns average latency per flit for a channel
	virtual double return_avg_tput(UI)                = 0;		///< returns average throughput for a channel
	virtual ULL    return_total_latency()             = 0;		///< returns total latency for a channel
    virtual ULL    return_total_packets()             = 0;      ///< returns total number of packets through a channel
	virtual ULL    return_total_flits()               = 0;		///< returns total number of flits through a channel
    
    //core based
    virtual double return_latency_core()              = 0;		///< returns average latency per packet for a core
	virtual double return_latency_flit_core()         = 0;	    ///< returns average latency per flit for a core
	virtual double return_wc_latency_flit_core()      = 0;	    ///< returns worst-case latency per flit for a core
	virtual double return_avg_tput_core()             = 0;		///< returns average throughput for a core
	virtual ULL    return_total_latency_core()        = 0;		///< returns total latency for a core
	
	//additional core info
	virtual ULL    return_wc_num_waits()              = 0;      ///< returns worst-case number of clocks waited by flit
	virtual ULL    return_wc_num_sw()                 = 0;      ///< returns worst-case number of switch travelled by flit
	virtual double return_avg_num_waits()             = 0;      ///< returns average number of clocks waited by flit
	virtual double return_avg_num_sw()                = 0;      ///< returns average number of switch travelled by flit
    virtual double return_bufs_util()                 = 0;      ///< returns buffers utilization by current tile
    virtual double return_vcs_util()                  = 0;      ///< returns VCs utilization by current tile
    virtual double return_avr_latency_unrouted()      = 0;      ///< returns average latency of unrouted flits
    virtual ULL    return_wc_latency_unrouted()       = 0;      ///< returns worst-case latency of unrouted flits
    
    //core work info
    virtual ULL    return_send_packets_number()       = 0;		///< returns send packet number by current tile
    virtual ULL    return_send_flits_number()         = 0;		///< returns send flits number by current flit
    virtual ULL    return_recv_packets_number()       = 0;		///< returns received packets number by current flit 
    virtual ULL    return_recv_flits_number()         = 0;		///< returns received flits number by current flit
      
    //Additional functionality
    virtual UI     getportid(UI)                      = 0;      ///< returns id corresponding to a port direction (N, S, E, W)
    virtual bool   set_router_fail_dir(UI, bool)      = 0;      ///< change state of router output channel
    virtual bool   get_router_fail_dir(UI)            = 0;      ///< get state of router output channel
    virtual bool   is_router_shutdown()               = 0;      ///< check is router turned off
    virtual bool   set_creating_flits_state(UI, bool) = 0;      ///< change ip core state
};

#endif
