
/*
 * Controller.cpp
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
 * Author: Lavina Jain
 *
 */
 /*
 * Portions of changes by Alexander Rumyanthev (darkstreamray@gmail.com) SPbSU ITMO 2010.
 */

//////////////////////////////////////////////////////////////////////////////////////////////////
/// \file Controller.cpp
/// \brief Implements routing
//////////////////////////////////////////////////////////////////////////////////////////////////

#include "Controller.h"
#include "../config/extern.h"

//////////////////////////////////////////////////////////////
/// SystemC constructor 
///
/// - attach corresponding roting algorith
/// - initialize local variables and systemC sensetive lists
//////////////////////////////////////////////////////////////
template<UI num_nb, UI num_ip>
Controller<num_nb, num_ip>::Controller(sc_module_name Controller): sc_module(Controller) {
	
	void *hndl;
	void *mkr;
	
	string libname = string("./router/lib/");
	
	switch(RT_ALGO) {
		case OE: 
			libname = libname + string("OE_router.so");
			break;
		case XY:
			libname = libname + string("XY_router.so");
			break;
		case SOURCE:
			libname = libname + string("source_router.so");
			break;
        case DyXY:
			libname = libname + string("DyXY_router.so");
			break;
        case DyAD_OE:
			libname = libname + string("DyAD_OE_router.so");
			break;
        case West_First:
			libname = libname + string("West_First_router.so");
			break;
        case North_Last:
			libname = libname + string("North_Last_router.so");
			break;
        case Negative_First:
			libname = libname + string("Negative_First_router.so");
			break;
        case DyBM:
			libname = libname + string("DyBM_router.so");
			break;
        case DyXY_FT:
			libname = libname + string("DyXY_FT_router.so");
			break;
        default:
			libname = libname + string("XY_router.so");
			break;
	}
	
    hndl = dlopen(libname.c_str(), RTLD_NOW);
    if(hndl == NULL) {
        cerr << dlerror() << endl;
        exit(-1);
    }
    mkr = dlsym(hndl, "maker");
    rtable = ((router*(*)())(mkr))();
		
	SC_THREAD(allocate_route);
	for(UI i = 0; i < num_ip; i++)
		sensitive << rtRequest[i];
        
    SC_METHOD(update_stress_value);
    for(UI i = 0; i < num_nb; i++)
		sensitive << stress_value[i];
    
    SC_METHOD(update_congestion_flags);
    for(UI i = 0; i < num_nb; i++)
		sensitive << congestion_flag[i];
        
    for (UI i = 0; i < num_nb; i++) {
        stress_value_arr[i] = 0;
        congestion_flags_arr[i] = false;
    }
    
    for (UI i = num_nb; i < 5; i++) {
        stress_value_arr[i] = std::numeric_limits<unsigned int>::max();   
        congestion_flags_arr[i] = true;
    } 
        
    if (LOG >= 4) {
        for (UI i = 0; i < 5; i++) {
            if (RT_ALGO == DyXY)
                eventlog<<"\ntime: "<<sc_time_stamp()<<" name: "<<this->name()<<" stress_value_arr["<<i<<"] = "<<stress_value_arr[i]<<endl;
            
            eventlog<<"\ntime: "<<sc_time_stamp()<<" name: "<<this->name()<<" congestion_flags_arr["<<i<<"] = "<<congestion_flags_arr[i]<<endl;
        }
    }
}

///////////////////////////////////////////////////////////////////////////
/// allocation route systemC thread 	
///
/// - Process sensitive to route request
/// - Calls routing algorithm
///////////////////////////////////////////////////////////////////////////
template<UI num_nb, UI num_ip>
void Controller<num_nb, num_ip>::allocate_route() {
	while(true) {
		wait();
		for(UI i = 0; i < num_ip; i++) {
			if(rtRequest[i].event() && rtRequest[i].read() == ROUTE) { // routing request
				UI src = sourceAddress[i].read();
				UI dest = destRequest[i].read();
				UI ip_dir = idToDir(i);
                routing_fault_info rfi = faultInfoIn[i].read();
                UI op_dir = rtable->calc_next(ip_dir, src, dest, &rfi);
                
                faultInfoOut[i].write(rfi);               
				rtReady[i].write(true);
				nextRt[i].write(op_dir);
			}
			// request from IC to update //////////////////////////
			if(rtRequest[i].event() && rtRequest[i].read() == UPDATE) {
				UI src = sourceAddress[i].read();
				UI dest = destRequest[i].read();
				// rtable.update(dest, i); 
				rtReady[i].write(true);
			}
			if(rtRequest[i].event() && rtRequest[i].read() == NONE) { // nothing to do
				rtReady[i].write(false);
			}
		}
	}// end while
}// end allocate_route

