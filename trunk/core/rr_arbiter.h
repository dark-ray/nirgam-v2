
/*
 * rr_arbiter.h
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
 * Date:  25.02.2010
 *
 */

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/// \file rr_arbiter.h
/// \brief Defines round-robin arbiter module
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#ifndef _RR_ARBITER_
#define _RR_ARBITER_
#include "systemc.h"
#include <string>
#include <fstream>
#include <iostream>
#include "../config/constants.h"
#include "../config/extern.h"

/// required for stl
using namespace std;

/////////////////////////////////////////
/// \brief rr_arbiter data structure
/// Template parameters:
/// - num_req: Number of units needed arbitration
////////////////////////////////////////
template<UI num_req = NUM_VCS>
struct rr_arbiter : public sc_module {
    sc_in<bool>     req[num_req];   ///< input ports to receive arbitration info
    sc_in<bool>     arbRequest;     ///< input port to receive arbitration request
    sc_out<UI>      grant;          ///< output port to send arbitration result
    sc_out<bool>    arbReady;       ///< output port to send ready signal
    
    /// Constructor
	SC_CTOR(rr_arbiter);
    
    void make_arbitrage();          ///< make arbitration method
    void update_requests();         ///< update arbitration info (requests)
    
    UI cur_grant;                   ///< current grant id
    bool requests[num_req];         ///< array of arbitration info (requests)
};

#endif
