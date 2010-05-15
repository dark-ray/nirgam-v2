
/*
 * source_router.cpp
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
/// \file source_router.cpp
/// \brief Implements source routing algorithm
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#include "source_router.h"
#include "../../config/extern.h"

////////////////////////////////////////////////
/// Method that implements source routing
/// inherited from base class router
/// \param ip_dir input direction from which flit entered the tile
/// \param source_id tileID of source tile
/// \param dest_id tileID of destination tile
/// \param rfi routing fault info
/// \return next hop direction
////////////////////////////////////////////////
UI source_router::calc_next(UI ip_dir, ULL source_id, ULL rt_code, routing_fault_info* rfi) {
	// return rightmost 3 bits of route code
	return ((rt_code)&(7));
}

////////////////////////////////////////////////
/// Method containing any initializations
/// inherited from base class router
////////////////////////////////////////////////
// may be empty
// definition must be included even if empty, because this is a virtual function in base class
void source_router::initialize() {

}

// for dynamic linking
extern "C" {
router *maker() {
	return new source_router;
}
}
