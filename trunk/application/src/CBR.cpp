
/*
 * CBR.cpp
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
/// \file CBR.cpp
/// \brief Implements CBR (constant bit rate) traffic generator
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#include "CBR.h"
#include "../../config/extern.h"

////////////////////////////////////////////////
/// Constructor
////////////////////////////////////////////////
CBRTraffic::CBRTraffic(sc_module_name CBRTraffic) : TrafficGenerator(CBRTraffic)
{
	// clear traffic log
	system("rm -f log/traffic/*");
	// initialize random number generator
	//rnum = new RNG((RNG::RNGSources)2, 1); //HEURISTIC_SEED_SOURCE
	char str_id[3];
	ofstream trafstream;
	
	// create new traffic log file
	for(int i = 0; i < num_tiles; i++) {
		sprintf(str_id, "%d", i);
		string traffic_filename = string("log/traffic/tile-") + string(str_id);
		trafstream.open(traffic_filename.c_str());
		trafstream.close();
	}
	
	// thread sensitive to clock
	SC_CTHREAD(init, clock.pos());
}

////////////////////////////////////////////////
/// Process to generate CBR traffic
/// - read traffic configuration file
/// - generate traffic pattern in log file
////////////////////////////////////////////////
void CBRTraffic::init()
{
	// open traffic config file
	char str_id[3];
	sprintf(str_id, "%d", tileID);
	string traffic_filename = string("config/traffic/tile-") + string(str_id);
	ifstream instream;
	instream.open(traffic_filename.c_str());

	while(!instream.eof()) {
		string field;
		instream >> field;
		if(field == "PKT_SIZE") {	// read packet size
			int value; instream >> value; pkt_size = value;
		}
		else if(field == "LOAD") {	// read load percentage
			int value; instream >> value; load = value;
		}
		else if(field == "DESTINATION") {
			instream >> dst_type;	// read destination type
			if(dst_type == "FIXED") {	// if fixed destination, read destination tileID or route code
				int value; instream >> value; route_info = value;
			}
		}
		else if(field == "FLIT_INTERVAL") {	// read inter-flit interval
			int value; instream >> value; flit_interval = value;
		}
	}
	
	instream.close();

	// compute number of flits in packet (function of pkt size and flit size)
	num_flits = (int)((ceil)((double)(pkt_size - HEAD_PAYLOAD)/(DATA_PAYLOAD)) + 1);
	// compute inter-packet interval
	pkt_interval = (int)((ceil)(100.0/load) * num_flits * cycles_per_flit);
	
	// open traffic log file
	string dest_type;
	traffic_filename = string("log/traffic/tile-") + string(str_id);
	ofstream trafstream;
	trafstream.open(traffic_filename.c_str());
	// write inter-flit interval
	trafstream<<"FLIT_INTERVAL: "<<flit_interval<<endl;
	
    ULL total_time = WARMUP;
    int next_pkt_time;
    
    while (total_time <= TG_NUM)
    {
        next_pkt_time = (int)next_interval();	// get next packet interval
        total_time += (ULL)next_pkt_time;
        trafstream<<"NEXT_PKT_INTERVAL: "<<next_pkt_time<<endl;	// write next packet interval
		if(dst_type == "RANDOM")
			route_info = get_random_dest();		// get random destination
		trafstream<<"NUM_FLITS: "<<num_flits<<endl;	// write number of flits
		trafstream<<"DESTINATION: "<<route_info<<endl;	// write destination ID or route code        
    }
    
    /*
	for(int i = WARMUP; i <= TG_NUM; i++) {
		next_pkt_time = (int)next_interval();	// get next packet interval
		trafstream<<"NEXT_PKT_INTERVAL: "<<next_pkt_time<<endl;	// write next packet interval
		if(dst_type == "RANDOM")
			route_info = get_random_dest();		// get random destination
		trafstream<<"NUM_FLITS: "<<num_flits<<endl;	// write number of flits
		trafstream<<"DESTINATION: "<<route_info<<endl;	// write destination ID or route code
	}
     * */
	trafstream.close();	// close traffic log file
}

////////////////////////////////////////////////
/// Method to return inter-packet interval
////////////////////////////////////////////////
double CBRTraffic::next_interval()
{
	return pkt_interval;	// return uniform interval
}

// for dynamic linking
extern "C" {
	ipcore *maker() {
		return new CBRTraffic("CBR");
	}
}
