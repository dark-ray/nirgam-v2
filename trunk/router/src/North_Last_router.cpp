
/*
 * North_Last_router.cpp
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
/// \file North_Last_router.cpp
/// \brief Implements North Last routing algorithm
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#include "North_Last_router.h"
#include "../../config/extern.h"

////////////////////////////////////////////////
/// Method that implements North Last routing
/// inherited from base class router
/// \param ip_dir input direction from which flit entered the tile
/// \param source_id tileID of source tile
/// \param dest_id tileID of destination tile
/// \param rfi routing fault info
/// \return next hop direction
////////////////////////////////////////////////
UI North_Last_router::calc_next(UI ip_dir, ULL source_id, ULL dest_id, routing_fault_info* rfi) {
	bool set_avail[4];
	for(UI i = 0; i < 4; i++)
		set_avail[i] = false;
    int xco = id / num_cols;
	int yco = id % num_cols;
	int dest_xco = dest_id / num_cols;
	int dest_yco = dest_id % num_cols;
    
	if(dest_yco > yco) {
        if (dest_xco < xco) {
            set_avail[N] = true;
            set_avail[E] = true;
        }
        else
            return E;
    }
    else if (dest_yco < yco) {
        if (dest_xco < xco) {
            set_avail[N] = true;
            set_avail[W] = true;
        }
        else
            return W;
        
    }
    else {
        if (dest_xco > xco)
            return S;
        else if (dest_xco < xco)
            return N;
        else
            return C;
    }
    
    //use TR_TYPE
    if (TR_TYPE == CONGESTION) {
        for (UI i = 0; i < 4; i++) //dirs N S W E
            if ((dir_arr[i] != C) && !congestion_flags_arr[dir_arr[i]] && set_avail[i])
                return i;                
    }
    
    
    //random part
    UI index_res = 0;
    while (1) {                        //random uniform choice
        index_res = rnum.uniform(4);
        if (set_avail[index_res])
             return index_res;
    }
	return 0;
}

////////////////////////////////////////////////
/// Method containing any initializations
/// inherited from base class router
////////////////////////////////////////////////
// may be empty
// definition must be included even if empty, because this is a virtual function in base class
void North_Last_router::initialize() {

}

// for dynamic linking
extern "C" {
router *maker() {
	return new North_Last_router;
}
}
