
/*
 * DyXY_FT_router.h
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
/// \file DyXY_FT_router.h
/// \brief Defines class to implement DyXY_FT with fault-tolerance routing algorithm
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#ifndef __DyXY_FT_router__
#define __DyXY_FT_router__

#include "../../core/router.h"

//////////////////////////////////////////////////////////////////////
/// \brief Class to implement DyXY_FT with fault-tolerance routing algorithm
///
/// inherited from from class router
//////////////////////////////////////////////////////////////////////
class DyXY_FT_router : public router {
	public:
		/// Constructor
		DyXY_FT_router() { }
		
		/// \brief returns next hop for a given source and destination. ip_dir is the incoming direction and rouing fault info
		UI calc_next(UI ip_dir, ULL source_id, ULL dest_id, routing_fault_info* rfi);
		
		void initialize();	///< any initializations to be done
};

#endif

