
/*
 * TG.cpp
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
/// \file TG.cpp
/// \brief Implements abstract traffic generator
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#include "TG.h"
#include "../../config/extern.h"

////////////////////////////////////////////////
/// Constructor
////////////////////////////////////////////////
TrafficGenerator::TrafficGenerator(sc_module_name TrafficGenerator): ipcore(TrafficGenerator) {
	flit_interval = 1;	// inter flit interval
	cycles_per_flit = 2;	// clock cycles required for processing of a flit
    next_pkt_time = 0;
    num_flits = 0;
    route_info = 0;
}

////////////////////////////////////////////////
/// Thread sensitive to clock
/// - inherited from ipcore
/// - send flits as per traffic configuration
////////////////////////////////////////////////
void TrafficGenerator::send_app() {
	num_pkts_gen = 0;	// initialize number of packets generated to zero
	wait(WARMUP);		// wait for WARMUP period
	
	string field;
	
	// open traffic log file
	char str_id[3];
	sprintf(str_id, "%d", tileID);
	string traffic_filename = string("log/traffic/tile-") + string(str_id);
	ifstream trafstream;
	trafstream.open(traffic_filename.c_str());

	// read inter-flit interval
	trafstream >> field >> flit_interval;
    
    int num_flits_gen_int = 0;
	
    // generate traffic until TG_NUM
    while(sim_count <= TG_NUM && !trafstream.eof()) {

        // read inter-pkt interval
        trafstream >> field >> next_pkt_time;
        // read no. of flits in packet
        trafstream >> field >> num_flits;
        // read destination or route code
        trafstream >> field >> route_info;
        
        flit flit_out;
        
        if (!accept_destinations[route_info]) {
            if(LOG >= 3)
                eventlog<<"\ntime: "<<sc_time_stamp()<<" name: "<<this->name()<<" tile: "<<tileID<<" Not accepted destination "<<route_info;
            continue;
        }            
        
        // create hdt/head flit
        if(num_flits == 1)
            flit_out = *create_hdt_flit(num_pkts_gen, 0, route_info);
        else
            flit_out = *create_head_flit(num_pkts_gen, 0, route_info);
            
        // wait while buffer has space
        while(!credit_in[0].read().freeBuf) {
            //cout<<"Time: "<<sc_time_stamp()<<" ipcore: "<<tileID<<" No space in core buffer"<<endl;
            wait();
            next_pkt_time--;
        }
        
        // write flit to output port
        flit_outport.write(flit_out);
        if(LOG >= 1)
            eventlog<<"\ntime: "<<sc_time_stamp()<<" name: "<<this->name()<<" tile: "<<tileID<<" Sending flit from core "<<flit_out;
        
        num_flits_gen_int = 1;
        num_flits_gen++;
        // generate subsequent flits in packet
        while(num_flits_gen_int < num_flits) {
            wait(flit_interval);
            next_pkt_time -= flit_interval;
            
            // create flit
            if(num_flits_gen_int == num_flits-1)
                flit_out = *create_tail_flit(num_pkts_gen, num_flits_gen_int);
            else
                flit_out = *create_data_flit(num_pkts_gen, num_flits_gen_int);
            
            num_flits_gen++;
            num_flits_gen_int++;

            // wait until space in buffer
            while(!credit_in[0].read().freeBuf) {
                //cout<<"Time: "<<sc_time_stamp()<<" ipcore: "<<tileID<<" No space in core buffer"<<endl;
                wait();
                next_pkt_time--;
            }
            
            // send flit
            flit_outport.write(flit_out);
            if(LOG >= 1)
                eventlog<<"\ntime: "<<sc_time_stamp()<<" name: "<<this->name()<<" tile: "<<tileID<<" Sending flit from core "<<flit_out;
        }

        num_pkts_gen++;
        if(next_pkt_time > 0)
            wait(next_pkt_time);
        else wait(1);
    }
    #ifdef DEBUG_NOC
        cout<<sc_time_stamp()<<" name: "<<this->name()<<" tile: "<<tileID<<" packets: "<<num_pkts_gen<<" flits: "<<num_flits_gen<<endl;
    #endif
    
    trafstream.close();	// close traffic config file
}

////////////////////////////////////////////////
/// Thread sensitive to clock and inport event
/// - inherited from ipcore
/// - recieve incoming flits
/// - assign arrival timestamps
////////////////////////////////////////////////
void TrafficGenerator::recv_app() {
		wait();
}

////////////////////////////////////////////////
/// Method to convert time unit from string representation to systemC representation
////////////////////////////////////////////////
sc_time_unit TrafficGenerator::strToTime(string unit) {
	if(unit == "s")
		return SC_SEC;
	if(unit == "ms")
		return SC_MS;
	if(unit == "us")
		return SC_US;
	if(unit == "ns")
		return SC_NS;
	if(unit == "ps")
		return SC_PS;
	if(unit == "fs")
		return SC_FS;
}


