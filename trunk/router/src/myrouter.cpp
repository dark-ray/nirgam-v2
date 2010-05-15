
/*
 * myrouter.cpp
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

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/// \file myrouter.cpp
/// \brief Implements routing algorithm
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#include "myrouter.h"
#include "../../config/extern.h"

////////////////////////////////////////////////
/// Method that implements routing
/// inherited from base class router
/// \param ip_dir input direction from which flit entered the tile
/// \param source_id tileID of source tile
/// \param dest_id tileID of destination tile
/// \param rfi routing fault info
/// \return next hop direction
////////////////////////////////////////////////
UI myrouter::calc_next(UI ip_dir, ULL source_id, ULL dest_id, routing_fault_info* rfi) {
	// insert route logic here
	// return next hop direction(N, S, E, W, C)
	return 0;
}

////////////////////////////////////////////////
/// Method containing any initializations
/// inherited from base class router
////////////////////////////////////////////////
// may be empty
// definition must be included even if empty, because this is a virtual function in base class
void myrouter::initialize() {

}

// for dynamic linking
extern "C" {
router *maker() {
	return new myrouter;
}
}
