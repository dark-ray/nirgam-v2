
/*
 * App_sample.cpp
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

//////////////////////////////////////////////////////////////////
/// \file App_sample.cpp
/// \brief Sample cpp file for application
/////////////////////////////////////////////////////////////////

#include "App_sample.h"
#include "../../config/extern.h"

////////////////////////////////////////////////
/// Constructor
////////////////////////////////////////////////
App_sample::App_sample(sc_module_name App_sample): ipcore(App_sample) {
}

////////////////////////////////////////////////
/// Thread sensitive to clock
/// - inherited from ipcore
////////////////////////////////////////////////
void App_sample::send_app() {
	// Insert code to create and send flits here.
}

////////////////////////////////////////////////
/// Thread sensitive to clock and inport event
/// - inherited from ipcore
/// - recieve incoming flits
/// - assign arrival timestamps
////////////////////////////////////////////////
void App_sample::recv_app() {
	// Insert code to recieve flits here
}

// for dynamic linking
extern "C" {
ipcore *maker() {
	return new App_sample("App_sample");
}
}
