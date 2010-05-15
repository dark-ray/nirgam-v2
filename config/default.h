
/*
 * default.h
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
/// \file default.h
/// \brief Defines default values for NIRGAM parameters
///////////////////////////////////////////////////////

#ifndef _DEFAULT_H_
#define _DEFAULT_H_

topology TOPO = MESH;			                ///< NoC topology
UI num_rows = 4;			                    ///< number of rows in topology
UI num_cols = 4;			                    ///< number of columns in topology
UI num_tiles = num_rows * num_cols;	            ///< number of tiles in topology

routing_type RT_ALGO  = XY;		                ///< routing algorithm
turn_routing_type TR_TYPE = RANDOM;             ///< turn routing algorithm choice type
input_arbitry_type IAT_TYPE = RR;               ///< type of input arbitry

bool ADDITIONAL_INFO = false;                   ///< output additional information about simulation
UI  LOG = 0;				                    ///< log level
bool MATLAB_MAKE_IMAGES = false;                ///< not just make matlab figs, but also convert them to images

UI NUM_BUFS = 8;	                            ///< buffer depth (number of buffers) in input channel fifo
UI HALF_NUM_BUFS = (NUM_BUFS * NUM_VCS) / 2;    ///< half of maximum numbers of flits that can be placed in buffers
UI FLITSIZE = 5;	                            ///< size of flit in bytes
UI HEAD_PAYLOAD = 1;	                        ///< payload size (in bytes) in head/hdt flit
UI DATA_PAYLOAD = 4;	                        ///< payload size (in bytes) in data/tail flit
UI CONGESTION_LEVEL = (NUM_BUFS / 2);           ///< number of flits in buffer to signal a congestion state
UI CONGESTION_PRIORITY = 1;                     ///< priority level of buffer in congestion state
UI HOP_LEVEL = 0;                               ///< hop level to signal old travelled packet in NoC
bool CONGESTION_AFFECT_VC = false;              ///< congestion state input selection logic affect VCA allocation 
bool CONGESTION_USE = false;                    ///< use congestion state input selection logic
bool HOP_USE = false;                           ///< use max hop input selection logic
bool WRITE_THROUGH_OUTPORT = false;             ///< don't wait output arbitration if r_vc and r_in got free simanteniously

ULL WARMUP = 5;		                            ///< warmup period (in clock cycles)
ULL SIM_NUM = 3000;	                            ///< simulation clock cycles
ULL TG_NUM = 1000;	                            ///< clock cycles until which traffic is generated

double CLK_FREQ = 1;			                ///< clock frequency (in GHz)
double CLK_PERIOD = (1/CLK_FREQ);	            ///< clock period (in ns)

#endif