///////////////////////////////////////////////////////////////////////////
/// update stress value systemC method
///////////////////////////////////////////////////////////////////////////
template<UI num_nb, UI num_ip>
void Controller<num_nb, num_ip>::update_stress_value() {
    for(UI i = 0; i < num_nb; i++)
        if (stress_value[i].event()) {
            stress_value_arr[i] = stress_value[i].read();
        }
}

///////////////////////////////////////////////////////////////////////////
/// update congestion flags systemC method
///////////////////////////////////////////////////////////////////////////
template<UI num_nb, UI num_ip>
void Controller<num_nb, num_ip>::update_congestion_flags() {
    for(UI i = 0; i < num_nb; i++)
        if (congestion_flag[i].event()) {
            congestion_flags_arr[i] = congestion_flag[i].read();
        }
}

///////////////////////////////////////////////////////////////////////////
/// sets tile ID and id corresponding to port directions
/// \param id tile ID
/// \param port_N id corresponding to North direction
/// \param port_S id corresponding to South direction
/// \param port_E id corresponding to East direction
/// \param port_W id corresponding to West direction
///
/// - assign tile ID
/// - init adaptive roting
/// - init if needed DyXY and DyXY_FT
/// - init congestion flags
///////////////////////////////////////////////////////////////////////////
template<UI num_nb, UI num_ip>
void Controller<num_nb, num_ip>::setTileID(UI id, UI port_N, UI port_S, UI port_E, UI port_W) {
	tileID = id;
	portN = port_N;
	portS = port_S;
	portE = port_E;
	portW = port_W;
	rtable->setID(id);
    
    //adative routing init
    UI dir_list[6];
    for(UI i = 0; i < 6; i++)
        dir_list[i] = 4;
    for(UI i = 0; i < num_ip; i++) {
        dir_list[idToDir(i)] = i;
        if(LOG >= 4)
            eventlog<<"\ntime: "<<sc_time_stamp()<<" name: "<<this->name()<<" tileID:"<<tileID<<" dir_list["<<idToDir(i)<<"] = "<<i<<endl;
    }
    rtable->init_adaptive_ability(dir_list);
    
    //DyXY && DyXY_FT init
    if ((RT_ALGO == DyXY) || (RT_ALGO == DyXY_FT))
        rtable->init_DyXY_routing(stress_value_arr);
        
    //init
    rtable->init_congestion_flags(congestion_flags_arr);
}

/////////////////////////////////////////////////////////////////////
/// returns direction (N, S, E, W) corresponding to a given port id
/// \param dir direction 
/// \return port id for that direction
////////////////////////////////////////////////////////////////////
template<UI num_nb, UI num_ip>
UI Controller<num_nb, num_ip>::idToDir(UI dir) {
	if(dir == portN)
		return N;
	else if(dir == portS)
		return S;
	else if(dir == portE)
		return E;
	else if(dir == portW)
		return W;
	return C;
}

/////////////////////////////////////////////////////////////////////
/// returns id corresponding to a given port direction (N, S, E, W)
/// \param port_id port id
/// \return corresponding direction
////////////////////////////////////////////////////////////////////
template<UI num_nb, UI num_ip>
UI Controller<num_nb, num_ip>::dirToId(UI port_dir) {
	switch(port_dir) {
		case N: return portN;
			break;
		case S: return portS;
			break;
		case E: return portE;
			break;
		case W: return portW;
			break;
		case C: return num_ip - 1;
			break;
	}
	return num_ip - 1;
}

///////////////////////////////////////////////////////////////////
/// changes router's output channel state to fail or working
/// \param dir output channel direction
/// \param fail new state (true - fail, false - working)
///////////////////////////////////////////////////////////////////
template<UI num_nb, UI num_ip>
void Controller<num_nb, num_ip>::set_router_fail_dir(UI dir, bool fail) {
    rtable->set_router_fail_dir(dir, fail);
}

///////////////////////////////////////////////////////////////////
/// gets router's output channel state
/// \param dir output channel direction
/// \return state (true - fail, false - working)
//////////////////////////////////////////////////////////////////
template<UI num_nb, UI num_ip>
bool Controller<num_nb, num_ip>::get_router_fail_dir(UI dir) {
    return rtable->get_router_fail_dir(dir);
}

////////////////////////////////////////////////////
/// Method to check is router turned off
/// \return router turn off state (yes/no)
///////////////////////////////////////////////////
template<UI num_nb, UI num_ip>
bool Controller<num_nb, num_ip>::is_router_shutdown() {
    return rtable->is_router_shutdown();
}

template struct Controller<NUM_NB, NUM_IC>;
template struct Controller<NUM_NB_B, NUM_IC_B>;
template struct Controller<NUM_NB_C, NUM_IC_C>;
