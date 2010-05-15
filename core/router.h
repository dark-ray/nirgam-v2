
/*
 * router.h
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

////////////////////////////////////////////////////////////////////////////////////////////////
/// \file router.h
/// \brief Defines abstract router class
////////////////////////////////////////////////////////////////////////////////////////////////

#ifndef __noc_router__
#define __noc_router__

#include "systemc.h"
#include "../config/constants.h"
#include "rng.h"
#include "router_state.h"
#include "routing_fault_info.h"
#include <string>
#include <vector>
#include <time.h>

///////////////////////////////////////////////////////////////////////////
/// \brief Abstract router class 
/// 
/// classes implementing routing algorithms are derived from this class)
///////////////////////////////////////////////////////////////////////////
class router {
	protected:	
		UI              id;	                    ///< unique tile identifier
        RNG             rnum;		            ///< random number generator
        UI              dir_arr[6];             ///< array of corresponding directions to port id's
        UI*             stress_value_arr;       ///< pointer to stress values from adjancet routers for DyXY-like routing algorithm
        bool*           congestion_flags_arr;   ///< pointer to congestion flags from adjancent routers
        router_state    state;                  ///< router state info
        
	public:
		router();   ///< constructor
        
		/// \brief virtual function that implements routing 
		virtual UI calc_next(UI ip_dir, ULL src_id, ULL dst_id, routing_fault_info* rfi) = 0;
        
		/// \brief virtual function to perform some initialization in routing algorithm
		virtual void initialize() = 0;
        
		void setID(UI);                             ///< function to set identifier
        void init_adaptive_ability(UI dir_list[6]); ///< init for adaptive routing algorithms
        void init_DyXY_routing(UI* stress_arr);     ///< DyXY-like algorithm init
        void init_congestion_flags(bool* cf_arr);   ///< congestions flags init
        void set_router_fail_dir(UI dir, bool fail);///< set router output channel to fail state
        bool get_router_fail_dir(UI dir);           ///< get router output channel state
        bool is_router_shutdown();                  ///< check is router turned off
};

#endif
