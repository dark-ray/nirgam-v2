
/*
 * RouteInfo.h
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
 * Author: Alexander Rumyanthev (darkstreamray@gmail.com)
 * Date:  15.02.2010
 *
 */

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/// \file RouteInfo.h
/// \brief Defines additional information collector module for routing calculation
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#ifndef	_ROUTEINFO_
#define	_ROUTEINFO_

#include "systemc.h"
#include "../config/constants.h"
#include <string>
#include <fstream>
#include <iostream>

/// required for stl
using namespace std;

/////////////////////////////////////////////////////////////////////////////
/// \brief Module to represent a Route Information module
///
/// This module defines a Route Information module (RouteInfo) in a network tile
/////////////////////////////////////////////////////////////////////////////
template<UI num_nb = NUM_NB>
struct RouteInfo : public sc_module {
	
	// PORTS //////////////////////////////////////////////////////////////////////////////////	
    sc_in<bool> in_flit[num_nb];                    ///< input flit arrived
    sc_in<bool> out_flit[num_nb];                   ///< input flit send
		
	sc_out<sc_uint<32> > stress_value[num_nb];	    ///< output stress value of current tile (router)
    sc_out<bool> in_flit_request[num_nb];           ///< output request to input to reset signal
    sc_out<bool> out_flit_request[num_nb];          ///< output request to output to reset signal
	// PORTS END //////////////////////////////////////////////////////////////////////////////////	
	
	/// Constructor
	SC_CTOR(RouteInfo);

	// FUNCTIONS /////////////////////////////////////////////////////////////////////////////
	void exec();	        ///< process sensitive 
    
    /// sets tile ID and id corresponding to port directions
	void setTileID(UI tileID);
	// FUNCTIONS END /////////////////////////////////////////////////////////////////////////////
	
	// VARIABLES /////////////////////////////////////////////////////////////////////////////
	sc_uint<32> cur_stress_value;	///< status register to store current stress value
    
    UI tileID;	///< TileID
	// VARIABLES END /////////////////////////////////////////////////////////////////////////////
};

#endif
