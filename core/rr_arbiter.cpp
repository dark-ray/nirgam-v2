
/*
 * rr_arbiter.cpp
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
/// \file rr_arbiter.cpp
/// \brief Implements virtual channel allocator
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#include "rr_arbiter.h"

////////////////////////
/// constructor
////////////////////////
template<UI num_req>
rr_arbiter<num_req>::rr_arbiter(sc_module_name rr_arbiter): sc_module(rr_arbiter) {

	// make_arbitrage
	SC_THREAD(make_arbitrage);
	sensitive << arbRequest;
    
    // updates requests
	SC_THREAD(update_requests);
	for(UI i = 0; i < num_req; i++){
		sensitive << req[i];
	}
	
	// initialize credit status
	for(UI i = 0; i < num_req; i++) {
		requests[i] = false;
	}
    
    cur_grant = 0;
}

/////////////////////////////////////////////////////////
/// Method to updates arbitration incoming data (requests)
////////////////////////////////////////////////////////
template<UI num_req>
void rr_arbiter<num_req>::update_requests() {
	while(true) {
		wait();	// wait until change in req info
		for(UI i = 0; i < num_req; i++) {
			if(req[i].event()) {
				requests[i] = req[i].read();	// update 
			}
		}
	}//end while
}//end entry

///////////////////////////////////////////////////////////
/// Method to make arbitration and output result
//////////////////////////////////////////////////////////
template<UI num_req>
void rr_arbiter<num_req>::make_arbitrage() {
	while(true) {
		wait();	// wait until change in req info
        if (arbRequest.event() && (arbRequest.read() == true)) {  // can do arbitration
            UI temp = (cur_grant + 1) % num_req;
            bool done = false;
            for(UI i = 0; i < num_req; i++) {
                if(requests[temp]) {
                    done = true;
                    break;
                }
                temp = (temp + 1) % num_req;
            }
            if (done)
                cur_grant = temp; 
            grant.write(cur_grant);
            arbReady.write(true);    
        }        
        if(arbRequest.event() && (arbRequest.read() == false)) { // no request
            arbReady.write(false);
        }
	}//end while
}//end entry

template struct rr_arbiter<NUM_VCS>;
