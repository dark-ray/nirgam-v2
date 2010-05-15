
/*
 * VCAllocator.cpp
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

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/// \file VCAllocator.cpp
/// \brief Implements virtual channel allocator
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#include "VCAllocator.h"
#include "../config/extern.h"
	
////////////////////////
/// constructor
////////////////////////
template<UI num_ip>
VCAllocator<num_ip>::VCAllocator(sc_module_name VCAllocator): sc_module(VCAllocator) {

	// process sensitive to VC request, calls VC allocation
	SC_THREAD(allocate_VC);
	for(UI i = 0; i < num_ip; i++)
		sensitive << vcRequest[i];
	
	// process sensitive to credit info, updates credit status
	SC_THREAD(update_credit);
	for(UI i = 0; i < num_ip; i++){
		for(UI j = 0; j < NUM_VCS; j++) {
			sensitive << Icredit[i][j];
		}
	}
	
	// initialize credit status
	for(UI i = 0; i < num_ip; i++){
		for(UI j = 0; j < NUM_VCS; j++) {
			vcFree[i][j] = true;
		}
	}
}

///////////////////////////////////////////////////////////////////////////
/// Process sensitive to incoming credit information.
/// updates credit info (buffer status) of neighbor tiles
///////////////////////////////////////////////////////////////////////////
template<UI num_ip>
void VCAllocator<num_ip>::update_credit() {
	while(true) {
		wait();	// wait until change in credit info
		for(UI i = 0; i < num_ip; i++) {
			for(UI j = 0; j < NUM_VCS; j++) {
				if(Icredit[i][j].event()) {
					vcFree[i][j] = Icredit[i][j].read().freeVC;	// update credit
					if(LOG >= 4)
						eventlog<<"\nTime: "<<sc_time_stamp()<<" name: "<<this->name()<<" tile: "<<tileID<<" Credit change in "<<i<<" dir, vc num: "<<j<<" status: "<<vcFree[i][j]<<endl;
				}
			}//end for
		}
	}//end while
}//end entry

///////////////////////////////////////////////////////////////////////////
/// Process sensitive to incoming request for virtual channel allocation
/// - reads request
/// - allocates virtual channel in the requested channel on neighbor tile
/// - returns allocated vcid and ready signal
///////////////////////////////////////////////////////////////////////////
template<UI num_ip>
void VCAllocator<num_ip>::allocate_VC() {
	while(true) {
		wait();		// wait until request from any IC
		for(UI i = 0; i < num_ip; i++) {
			if(vcRequest[i].event() && vcRequest[i].read() == true) {
				// read output direction in which VC is requested
				UI dir = opRequest[i].read();
				UI nextvc = NUM_VCS + 1;
                
                // get next VC, parameters: o/p direction requested, i/p direction from which request recieved
                if (CONGESTION_USE && CONGESTION_AFFECT_VC)
                    nextvc = getNextVCID_AA(dir,i);
                else
                    nextvc = getNextVCID(dir,i);
				
				if(LOG >= 4) 
					eventlog<<"\nTime: "<<sc_time_stamp()<<" name: "<<this->name()<< " VA: got this next vc " << nextvc << endl;
				
				// send ready signal and nextVCID to IC
				vcReady[i].write(true);
				nextVCID[i].write(nextvc);
			}
			if(vcRequest[i].event() && vcRequest[i].read() == false) {
				vcReady[i].write(false);
			}
		}
	} //end while
} //end allocate_VC

//////////////////////////////////////////////////////////////////////////////
/// Method that implements virtual channel allocation. 
/// \param dir o/p direction (neighbor tile) in which virtual channel is requested
/// \param dir_from i/p direction (IC) from which request is recieved
/// \return allocated VC id
//////////////////////////////////////////////////////////////////////////////
template<UI num_ip>
UI VCAllocator<num_ip>::getNextVCID (UI dir, UI dir_from) {	
	for(UI i = 0; i < NUM_VCS; i++) {
		if(vcFree[dir][i]) {
			vcFree[dir][i] = false;
			return i;
		}
	}
	if(LOG >= 3) 
		eventlog<<"\nTime: "<<sc_time_stamp()<<" name: "<<this->name()<<" VA: did not find a free VC at "<<dir<<endl;
	return NUM_VCS + 1;
}//end getnextVCID

//////////////////////////////////////////////////////////////////////////////
/// Method that implements virtual channel allocation with respect adaptive arbitry
/// \param dir o/p direction (neighbor tile) in which virtual channel is requested
/// \param dir_from i/p direction (IC) from which request is recieved
/// \return allocated VC id
//////////////////////////////////////////////////////////////////////////////
template<UI num_ip>
UI VCAllocator<num_ip>::getNextVCID_AA (UI dir, UI dir_from) {	
	for(UI i = 0; i < NUM_VCS; i++) {                     // not congested free VC search
		if(vcFree[dir][i] && !congestion_status_in[dir][i]) {
			vcFree[dir][i] = false;
			return i;
		}
	}
    
    for(UI i = 0; i < NUM_VCS; i++) {                     // any free VC search
		if(vcFree[dir][i]) { 
			vcFree[dir][i] = false;
			return i;
		}
	}    
    
	if(LOG >= 3)                                          // searches FAIL
		eventlog<<"\nTime: "<<sc_time_stamp()<<" name: "<<this->name()<<" VA: did not find a free VC at "<<dir<<endl;
	return NUM_VCS + 1;
}

///////////////////////////////////////////////////////////////////////////
/// Method to assign tile IDs and port IDs
/// \param id tile ID
/// \param port_N id corresponding to North direction
/// \param port_S id corresponding to South direction
/// \param port_E id corresponding to East direction
/// \param port_W id corresponding to West direction
///////////////////////////////////////////////////////////////////////////
template<UI num_ip>
void VCAllocator<num_ip>::setTileID(UI id, UI port_N, UI port_S, UI port_E, UI port_W){
	tileID = id;
	portN = port_N;
	portS = port_S;
	portE = port_E;
	portW = port_W;
}

template struct VCAllocator<NUM_IC>;
template struct VCAllocator<NUM_IC_C>;
template struct VCAllocator<NUM_IC_B>;
