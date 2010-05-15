
/*
 * RouteInfo.cpp
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
/// \file RouteInfo.cpp
/// \brief Implements Route Information module
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#include "RouteInfo.h"
#include "../config/extern.h"
	
////////////////////////
/// Constructor
////////////////////////
template<UI num_nb>
RouteInfo<num_nb>::RouteInfo(sc_module_name RouteInfo): sc_module(RouteInfo) {

	// process sensitive to RouteInfo update
	SC_THREAD(exec);
    for(UI i = 0; i < num_nb; i++)
    {
		sensitive << in_flit[i].pos();
		sensitive << out_flit[i].pos();
	}
	
	// initialize 
    cur_stress_value = 0;
    
    for(UI i = 0; i < num_nb; i++)
    {
        in_flit_request[i].initialize(false);
        out_flit_request[i].initialize(false);
        stress_value[i].initialize(0);
    }
}

///////////////////////////////////////////////////////////////////////////
/// Process sensitive to incoming reset or flit updates.
/// updates stress value of router
///////////////////////////////////////////////////////////////////////////
template<UI num_nb>
void RouteInfo<num_nb>::exec() {
    
    bool inc_calc = false;
    bool dec_calc = false;
    
    while(true)
    {
        wait();
        for(UI i = 0; i < num_nb; i++)
        {
            if (in_flit[i].posedge())
            {
                in_flit_request[i].write(true);
                inc_calc = true;
                if(LOG >= 4)
                    eventlog<<"\nTime: "<<sc_time_stamp()<<" name: "<<this->name()<<" tile: "<<tileID<<" Increment stress value. Val = "<<cur_stress_value<<endl;
            }
            else
                in_flit_request[i].write(false);

            if (out_flit[i].posedge())
            {
                out_flit_request[i].write(true);
                dec_calc = true;
                if(LOG >= 4)
                    eventlog<<"\nTime: "<<sc_time_stamp()<<" name: "<<this->name()<<" tile: "<<tileID<<" Decrement stress value. Val = "<<cur_stress_value<<endl;
            }
            else
                out_flit_request[i].write(false);
        }
        
        if (inc_calc)
            cur_stress_value++;
            
        if (dec_calc)
            cur_stress_value--;
        
        if (inc_calc || dec_calc)
        {
            for(UI i = 0; i < num_nb; i++)
                stress_value[i].write(cur_stress_value); 
        }       
    }
}//end entry

///////////////////////////////////////////////////////////////////////////
/// Method to assign tile IDs and port IDs
///////////////////////////////////////////////////////////////////////////
template<UI num_nb>
void RouteInfo<num_nb>::setTileID(UI id){
	tileID = id;
}

template struct RouteInfo<NUM_NB>;
template struct RouteInfo<NUM_NB_C>;
template struct RouteInfo<NUM_NB_B>;
