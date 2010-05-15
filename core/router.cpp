 
/*
 * router.cpp
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
 * Date:  22.02.2010
 *
 */
 
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/// \file router.cpp
/// \brief Abstract router class, partial implementation
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#include "router.h"
#include "../config/extern.h"

///////////////////////
/// constructor
//////////////////////
router::router() {
    id = 0;
    for (UI i = 0; i < 6; i++)
        dir_arr[i] = 4;
    
    stress_value_arr     = NULL;
    congestion_flags_arr = NULL;
    
    rnum.set_seed((RNG::RNGSources)2, 1); //HEURISTIC_SEED_SOURCE
}

///////////////////////////////////////////////////
/// Method to set router identifier
/// \param id_tile tile ID
///
/// Set router ID than call initialize
//////////////////////////////////////////////////
void router::setID(UI id_tile) {
    id = id_tile;
    
    if (cornerNW(id)) {
        state.faultDir[N] = true;
        state.faultDir[W] = true;
    }
    else if (cornerNE(id)) {
        state.faultDir[N] = true;
        state.faultDir[E] = true;
    }
    else if (cornerSW(id)) {
        state.faultDir[S] = true;
        state.faultDir[W] = true;
    }
    else if (cornerSE(id)) {
        state.faultDir[S] = true;
        state.faultDir[E] = true;
    }
    else if (borderN(id))
        state.faultDir[N] = true;
    else if (borderS(id)) 
        state.faultDir[S] = true;
    else if (borderE(id)) 
        state.faultDir[E] = true;
    else if (borderW(id)) 
        state.faultDir[W] = true;
        
    UI id_x = id / num_cols;
    UI id_y = id % num_cols;
    if (((id_x + id_y) % 2) == 0)
        state.typeID = true;
    else
        state.typeID = false;
    
    initialize();
}

//////////////////////////////////////////////////////////////////
/// Method to make initialization for adaptive routing algorithms
/// \param dir_list array of corresponding directions to port id's
/////////////////////////////////////////////////////////////////
void router::init_adaptive_ability(UI dir_list[6]) {
    if (dir_list != NULL) {
        for (UI i = 0; i < 6; i++)
            dir_arr[i] = dir_list[i];
    }
    else       // Error in simulator design
        eventlog<<"\ntime: "<<sc_time_stamp()<<" tileID = "<<id<<" Error in simulator design: dir_list param of init_adaptive_ability is NULL!"<<endl;
}

/////////////////////////////////////////////////////////////////
/// Method to DyXY-like routing init
/// \param stress_arr array of stress value of near routers fifo's
/////////////////////////////////////////////////////////////////
void router::init_DyXY_routing(UI* stress_arr) {
    if (stress_arr != NULL)
        stress_value_arr = stress_arr;
    else      // Error in simulator design
        eventlog<<"\ntime: "<<sc_time_stamp()<<" tileID = "<<id<<" Error in simulator design: stress_arr param of init_DyXY_routing is NULL!"<<endl;
}

///////////////////////////////////////////////////
/// Method to congestions flags init
/// \param cf_arr pointer to congestion flags array
///////////////////////////////////////////////////
void router::init_congestion_flags(bool* cf_arr) {
    if (cf_arr != NULL)
        congestion_flags_arr = cf_arr;
    else      // Error in simulator design
        eventlog<<"\ntime: "<<sc_time_stamp()<<" tileID = "<<id<<" Error in simulator design: cf_arr param of init_DyAD_OE_routing is NULL!"<<endl;
}

//////////////////////////////////////////////////
/// Method to set router output channel to fail state
/// \param dir output channel direction
/// \param fail new fail state
/////////////////////////////////////////////////
void router::set_router_fail_dir(UI dir, bool fail) {
    if (dir > C)
        return;
    
    state.faultDir[dir] = fail;
    
    #ifdef DEBUG_NOC
        eventlog<<"!! Changing router: "<<id<<" state of direction: "<<dir<<" to: "<<fail<<endl;
    #endif
}

////////////////////////////////////////////////////
/// Method to get router output channel state
/// \param dir output channel direction
/// \return fail state
///////////////////////////////////////////////////
bool router::get_router_fail_dir(UI dir) {
    if (dir > C)
        return false;
    else
        return state.faultDir[dir];
}

////////////////////////////////////////////////////
/// Method to check is router turned off
/// \return router turn off state (yes/no)
///////////////////////////////////////////////////
bool router::is_router_shutdown() {
    return (state.faultDir[N] && state.faultDir[W] && state.faultDir[S] && state.faultDir[E]);
}



