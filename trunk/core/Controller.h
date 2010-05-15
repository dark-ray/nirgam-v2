
/*
 * Controller.h
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
/// \file Controller.h
/// \brief Defines Controller module that implements routing
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#ifndef _CONTROLLER_
#define _CONTROLLER_

#include "systemc.h"
#include "credit.h"
#include "../config/constants.h"
#include "router.h"
#include <string>
#include <fstream>
#include <iostream>
#include <limits>
#include <dlfcn.h>

using namespace std;

//////////////////////////////////////////////////////////////////////////
/// \brief Module to represent a Controller
///
/// This module defines a Controller (implements router)
/// Template parameters:
/// - num_nb: Number of neighbors
/// - num_ip: Number of input channels
//////////////////////////////////////////////////////////////////////////
template<UI num_nb = NUM_NB, UI num_ip = NUM_IC>
struct Controller : public sc_module {

	// PORTS ////////////////////////////////////////////////////////////////////////////////
    //input ports
	sc_in_clk                   switch_cntrl;		                    ///< input clock port
	sc_in<request_type>         rtRequest[num_ip];	                    ///< input ports to receive route request from ICs
	sc_in<UI>                   destRequest[num_ip];	                ///< input ports to recieve destination address
	sc_in<UI>                   sourceAddress[num_ip];                  ///< input ports to receive source address
    sc_in<routing_fault_info>   faultInfoIn[num_ip];                    ///< input ports to receive fault info for fault-tolerance routing 
	sc_in<creditLine>           Icredit[num_ip][NUM_VCS];	            ///< input ports to receive credit info (buffer info) from ICs
    sc_in<UI>                   stress_value[num_nb];                   ///< input ports to receive stress level from adjacent router
    sc_in<bool>                 congestion_flag[num_nb];                ///< input ports to receive congestion flag from adjancent router
    
    //output ports
	sc_out<bool>                rtReady[num_ip];		                ///< output ports to send ready signal to ICs
	sc_out<UI>                  nextRt[num_ip];	                        ///< output ports to send routing decision to ICs
    sc_out<routing_fault_info>  faultInfoOut[num_ip];                   ///< output ports to send fault info for fault-tolerance routing
	// PORTS END /////////////////////////////////////////////////////////////////////////////
	
	/// SystemC constructor
	SC_CTOR(Controller);
	
	// PROCESSES /////////////////////////////////////////////////////////////////////////////
	
    void setTileID(UI tileID, UI portN, UI portS, UI portE, UI portW);  ///< sets tile ID and id corresponding to port directions
	void allocate_route();                                              ///< allocation route systemC thread 	
    void update_stress_value();                                         ///< update stress value systemC method
    void update_congestion_flags();                                     ///< update congestion flags systemC method
    void set_router_fail_dir(UI dir, bool fail);                        ///< changes router's output channel state to fail or working
    bool get_router_fail_dir(UI dir);                                   ///< gets router's output channel state
    bool is_router_shutdown();                                          ///< check is router turned off
	UI   idToDir(UI dir);	                                            ///< returns direction (N, S, E, W) corresponding to a given port id	
	UI   dirToId(UI port_id);                                           ///< returns port id for a given direction (N, S, E, W)		
	// PROCESSES END /////////////////////////////////////////////////////////////////////////
	
	// VARIABLES /////////////////////////////////////////////////////////////////////////////
	UI      tileID;	                    ///< Unique tile identifier
	UI      portN;	                    ///< port number representing North direction
	UI      portS;	                    ///< port number representing South direction
	UI      portE;	                    ///< port number representing North direction
	UI      portW;	                    ///< port number representing North direction
    UI      stress_value_arr[5];        ///< stress values from adjancent routers
    bool    congestion_flags_arr[5];    ///< congestion flags from adjancent routers 
	
	router  *rtable;	                ///< router (plug-in point for routing algorithm)
	// VARIABLES END /////////////////////////////////////////////////////////////////////////
};

#endif
 
