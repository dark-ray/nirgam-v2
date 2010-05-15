
/*
 * router_state.h
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
 * Date:  21.03.2010
 *
 */

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/// \file router_state.h
/// \brief Defines and implements structure for router state info
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#ifndef _ROUTER_STATE_
#define _ROUTER_STATE_

#include "systemc.h"

///////////////////////////////////////////////////////////////////////
/// \brief router state structure to transmit router state info
///////////////////////////////////////////////////////////////////////
struct router_state {
	bool	faultDir[5];		///< fault directions (0 - N, 1 - S, 2 - E, 3 - W, 4 - C)
	bool	typeID;	            ///< current router ID is even (true) or odd(false)

	/// \brief constructor 
    router_state () {
        for (UI i = 0; i < 5; i++)
            faultDir[i] = false;
        typeID = false;
    }
};

/// \brief overloading extraction operator for router_state
inline ostream& operator << ( ostream& os, const router_state& a) {
    for (UI i = 0; i < 5; i++)
        os << "faultDir["<<i<<"] = "<<a.faultDir[i]<<endl;
	os <<"typeID: "<< a.typeID << endl;
	return os;
}

/// \brief overloading sc_trace for router_state
inline void sc_trace( sc_trace_file*& tf, const router_state& a, const std::string& name) {
    
    sc_trace( tf, a.faultDir[0], name+".faultDir[0]");
    sc_trace( tf, a.faultDir[1], name+".faultDir[1]");
    sc_trace( tf, a.faultDir[2], name+".faultDir[2]");
    sc_trace( tf, a.faultDir[3], name+".faultDir[3]");
    sc_trace( tf, a.faultDir[4], name+".faultDir[4]");
	sc_trace( tf, a.typeID, name+".typeID");
}

#endif
