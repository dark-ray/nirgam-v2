
/*
 * extern.h
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

/////////////////////////////////////////////////////////////
/// \file extern.h
/// \brief Defines extern variables available to all modules
/////////////////////////////////////////////////////////////

#ifndef _EXTERN_H_
#define _EXTERN_H_

#include <string>

extern topology TOPO;		                    ///< NoC topology
extern UI num_rows;		                        ///< number of rows in topology
extern UI num_cols;		                        ///< number of columns in topology
extern UI num_tiles;		                    ///< number of tiles in topology
extern sc_clock *nw_clock;	                    ///< pointer to clock

extern routing_type RT_ALGO;	                ///< routing algorithm
extern turn_routing_type TR_TYPE;               ///< turn routing algorithm choice type
extern input_arbitry_type IAT_TYPE;             ///< type of input arbitry
extern UI CONGESTION_LEVEL;                     ///< number of flits in buffer to signal a congestion state
extern UI CONGESTION_PRIORITY;                  ///< priority level of buffer in congestion state
extern UI HOP_LEVEL;                            ///< hop level to signal old travelled packet in NoC
extern bool CONGESTION_AFFECT_VC;               ///< congestion state input selection logic affect VCA allocation
extern bool CONGESTION_USE;                     ///< use congestion state input selection logic
extern bool HOP_USE;                            ///< use max hop input selection logic
extern bool WRITE_THROUGH_OUTPORT;              ///< don't wait output arbitration if r_vc and r_in got free simanteniously

extern bool ADDITIONAL_INFO;                    ///< output additional information about simulation
extern UI LOG;			                        ///< log level (0 - 6)
extern bool MATLAB_MAKE_IMAGES;                 ///< not just make matlab figs, but also convert them to images
extern ULL WARMUP;		                        ///< warmup period (in clock cycles) before traffic generation begins
extern ULL SIM_NUM;		                        ///< total simulation clock cycles
extern ULL TG_NUM;		                        ///< number of clock cycles until which traffic is generated

extern UI NUM_BUFS;		                        ///< buffer depth (number of buffers in i/p channel fifo)
extern UI HALF_NUM_BUFS;                        ///< half of maximum numbers of flits that can be placed in buffers
extern UI FLITSIZE;		                        ///< size of flit (in bytes)
extern UI HEAD_PAYLOAD;	                        ///< payload size (in bytes) in head/hdt flit
extern UI DATA_PAYLOAD;	                        ///< payload size (in bytes) in data/tail flit

extern ofstream eventlog;	                    ///< file stream to log events
extern sc_trace_file* tracefile;                ///< file stream to generate vcd trace
extern ofstream results_log;	                ///< file stream to log results

extern double CLK_FREQ;		                    ///< clock frequency (in GHz)
extern double CLK_PERIOD;	                    ///< clock period (in ns)

extern std::string DIRS_NAMES[6];               ///< dierction names array (N, S, E, W, C, ND)

#endif
 
