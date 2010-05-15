 

/*
 * routing_fault_info.h
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
/// \file routing_fault_info.h
/// \brief Defines and implements structure for routing fault info
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#ifndef _ROUTING_FAULT_INFO_
#define _ROUTING_FAULT_INFO_

#include "systemc.h"

/////////////////////////////////////////////////////////////////////////////////////////////////
/// \brief router state structure to transmit router state info
/// This structure need be transfered with HDT/HEAD to make fault-tolerant DyBM algorithm works
//////////////////////////////////////////////////////////////////////////////////////////////////
struct routing_fault_info {
    bool last_back_adap;    ///< last turn back was adaptive
    bool last_back;         ///< last time we turned back
    UI   last_dir;          ///< last time used type of subrouting algorithm
    bool fail;              ///< this packet is FAILED to be delivered
	UI   history;           ///< bit-field of last states (0 - normal, 1 - adaptive)
    
    /// \brief overloading equality operator for routing_fault_info
	inline bool operator == (const routing_fault_info& rfi) const {
		return ((rfi.last_back == last_back) && 
                (rfi.last_back_adap == last_back_adap) && (rfi.fail == fail) && 
                (rfi.history == history) && (rfi.last_dir == last_dir));
	}
};

/// \brief overloading extraction operator for routing_fault_info
inline ostream& operator << ( ostream& os, const routing_fault_info& a) {
    os << " last_back: "<<a.last_back<<" last_back_adap: "<<a.last_back_adap<<
          " fail: "<<a.fail<<" last_dir: "<<a.last_dir<<" history: "<<a.history<<endl;
	return os;
}

/// \brief overloading sc_trace for routing_fault_info
inline void sc_trace( sc_trace_file*& tf, const routing_fault_info& a, const std::string& name) {
    sc_trace( tf, a.last_back, name+".last_back");
    sc_trace( tf, a.last_back_adap, name+".last_back_adap");
    sc_trace( tf, a.last_dir, name+".last_dir");
    sc_trace( tf, a.fail, name+".fail");
    sc_trace( tf, a.history, name+".history");
}

#endif
