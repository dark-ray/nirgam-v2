
/*
 * App_sample.h
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
/// \file App_sample.h
/// \brief Sample header file for an application
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#ifndef _App_sample_H_
#define _App_sample_H_

#include "../../core/ipcore.h"
#include <fstream>
#include <string>
#include <math.h>

using namespace std;

//////////////////////////////////////////////////////////////////////
/// \brief Module to define sample application
///
/// - Module derived from ipcore
/// - Also inherits send and recv processes
//////////////////////////////////////////////////////////////////////
struct App_sample : public ipcore {
	
	/// Constructor
	SC_CTOR(App_sample);
	
	// PROCESSES /////////////////////////////////////////////////////
	void send_app();			///< send flits
	void recv_app();			///< recieve flits
	// Define your processes here
	// PROCESSES END /////////////////////////////////////////////////////
};

#endif
