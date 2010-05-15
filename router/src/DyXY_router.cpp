
/*
 * DyXY_router.cpp
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
/// \file DyXY_router.cpp
/// \brief Implements DyXY routing algorithm
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#include "DyXY_router.h"
#include "../../config/extern.h"

////////////////////////////////////////////////
/// Method that implements DyXY routing
/// inherited from base class router
/// \param ip_dir input direction from which flit entered the tile
/// \param source_id tileID of source tile
/// \param dest_id tileID of destination tile
/// \param rfi routing fault info
/// \return next hop direction
////////////////////////////////////////////////
UI DyXY_router::calc_next(UI ip_dir, ULL source_id, ULL dest_id, routing_fault_info* rfi) {
    
	int xco = id / num_cols;
	int yco = id % num_cols;
	int dest_xco = dest_id / num_cols;
	int dest_yco = dest_id % num_cols;
    
    int choice_1 = 0, choice_2 = 0;
    UI  deb_res = 0;
    UI  calc_res_1 = 0, calc_res_2 = 0; 
    
    if (stress_value_arr != NULL) {
        if (dest_yco == yco) {
            if (dest_xco < xco)
                return N;
            else if (dest_xco > xco)
                return S;
            else if (dest_xco == xco)
                return C;
        }
        else if (dest_xco == xco) {
            if (dest_yco > yco)
                return E;
            else
                return W;
        }
        else {
			if (dest_xco > xco)
			    choice_1 = S;
			else
				choice_1 = N;
            if (dest_yco > yco)
                choice_2 = E;
            else
                choice_2 = W;                
        }
                
        if (LOG >= 5)
            eventlog<<"\ntime: "<<sc_time_stamp()<<" tileID = "<<id<<" ip_dir = "<<ip_dir<<" Additional routing"<<endl;
        
        calc_res_1 = stress_value_arr[dir_arr[choice_1]];
        calc_res_2 = stress_value_arr[dir_arr[choice_2]];
                
        if (calc_res_1 > calc_res_2)
            deb_res = choice_2;
        else if (calc_res_1 < calc_res_2)
            deb_res = choice_1;
        else  {                                 // random uniform select output port
            int random_choice = rnum.uniform(2);
            if (random_choice == 0)
                deb_res = choice_1;
            else
                deb_res = choice_2;
        }
        
        if (deb_res == ip_dir) {                // don't transfer back
            if (choice_1 == ip_dir)
                deb_res = choice_2;
            else
                deb_res = choice_1;
        }
            
        if (LOG >= 5) {
            eventlog<<"\ntime: "<<sc_time_stamp()<<" xco = "<<xco<<" yco = "<<yco<<" dest_xco = "<<dest_xco<<" dest_yco = "<<dest_yco<<endl;
            eventlog<<"\ntime: "<<sc_time_stamp()<<" c1 = "<<choice_1<<" port = "<<dir_arr[choice_1]<<" val = "<< calc_res_1<<endl;
            eventlog<<"\ntime: "<<sc_time_stamp()<<" c2 = "<<choice_2<<" port = "<<dir_arr[choice_2]<<" val = "<< calc_res_2<<endl;
            eventlog<<"\ntime: "<<sc_time_stamp()<<" Result = "<<DIRS_NAMES[deb_res]<<endl;
        }
        return deb_res;
    }
    else
        return 0; //error!
}

////////////////////////////////////////////////
/// Method containing any initializations
/// inherited from base class router
////////////////////////////////////////////////
// may be empty
// definition must be included even if empty, because this is a virtual function in base class
void DyXY_router::initialize() {

}

// for dynamic linking
extern "C" {
router *maker() {
	return new DyXY_router;
}
}
