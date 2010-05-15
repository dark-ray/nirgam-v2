
/*
 * Sink.cpp
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
/// \file Sink.cpp
/// \brief Implements sink application to recieve flits
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#include "Sink.h"
#include "../../config/extern.h"

////////////////////////////////////////////////
/// Constructor
////////////////////////////////////////////////
Sink::Sink(sc_module_name Sink): ipcore(Sink) {

}

////////////////////////////////////////////////
/// Thread sensitive to clock
/// - inherited from ipcore
////////////////////////////////////////////////
// you should include the process definition even if it is empty
void Sink::send_app() {
}

////////////////////////////////////////////////
/// Thread sensitive to clock and inport event
/// - inherited from ipcore
/// - recieve incoming flits
/// - assign arrival timestamps
////////////////////////////////////////////////
void Sink::recv_app() {
	wait();	// wait until inport event
}

// for dynamic linking
extern "C" {
ipcore *maker() {
	return new Sink("Sink");
}
}
