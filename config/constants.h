
/*
 * constants.h
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

///////////////////////////////////////////////////////
/// \file constants.h
/// \brief Defines constant parameters for NIRGAM
///////////////////////////////////////////////////////

#ifndef _NOC_CONSTANTS_H_
#define _NOC_CONSTANTS_H_

/// debug output log for NIRGAM
//#define DEBUG_NOC

/// current time
#define CURRENT_TIME sc_simulation_time()

/// number of virtual channels
#define NUM_VCS 4

// parameters for topology
/// maximum number of rows
#define MAX_NUM_ROWS 9
/// maximum number of columns
#define MAX_NUM_COLS 9
/// maximum number of tiles
#define MAX_NUM_TILES MAX_NUM_ROWS * MAX_NUM_COLS

// parameters for NWTile
/// number of neighbors of a general tile in mesh/torus
#define NUM_NB 4
/// number of input channels in a general tile in mesh/torus
#define NUM_IC 5
/// number of output channels in a general tile in mesh/torus
#define NUM_OC 5

// parameters for mesh (non-toroidal) topology
/// number of neighbors of a corner tile in mesh
#define NUM_NB_C 2
/// number of input channels in a corner tile in mesh
#define NUM_IC_C 3
/// number of output channels in a corner tile in mesh
#define NUM_OC_C 3

// parameters for mesh (non-toroidal) topology
/// number of neighbors of a border tile in mesh
#define NUM_NB_B 3
/// number of input channels in a border tile in mesh
#define NUM_IC_B 4
/// number of output channels in a border tile in mesh
#define NUM_OC_B 4

// direction ids
/// North direction
#define N 0
/// South direction
#define S 1
/// East direction
#define E 2
/// West direction
#define W 3
/// Core direction
#define C 4
/// not defined
#define ND 5

/// maximum buffer depth
#define	MAX_NUM_BUFS	16

/////////////////////////////////////////
/// types of flits: HEAD, DATA, TAIL, HDT
////////////////////////////////////////
enum flit_type{
	HEAD,
	DATA,
	TAIL,
	HDT
};

/////////////////////////////////////////
/// types of packets: ANT, NOC
////////////////////////////////////////
enum pkt_type {
	ANT,
	NOC
};

////////////////////////////////////////////
/// types of ant packets: FORWARD, BACKWARD
////////////////////////////////////////////
enum ant_type {
	FORWARD,
	BACKWARD
};

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/// types of routing algorithms: SOURCE, XY, OE, DyXy, DyAD_OE, West_First, North_Last, Negative_First, DyBM, DyXY_FT
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
enum routing_type {
	SOURCE,
	XY,
	OE,
    DyXY,
    DyAD_OE,
    West_First,
    North_Last,
    Negative_First,
    DyBM,
    DyXY_FT
};

//////////////////////////////////////////////////////////////////////
/// types of turn routing algorithm choice type: RANDOM, CONGESTION
//////////////////////////////////////////////////////////////////////
enum turn_routing_type {
	RANDOM,
	CONGESTION
};

////////////////////////////////////////////////////
/// types of input arbitry: SEQUENCE, RR, HA, AA
///////////////////////////////////////////////////
enum input_arbitry_type {
	SEQUENCE,
	RR,
    AA
};

/////////////////////////////////////////////////////////////
/// types of request to controller: NONE, ROUTE, UPDATE
////////////////////////////////////////////////////////////
enum request_type {
	NONE,
	ROUTE,
	UPDATE
};

/////////////////////////////////////////
/// types of topology: MESH, TORUS
////////////////////////////////////////
enum topology {
	MESH,
	TORUS
};

#define UI  unsigned int           ///< short alias of unsigned int
#define UL  unsigned long          ///< short alias of unsigned long
#define ULL unsigned long long     ///< short alias of unsigned long long

#define borderN(ID) ((ID) < num_cols)                ///< is tile ID at North border
#define borderS(ID) ((ID) >= num_tiles - num_cols)   ///< is tile ID at South border
#define borderE(ID) (((ID) + 1) % num_cols == 0)     ///< is tile ID at East border
#define borderW(ID) (((ID) % num_cols) == 0)         ///< is tile ID at West border

#define border(ID) (borderN(ID) || borderS(ID) || borderE(ID) || borderW(ID))       ///< is tile ID at any border

#define cornerNW(ID) ((ID) == 0)                    ///< is tile ID at North-West corner
#define cornerNE(ID) ((ID) == num_cols - 1)         ///< is tile ID at North-East corner
#define cornerSW(ID) ((ID) == num_tiles - num_cols) ///< is tile ID at South-West corner
#define cornerSE(ID) ((ID) == num_tiles - 1)        ///< is tile ID at South-East corner

#define corner(ID) (cornerNW(ID) || cornerNE(ID) || cornerSW(ID) || cornerSE(ID))   ///< is tile ID at any corner

#endif
