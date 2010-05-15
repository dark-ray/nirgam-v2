
/*
 * NoC.h
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
/// \file NoC.h
/// \brief Defines network topology
///
/// This file defines:
/// - module NoC, a 2-dimentional topology of network tiles.
/// - structure signals, a set of signals to connect tiles in the network.
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#ifndef __NOC__
#define __NOC__

#include <time.h>
#include "NWTile.h"
#include "../config/extern.h"

///////////////////////////////////////////////
/// \brief signals to connect neighboring tiles
///////////////////////////////////////////////
struct signals {
	sc_signal<flit> sig_toS;			        ///< data line (flit line) from a tile to its South neighbor
	sc_signal<flit> sig_fromS;			        ///< data line (flit line) to a tile from its South neighbor
	sc_signal<flit> sig_fromE;			        ///< data line (flit line) to a tile from its East neighbor
	sc_signal<flit> sig_toE;			        ///< data line (flit line) from a tile to its East neighbor

	sc_signal<creditLine> cr_sig_toS[NUM_VCS];	///< credit line (transmits buffer status) per virtual channel from a tile to its South neighbor
	sc_signal<creditLine> cr_sig_fromS[NUM_VCS];///< credit line (transmits buffer status) per virtual channel to a tile from its South neighbor
	sc_signal<creditLine> cr_sig_fromE[NUM_VCS];///< credit line (transmits buffer status) per virtual channel to a tile from its East neighbor
	sc_signal<creditLine> cr_sig_toE[NUM_VCS];	///< credit line (transmits buffer status) per virtual channel from a tile to its East neighbor

    sc_signal<UI> sv_sig_toS;                   ///< stress values line from a tile to its South neighbor
    sc_signal<UI> sv_sig_fromS;                 ///< stress values line to a tile from its South neighbor 
    sc_signal<UI> sv_sig_fromE;                 ///< stress values line to a tile from its East neighbor 
    sc_signal<UI> sv_sig_toE;                   ///< stress values line from a tile to its East neighbor 
    
    sc_signal<bool> cf_sig_toS;                 ///< congestion flag line from a tile to its South neighbor
    sc_signal<bool> cf_sig_fromS;               ///< congestion flag line to a tile from its South neighbor
    sc_signal<bool> cf_sig_fromE;               ///< congestion flag line to a tile from its East neighbor
    sc_signal<bool> cf_sig_toE;                 ///< congestion flag line from a tile to its East neighbor
    
    sc_signal<bool> cs_sig_toS[NUM_VCS];        ///< congestion state line from a tile to its South neighbor
    sc_signal<bool> cs_sig_fromS[NUM_VCS];      ///< congestion state line to a tile from its South neighbor
    sc_signal<bool> cs_sig_fromE[NUM_VCS];      ///< congestion state line to a tile from its East neighbor
    sc_signal<bool> cs_sig_toE[NUM_VCS];        ///< congestion state line from a tile to its East neighbor
};

////////////////////////////////////////////////////////////////////////////////////////////////////////
/// \brief Represents the entire Network-on-Chip
///
/// This module represents the NoC. It implements an array of network tiles and connectivity among them.
////////////////////////////////////////////////////////////////////////////////////////////////////////
struct NoC : public sc_module {

	sc_in_clk switch_cntrl;	    ///< clock input port

	/// \brief constructor
	SC_HAS_PROCESS(NoC);
	NoC(sc_module_name NoC, UI num_rows, UI num_cols, bool isProgBar = true);
	
	UI rows;	                ///< number of rows in topology
	UI cols;	                ///< number of columns in topology
	
	BaseNWTile	*nwtile[MAX_NUM_ROWS][MAX_NUM_COLS];	///< A 2-d array of network tiles
	signals		sigs[MAX_NUM_ROWS][MAX_NUM_COLS];	    ///< Signals to interconnect network tiles
    
    ULL  sim_count;             ///< NoC simulation ticks count
    bool drawProgressBar;       ///< Draw progress bar or not
    
	void entry();	            ///< Keeps count of number of simulation cycles
    
    bool set_router_fail(UI tileID);                                                    ///< set router to fail
    bool set_router_fail_dir(UI tileID, UI dir, bool fail, bool ack_like_real = true);  ///< set router output channel to fail
    
    //internal functions
    bool turn_off_ipcore(UI tileID);                    ///< turn off certain ip core
    bool turn_off_ipcore_and_links(UI tileID);          ///< turn off certain overall tile and channels to it
    bool is_turn_off_tile(UI x, UI y);                  ///< need to turn off tile (x,y)
    void progress_bar_draw(double total_to_count, double now_count, UI total_dotz); ///< function that draw progress bar
};

#endif
