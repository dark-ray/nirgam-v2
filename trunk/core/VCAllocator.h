
/*
 * VCAllocator.h
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
/// \file VCAllocator.h
/// \brief Defines virtual channel allocator
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#ifndef	_VC_ALLOC_
#define	_VC_ALLOC_

#include "systemc.h"
#include "credit.h"
#include "../config/constants.h"
#include "flit.h"
#include <string>
#include <fstream>
#include <iostream>

/// required for stl
using namespace std;

/////////////////////////////////////////////////////////////////////////////
/// \brief Module to represent a Virtual channel allocator
///
/// This module defines a Virtual channel allocator (VCA) in a network tile
/// Template parameters:
/// - num_ip: Number of input channels
/////////////////////////////////////////////////////////////////////////////
template<UI num_ip = NUM_IC>
struct VCAllocator : public sc_module {
	
	// PORTS //////////////////////////////////////////////////////////////////////////////////	
	sc_in_clk           switch_cntrl;				            ///< input clock port
	sc_in<bool>         vcRequest[num_ip];				        ///< input ports for request signals from ICs
	sc_in<UI>           opRequest[num_ip];			            ///< input ports to recieve output channel requested from ICs
	sc_in<creditLine>   Icredit[num_ip][NUM_VCS];		        ///< input ports to recieve credit info (buffer status) from ICs
    sc_in<bool>         congestion_status_in[num_ip][NUM_VCS];  ///< input ports to receive congestion status from ICs	
	sc_out<UI>          nextVCID[num_ip];	                    ///< output ports to send next VCID to ICs
	sc_out<bool>        vcReady[num_ip];				        ///< output ports to send ready signal to ICs
	// PORTS END //////////////////////////////////////////////////////////////////////////////////	
	
	SC_CTOR(VCAllocator); ///< constructor

	// FUNCTIONS /////////////////////////////////////////////////////////////////////////////
	void allocate_VC();	            ///< process sensitive to VC request
	void update_credit();	        ///< process sensitive to credit input,  updates credit (buffer status)
	/// \brief sets tile ID and id corresponding to port directions
	void setTileID(UI tileID, UI portN, UI portS, UI portE, UI portW);
	UI   getNextVCID(UI, UI);	    ///< allocates vcid in the requested channel
    UI   getNextVCID_AA(UI, UI);    ///< allocate vcid in the requested channel with respect adaptive arbitry
	// FUNCTIONS END /////////////////////////////////////////////////////////////////////////////
	
	// VARIABLES /////////////////////////////////////////////////////////////////////////////
	bool vcFree[num_ip][NUM_VCS];	///< status registers to store credit info (buffer status)
	UI   tileID;	                ///< TileID
	UI   portN;	                    ///< port number representing North direction
	UI   portS;	                    ///< port number representing South direction
	UI   portE;	                    ///< port number representing East direction
	UI   portW;	                    ///< port number representing West direction
	// VARIABLES END /////////////////////////////////////////////////////////////////////////////
};

#endif
