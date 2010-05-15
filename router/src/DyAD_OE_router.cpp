
/*
 * DyAD_OE_router.cpp
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
 * Date:  23.02.2010
 *
 */

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/// \file DyAD_OE_router.cpp
/// \brief Implements DyAD-OE routing algorithm
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#include "DyAD_OE_router.h"
#include "../../config/extern.h"

////////////////////////////////////////////////
/// Method that implements DyAD-OE routing
/// inherited from base class router
/// \param ip_dir input direction from which flit entered the tile
/// \param source_id tileID of source tile
/// \param dest_id tileID of destination tile
/// \param rfi routing fault info
/// \return next hop direction
////////////////////////////////////////////////
UI DyAD_OE_router::calc_next(UI ip_dir, ULL source_id, ULL dest_id, routing_fault_info* rfi) {
	bool set_avail[4];
	for(UI i = 0; i < 4; i++)
		set_avail[i] = false;
	int cur_xco = id / num_cols;
	int cur_yco = id % num_cols;
	int src_xco = source_id / num_cols;
	int src_yco = source_id % num_cols;
	int dst_xco = dest_id / num_cols;
	int dst_yco = dest_id % num_cols;
	int dif_xco = dst_xco - cur_xco;
	int dif_yco = dst_yco - cur_yco;
	
	if(dif_xco == 0 && dif_yco == 0)	// cur node is dest
		return C;
	if(dif_yco == 0) {	// cur node in same col as dest
		if(dif_xco < 0) {
			if(ip_dir != N && !borderN(id))		// 180-degree turn prohibited
				set_avail[N] = true;	// allow to route N
		}
		else {
			if(ip_dir != S && !borderS(id))
				set_avail[S] = true;	// allow to route S
		}
	}
	else {
		if(dif_yco > 0) {	// E-bound pkt
			if(dif_xco == 0) {	// cur in same row as dest
				if(ip_dir != E)
					set_avail[E] = true;
			}
			else {
				if(cur_yco % 2 != 0 || cur_yco == src_yco)	// N/S turn allowed only in odd col.
					if(dif_xco < 0 && !borderN(id) && ip_dir != N)
						set_avail[N] = true;
					else if(! borderS(id) && ip_dir != S)
						set_avail[S] = true;
				if(dst_yco % 2 != 0 || dif_yco != 1) {	// allow to go E only if dest is odd col
					if(ip_dir != E)
						set_avail[E] = true;	// because N/S turn not allowed in even col.
				}
			}
		}
		else {	// W-bound
			if(ip_dir != W)
				set_avail[W] = true;
			if(cur_yco % 2 == 0)	// allow to go N/S only in even col. because N->W and S->W not allowed in odd col.
				if(dif_xco <= 0 && !borderN(id) && ip_dir != N) // = 0 to allow non minimal path
					set_avail[N] = true;
				else if(!borderS(id) && ip_dir != S)
					set_avail[S] = true;
		}
	}
    
    if (LOG >= 5) {
        eventlog<<"\ntime: "<<sc_time_stamp()<<" src_xco = "<<src_xco<<" src_yco = "<<src_yco<<" dst_xco = "<<dst_xco<<" dst_yco = "<<dst_yco<<endl;
        eventlog<<"\ntime: "<<sc_time_stamp()<<" xco = "<<cur_xco<<" yco = "<<cur_yco<<endl;
    }
    
    bool need_adaptive_routing = false;
    for (UI i = 0; i < 4; i++) { //dirs N S W E
        if (dir_arr[i] != C)
            need_adaptive_routing |= congestion_flags_arr[dir_arr[i]];
    }

	for(UI i = 0; i < 4; i++) {
		if (set_avail[i] && !need_adaptive_routing) {
            if (LOG >= 5) 
                eventlog<<"\ntime: "<<sc_time_stamp()<<" No congestion flag. Not random. Result = "<<DIRS_NAMES[i]<<endl;
            return i;
        }
		       
        if (set_avail[i] && need_adaptive_routing) 
            if (!congestion_flags_arr[dir_arr[i]]) {
                if (LOG >= 5) 
                    eventlog<<"\ntime: "<<sc_time_stamp()<<" Congestion flag. Not random. Result = "<<DIRS_NAMES[i]<<endl;
                return i;   
            }                           
	}
    
    //all border input ports has congestion flag set
    UI index_res = 0;
    while (1) {                        //random uniform choice
        index_res = rnum.uniform(4);
        if (set_avail[index_res]) {
            if (LOG >= 5) 
                eventlog<<"\ntime: "<<sc_time_stamp()<<" Congestion flag. Random. Result = "<<DIRS_NAMES[index_res]<<endl;
            return index_res;
        }
    }
    
	return 0;
}

////////////////////////////////////////////////
/// Method containing any initializations
/// inherited from base class router
////////////////////////////////////////////////
// may be empty
// definition must be included even if empty, because this is a virtual function in base class
void DyAD_OE_router::initialize() {

}

// for dynamic linking
extern "C" {
router *maker() {
	return new DyAD_OE_router;
}
}
