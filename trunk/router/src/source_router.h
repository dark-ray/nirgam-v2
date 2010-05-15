
/*
 * source_router.h
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
/// \file source_router.h
/// \brief Defines class to implement source routing algorithm
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#ifndef __src_router__
#define __src_router__

#include "../../core/router.h"

//////////////////////////////////////////////////////////////////////
/// \brief Class to implement source routing algorithm
///
/// inherited from from class router
//////////////////////////////////////////////////////////////////////
class source_router : public router {
	public:
		/// Constructor
		source_router() { }
		
		/// \brief returns next hop for a given source and destination. ip_dir is the incoming direction and rouing fault info
		UI calc_next(UI ip_dir, ULL source_id, ULL dest_id, routing_fault_info* rfi);
		
		void initialize();	///< any initializations to be done
};

#endif

