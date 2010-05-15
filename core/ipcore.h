
/*
 * ipcore.h
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
/// \file ipcore.h
/// \brief Defines abstract ipcore module
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#ifndef _IPCORE_
#define _IPCORE_

#include "systemc.h"
#include "../config/constants.h"
#include "../config/extern.h"
#include "flit.h"
#include "credit.h"
#include "rng.h"

#include <fstream>
#include <string>
#include <math.h>
#include <dlfcn.h>

using namespace std;

//////////////////////////////////////////////////////////////////////////
/// \brief Module to represent an ipcore
///
/// This module defines an ipcore
//////////////////////////////////////////////////////////////////////////
struct ipcore : public sc_module {

	// PORTS //////////////////////////////////////////////////////////////////////////////////
	sc_in_clk               clock;			                ///< input clock port
	sc_in<flit>             flit_inport;		            ///< input data/flit port
	sc_out<flit>            flit_outport;		            ///< ouput data/flit port
	sc_inout<creditLine>    credit_in[NUM_VCS];	            ///< input ports to recieve credit info (buffer status)
	// PORTS END //////////////////////////////////////////////////////////////////////////////
	
	// Constructor
	SC_CTOR(ipcore);

	// FUNCTIONS /////////////////////////////////////////////////////////////////////////////
	void entry();			        ///< process to keep track of simulation count in the module, sensitive to clock
	void recv();	                ///< process to recieve flits, sensitive to clock and input flit port
	void send();	                ///< process to send flits, sensitive to clock
	virtual void recv_app() = 0;	///< abstract process to recieve flits, redefine at app. level
	virtual void send_app() = 0;	///< abstract process to send flits, redefine at app. level
	
	void setID(UI tileID);		    ///< sets tileID
	/// create a hdt flit with given packet id, flit id and destination
	flit* create_hdt_flit(int pkt_id, int flit_id, UI route_info);
	/// create a head flit with given packet id, flit id and destination
	flit* create_head_flit(int pkt_id, int flit_id, UI route_info);
	/// create a data flit with given packet id and flit id
	flit* create_data_flit(int pkt_id, int flit_id);
	/// create a tail flit with given packet id and flit id
	flit* create_tail_flit(int pkt_id, int flit_id);	
	
	/// sets command field of flit equal to given value
	void set_cmd(flit*, int cmd_value);
	/// sets integer data field of flit equal to given value
	void set_data(flit*, int data_int_value);
	/// sets string data field of flit equal to given value
	void set_data(flit*, string data_string_value);
	/// sets command field and integer data field of flit equal to given values
	void set_payload(flit*, int cmd_value, int data_int_value);
	/// sets command field and string data field of flit equal to given values
	void set_payload(flit*, int cmd_value, string data_string_value);

	/// return command field of flit in second parameter
	void get_cmd(flit, int &cmd_value);
	/// return integer data field of flit in second parameter
	void get_data(flit, int &data_int_value);
	/// return string data field of flit in second parameter
	void get_data(flit, string &data_string_value);
	/// return command field and integer data field of flit in second and third parameter respectively
	void get_payload(flit, int &cmd_value, int &data_int_value);
	/// return command field and string data field of flit in second and third parameter respectively
	void get_payload(flit, int &cmd_value, string &data_string_value);
	/// return a randomly chosen destination
	UI   get_random_dest();
    
    bool set_creating_flits_state(UI toTileID, bool grant); ///< change ip core state of generating flit
    
    void closeLogs(); ///< log functions
    
	// FUNCTIONS END /////////////////////////////////////////////////////////////////////////
	
	// VARIABLES /////////////////////////////////////////////////////////////////////////////
	UI      tileID;	                                ///< unique tile id
	ULL     sim_count;	                            ///< number of clock cycles
    ULL     total_latency;			                ///< total latency
    ULL     total_packets_recived;	                ///< total number of packets
	ULL     total_flits_recived;	                ///< total number of flits
    ULL     time_first_flit_in;		                ///< time when first flit was generated at this tile
    ULL     time_last_flit_in;	                    ///< time when last flit was generated at this tile
    ULL     total_cycles;                           ///< all working time from first generated flit to last
    ULL     num_pkts_gen;                           ///< number of generated packets
    ULL     num_flits_gen;                          ///< number of generated flits
	ULL     wc_num_waits;                           ///< worst-case number of waited clocks by flit
	ULL     num_waits;                              ///< all number of waited clocks by flit
	ULL     wc_num_sw;                              ///< worst-case number of hops traversed by flit
	ULL     num_sw;                                 ///< all number of hops traversed by flit
	double  avg_num_waits;                          ///< average number of waited clocks by flit
	double  avg_num_sw;                             ///< average number of hops traversed by flit
	double  wc_latency;                             ///< worst-case flit latency 
    double  avg_latency;		                    ///< average latency (in clock cycles) per packet
	double  avg_latency_flit;		                ///< average latency (in clock cycles) per flit
	double  avg_throughput;		                    ///< average throughput (in Gbps)
    bool    accept_destinations[MAX_NUM_TILES];     ///< destination to which flits can be generated
	RNG     *ran_var;	                            ///< random variable generator
	// VARIABLES END /////////////////////////////////////////////////////////////////////////
};

#endif
