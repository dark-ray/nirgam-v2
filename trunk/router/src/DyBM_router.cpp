 
/*
 * DyBM_router.cpp
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
/// \file DyBM_router.cpp
/// \brief Implements DyBM routing algorithm
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#include "DyBM_router.h"
#include "../../config/extern.h"

////////////////////////////////////////////////
/// Method to calculation subalgorithm type
/// \param src tileID of source tile
/// \param dst tileID of destination tile
/// \param back_dir tileID of prev. tile on way
/// \return subalgo type
////////////////////////////////////////////////
UI DyBM_router::get_fault_dir(UI src, UI dst, UI back_dir) {
    UI src_x = src / num_cols;
    UI src_y = src % num_cols;
    UI dst_x = dst / num_cols;
    UI dst_y = dst % num_cols;
    UI tmp = 0;
    
    if (dst_x > src_x) {
        if (dst_y > src_y)
            tmp = 0;
        else if (dst_y < src_y)
            tmp = 1;
        else {
            if (borderE(src) || (back_dir == E))
                tmp = 1;
            else
                tmp = 0;
        }
    }
    else if (dst_x < src_x) {
        if (dst_y > src_y)
            tmp = 2;
        else if (dst_y < src_y)
            tmp = 3;
        else {
            if (borderW(src)|| (back_dir == W))
                tmp = 2;
            else
                tmp = 3;
        }
    }
    else {
        if (dst_y > src_y) {
            if (borderN(src)|| (back_dir == N))
                tmp = 0;
            else 
                tmp = 2;            
        }
        else {
            if (borderS(src)|| (back_dir == S))
                tmp = 3;
            else 
                tmp = 1;              
        }
    }
   
    return tmp;
}

////////////////////////////////////////////////
/// Method that implements DyBM routing
/// inherited from base class router
/// Parameters:
/// \param ip_dir input direction from which flit entered the tile
/// \param source_id tileID of source tile
/// \param dest_id tileID of destination tile
/// \param rfi routing fault info
/// \return next hop direction
////////////////////////////////////////////////
UI DyBM_router::calc_next(UI ip_dir, ULL source_id, ULL dest_id, routing_fault_info* rfi) {
 
	int cur_xco = id / num_cols;
	int cur_yco = id % num_cols;
	int dest_xco = dest_id / num_cols;
	int dest_yco = dest_id % num_cols;
    
    if (rfi == NULL)
        return ND;
        
    // additional variables
    UI hist             = rfi->history;
    bool last_adap      = (hist & 1) ? true : false;
    bool first_is_adap  = hist & 0x80000000;
    bool lost_first     = false;
    bool correct_choise = false;
    
    bool cx_eq_dx_ex    = (cur_xco == dest_xco);
    bool cy_eq_dy_ex    = (cur_yco == dest_yco);
    UI dir              = 0;
    UI deb_res          = ND;
    UI norm_choise      = ND;
    UI adap_choise      = ND;   
    UI final_choise     = ND;
    UI back_choise      = ND;
    
    bool forw_move = false;
    bool forw_adap = false;
    
    dir = get_fault_dir(id, dest_id, ip_dir);
    
    if (dest_id == id) {
        if (state.faultDir[C]) {
            rfi->fail = true; //FAIL to route flit
            if (LOG >= 5)
                eventlog<<"id = "<<id<<" dest_id = "<<dest_id<<" FAIL!"<<endl;
        }
        else 
            deb_res = C;
        goto end_DyBM;        
    }
    
    switch (dir) {  // select two choise based on dir type and even flag
        case 0: { 
            if (state.typeID) {
                norm_choise = E;
                adap_choise = S;
                if (cy_eq_dy_ex)
                    correct_choise = true;
            }
            else {
                norm_choise = S;
                adap_choise = E;
                if (cx_eq_dx_ex)
                    correct_choise = true;
            }
        }; break;
        case 1: {
            if (state.typeID) {
                norm_choise = S;
                adap_choise = W;
                if (cx_eq_dx_ex)
                    correct_choise = true;
            }
            else {
                norm_choise = W;
                adap_choise = S;
                if (cy_eq_dy_ex)
                    correct_choise = true;
            }
        }; break;
        case 2: {
            if (state.typeID) {
                norm_choise = N;
                adap_choise = E;
                if (cx_eq_dx_ex)
                    correct_choise = true;
            }
            else {
                norm_choise = E;
                adap_choise = N;
                if (cy_eq_dy_ex)
                    correct_choise = true;
            }
        }; break;
        case 3: {
            if (state.typeID) {
                norm_choise = W;
                adap_choise = N;
                if (cy_eq_dy_ex)
                    correct_choise = true;
            }
            else {
                norm_choise = N;
                adap_choise = W;
                if (cx_eq_dx_ex)
                    correct_choise = true;
            }
        }; break;
    }     
    
    if (correct_choise) {   // swap choise and invert last back adaptie flag for moving in one dimension if needed
        if (state.faultDir[adap_choise] || (adap_choise == ip_dir)) { // try to move forward
            if (state.faultDir[norm_choise] || (norm_choise == ip_dir))
                forw_move = false;
            else {
                forw_move = true;
                forw_adap = true;
                final_choise = norm_choise;
            }
        }
        else {
            forw_move = true;
            forw_adap = false;
            final_choise = adap_choise;
        }
        
        if (rfi->last_back && rfi->last_back_adap) { // last move was back and not adaptive move forward was at that place
                                                    // so try move forward with adaptive choise
            if (state.faultDir[norm_choise] || (norm_choise == ip_dir))
                forw_move = false;
            else {
                forw_move = true;
                forw_adap = false;
                final_choise = norm_choise;
            }
        }
        
        if (rfi->last_back && !rfi->last_back_adap) // last move was back and adapitve move forward was at that place so move back one more  
                forw_move = false;
    }
    else {
        if (state.faultDir[norm_choise] || (norm_choise == ip_dir)) { // try to move forward
            if (state.faultDir[adap_choise] || (adap_choise == ip_dir))
                forw_move = false;
            else {
                forw_move = true;
                forw_adap = true;
                final_choise = adap_choise;
            }
        }
        else {
            forw_move = true;
            forw_adap = false;
            final_choise = norm_choise;
        }
        
        if (rfi->last_back && !rfi->last_back_adap) { // last move was back and not adaptive move forward was at that place
                                                    // so try move forward with adaptive choise
            if (state.faultDir[adap_choise] || (adap_choise == ip_dir))
                forw_move = false;
            else {
                forw_move = true;
                forw_adap = true;
                final_choise = adap_choise;
            }
        }
        
        if (rfi->last_back && rfi->last_back_adap) // last move was back and adapitve move forward was at that place so move back one more  
                forw_move = false;
    }
    
    if (forw_move) {
        lost_first = true;
        rfi->last_back = false;
        deb_res = final_choise;
        hist = hist << 1;
        if (forw_adap)
            hist += 1;
        else 
            hist += 0;
    }
    else {
        lost_first = false;
        rfi->last_back = true;
    }
    
    if (lost_first && first_is_adap) { // history overflow, bad, really BAD
        if (LOG >= 5) {
            eventlog<<"id = "<<id<<" dest_id = "<<dest_id<<" lost_first && first_is_adap: ";
            eventlog<<"0x"<<hex<<hist<<dec<<endl;
        }
        rfi->fail = true; //FAIL to route flit
        goto end_DyBM;
    }
    
    if (rfi->last_back) {       // try to move back
        if (source_id == id) {  // we are at start point, no way back left. BAD
            if (LOG >= 5)
                eventlog<<"id = "<<id<<" dest_id = "<<dest_id<<" rfi->last_back && source_id == id"<<endl;
            rfi->fail = true; //FAIL to route flit
            goto end_DyBM;
        }
        
        if (rfi->last_dir == ND) { // last dir code was ND, strange situation.
            if (LOG >= 5)
                eventlog<<"id = "<<id<<" dest_id = "<<dest_id<<" rfi->last_dir == ND"<<endl;
            rfi->fail = true; //FAIL to route flit
            goto end_DyBM;
        }
        
        switch (rfi->last_dir) { // select back choise direction, based on even flag and adaptive flag of last forward move
            case 0: {
                if (state.typeID) {
                    if (last_adap)
                        back_choise = W;
                    else
                        back_choise = N;
                }
                else {
                    if (last_adap)
                        back_choise = N;
                    else
                        back_choise = W;
                }
            }; break;
            case 1: {
                if (state.typeID) {
                    if (last_adap)
                        back_choise = N;
                    else
                        back_choise = E;
                }
                else {
                    if (last_adap)
                        back_choise = E;
                    else
                        back_choise = N;
                }
            }; break;
            case 2: {
                if (state.typeID) {
                    if (last_adap)
                        back_choise = S;
                    else
                        back_choise = W;
                }
                else {
                    if (last_adap)
                        back_choise = W;
                    else
                        back_choise = S;
                }
            }; break;
            case 3: {
                if (state.typeID) {
                    if (last_adap)
                        back_choise = E;
                    else
                        back_choise = S;
                }
                else {
                    if (last_adap)
                        back_choise = S;
                    else
                        back_choise = E;
                }
            }; break;
        }   
        
        if (state.faultDir[back_choise]) { // we can't go back - FAIL
            if (LOG >= 5)
                eventlog<<"id = "<<id<<" dest_id = "<<dest_id<<" state.faultDir[back_choise]"<<endl;
            rfi->fail = true; //FAIL to route flit
            goto end_DyBM;
        }
        else {
            deb_res = back_choise;
            rfi->last_back_adap = last_adap;
            hist = hist >> 1;
        }   
    }        

end_DyBM:
    rfi->history = hist;  // update history and choising dir type
    rfi->last_dir = dir;
    if (LOG >= 1)
        if (rfi->fail)    // FAIL to route flit
            eventlog<<"id = "<<id<<" dest_id = "<<dest_id<<" FAIL_FLAG"<<endl;
    return deb_res;
}

////////////////////////////////////////////////
/// Method containing any initializations
/// inherited from base class router
////////////////////////////////////////////////
// may be empty
// definition must be included even if empty, because this is a virtual function in base class
void DyBM_router::initialize() {

}

// for dynamic linking
extern "C" {
router *maker() {
	return new DyBM_router;
}
}
