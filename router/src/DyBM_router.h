 
/*
 * DyBM_router.h
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
/// \file DyBM_router.h
/// \brief Defines class to implement DyBM routing algorithm
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#ifndef __DyBM_router__
#define __DyBM_router__

#include "../../core/router.h"

//////////////////////////////////////////////////////////////////////
/// \brief Class to implement DyBM routing algorithm
///
/// inherited from from class router
//////////////////////////////////////////////////////////////////////
class DyBM_router : public router {
	public:
		/// Constructor
		DyBM_router() { }
        
        UI get_fault_dir(UI src, UI dst, UI back_dir); ///< calculation subalgorithm type
        
		/// \brief returns next hop for a given source and destination. ip_dir is the incoming direction and rouing fault info
		UI calc_next(UI ip_dir, ULL source_id, ULL dest_id, routing_fault_info* rfi);
		
		void initialize();	///< any initializations to be done
};

#endif

