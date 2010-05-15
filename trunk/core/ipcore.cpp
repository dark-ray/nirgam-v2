
/*
 * ipcore.cpp
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
/// \file ipcore.cpp
/// \brief Implements module ipcore (base class for applications)
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#include "ipcore.h"
#include "../config/extern.h"


////////////////////////
/// constructor
////////////////////////
ipcore::ipcore(sc_module_name ipcore): sc_module(ipcore) {

	sim_count = 0;
    num_pkts_gen = 0;
    num_flits_gen = 0;
    total_latency = 0;
    total_packets_recived = 0;
    total_flits_recived = 0;
    time_first_flit_in = 0;
    time_last_flit_in = 0;
	wc_latency = 0.0;
	wc_num_waits = 0;
	num_waits = 0;
	wc_num_sw = 0;
	num_sw = 0;
	avg_num_waits = 0.0;
	avg_num_sw = 0.0;
    
	ran_var = new RNG((RNG::RNGSources)2,1);
    
    for (UI i = 0; i < MAX_NUM_TILES; i++)
        accept_destinations[i] = true;

	// process sensitive to clock, sends out flit
	SC_CTHREAD(send, clock.pos());
	
	// process sensitive to clock, to keep track of simulation count
	SC_CTHREAD(entry, clock.pos());
	
	// process sensitive to clock and inport event, recieves incoming flit
	SC_THREAD(recv);
	sensitive << flit_inport << clock;

}

///////////////////////////////////////////////////////////////////////////
/// Method to assign tile ID
/// \param id tile ID
///////////////////////////////////////////////////////////////////////////
void ipcore::setID(UI id) {
	tileID = id;
}

///////////////////////////////////////////////////////////////////////////
/// Method to send flit 
/// - call send_app
///////////////////////////////////////////////////////////////////////////
void ipcore::send(){
	send_app();
}

///////////////////////////////////////////////////////////////////////////
/// Method to receive flit
/// - call recv_app
/// - process statistics
///////////////////////////////////////////////////////////////////////////
void ipcore::recv(){
	double temp = 0.0;
	while(true) {
		recv_app();
		if(flit_inport.event()) {         // got flit - make statistics
			flit flit_recd = flit_inport.read();
			flit_recd.simdata.atimestamp = sim_count;
			flit_recd.simdata.atime = sc_time_stamp();
            
			if (flit_recd.simdata.num_waits > wc_num_waits)
				wc_num_waits = flit_recd.simdata.num_waits;
			num_waits += flit_recd.simdata.num_waits;
			if (flit_recd.simdata.num_sw > wc_num_sw)
				wc_num_sw = flit_recd.simdata.num_sw;
            num_sw += flit_recd.simdata.num_sw;
			
			total_flits_recived++;
			total_latency += flit_recd.simdata.atimestamp - 1 - flit_recd.simdata.gtimestamp;
			temp = (double)(flit_recd.simdata.atimestamp - 1 - flit_recd.simdata.gtimestamp);
			if (temp > wc_latency)
				wc_latency = temp;
			if ((flit_recd.pkthdr.nochdr.flittype == TAIL) || (flit_recd.pkthdr.nochdr.flittype == HDT))
				total_packets_recived++;
			if (time_first_flit_in == 0)
				time_first_flit_in = sim_count;
			time_last_flit_in = sim_count;
			if(LOG >= 1)
				eventlog<<"\ntime: "<<sc_time_stamp()<<" name: "<<this->name()<<" tile: "<<tileID<<" Recieved flit at core "<<flit_recd<<flit_recd.simdata;
        }
	}
}

///////////////////////////////////////////////////////////////////////////
/// Process sensitive to clock
/// Keeps track of clock cycles in the module
///////////////////////////////////////////////////////////////////////////
void ipcore::entry() {
	sim_count = 0;
	while(true) {
		wait();
		sim_count++;
	}
}

///////////////////////////////////////////////////////////////////////////
/// Method to return a random destination
/// \return random destination
///////////////////////////////////////////////////////////////////////////
UI ipcore::get_random_dest() {
	UI dest = ran_var->uniform((int)num_tiles);
	while(dest == tileID)
		dest = ran_var->uniform((int)num_tiles);
	return dest;
}

//////////////////////////////////////////////////////////////////////////
/// Method to change ip core state of generating flit
/// \param toTileID tile ID to which state changing
/// \param grant accept generation (true) or not (false)
/// \return state changed flag
/////////////////////////////////////////////////////////////////////////
bool ipcore::set_creating_flits_state(UI toTileID, bool grant) {
    if (toTileID > num_tiles)
        return false;
    
    if (toTileID == num_tiles) {
        for (UI i = 0; i < num_tiles; i++)
            accept_destinations[i] = grant;
    }
    else
        accept_destinations[toTileID] = grant;
    return true;       
}

///////////////////////////////////////////////////////////////////////////
/// Method to create a hdt flit
/// hdt flit represents a packet consisting of single flit.
/// \param pkt_id packet id
/// \param flit_id flit id
/// \param route_info route direction
/// \return pointer to new flit
///////////////////////////////////////////////////////////////////////////
flit* ipcore::create_hdt_flit(int pkt_id, int flit_id, UI route_info) {
	flit *flit_out = new flit;
	flit_out->pkttype = NOC;
	flit_out->vcid = 0;
	flit_out->src = tileID;
	
	flit_out->pkthdr.nochdr.flittype = HDT;
	flit_out->pkthdr.nochdr.pktid = pkt_id;
	flit_out->pkthdr.nochdr.flitid = flit_id;
	flit_out->pkthdr.nochdr.hopcount = 0;
	flit_out->pkthdr.nochdr.flithdr.header.rtalgo = RT_ALGO;
    flit_out->pkthdr.nochdr.flithdr.header.rtfi.fail = false;
    flit_out->pkthdr.nochdr.flithdr.header.rtfi.last_back_adap = false;
    flit_out->pkthdr.nochdr.flithdr.header.rtfi.last_back = false;
    flit_out->pkthdr.nochdr.flithdr.header.rtfi.last_dir = ND;
    flit_out->pkthdr.nochdr.flithdr.header.rtfi.history = 0;
	if(RT_ALGO == SOURCE)
		flit_out->pkthdr.nochdr.flithdr.header.rthdr.sourcehdr.route = route_info;
	else
		flit_out->pkthdr.nochdr.flithdr.header.rthdr.dsthdr.dst = route_info;
	
	flit_out->simdata.gtime = sc_time_stamp();
	flit_out->simdata.ctime = sc_time_stamp();
	flit_out->simdata.atime = SC_ZERO_TIME;
	flit_out->simdata.atimestamp = 0;
	flit_out->simdata.num_waits = 0;
	flit_out->simdata.num_sw = 0;
	flit_out->simdata.gtimestamp = sim_count;
	
	return flit_out;
}

///////////////////////////////////////////////////////////////////////////
/// Method to create a head flit
/// \param pkt_id packet id
/// \param flit_id flit id
/// \param route_info route direction
/// \return pointer to new flit
///////////////////////////////////////////////////////////////////////////
flit* ipcore::create_head_flit(int pkt_id, int flit_id, UI route_info) {
	flit *flit_out = new flit;
	flit_out->pkttype = NOC;
	flit_out->vcid = 0;
	flit_out->src = tileID;
	
	flit_out->pkthdr.nochdr.flittype = HEAD;
	flit_out->pkthdr.nochdr.pktid = pkt_id;
	flit_out->pkthdr.nochdr.flitid = flit_id;
	flit_out->pkthdr.nochdr.hopcount = 0;
	flit_out->pkthdr.nochdr.flithdr.header.rtalgo = RT_ALGO;
    flit_out->pkthdr.nochdr.flithdr.header.rtfi.fail = false;
    flit_out->pkthdr.nochdr.flithdr.header.rtfi.last_back = false;
    flit_out->pkthdr.nochdr.flithdr.header.rtfi.last_back_adap = false;
    flit_out->pkthdr.nochdr.flithdr.header.rtfi.last_dir = ND;
    flit_out->pkthdr.nochdr.flithdr.header.rtfi.history = 0;
	if(RT_ALGO == SOURCE)
		flit_out->pkthdr.nochdr.flithdr.header.rthdr.sourcehdr.route = route_info;
	else
		flit_out->pkthdr.nochdr.flithdr.header.rthdr.dsthdr.dst = route_info;
	
	
	flit_out->simdata.gtime = sc_time_stamp();
	flit_out->simdata.ctime = sc_time_stamp();
	flit_out->simdata.atime = SC_ZERO_TIME;
	flit_out->simdata.atimestamp = 0;
	flit_out->simdata.num_waits = 0;
	flit_out->simdata.num_sw = 0;
	flit_out->simdata.gtimestamp = sim_count;
	
	return flit_out;
}

///////////////////////////////////////////////////////////////////////////
/// Method to create a data flit
/// \param pkt_id packet id
/// \param flit_id flit id
/// \return pointer to new flit
///////////////////////////////////////////////////////////////////////////
flit* ipcore::create_data_flit(int pkt_id, int flit_id) {
	flit *flit_out = new flit;
	flit_out->pkttype = NOC;
	flit_out->vcid = 0;
	flit_out->src = tileID;
	
	flit_out->pkthdr.nochdr.flittype = DATA;
	flit_out->pkthdr.nochdr.pktid = pkt_id;
	flit_out->pkthdr.nochdr.flitid = flit_id;
	flit_out->pkthdr.nochdr.hopcount = 0;
	
	flit_out->simdata.gtime = sc_time_stamp();
	flit_out->simdata.ctime = sc_time_stamp();
	flit_out->simdata.atime = SC_ZERO_TIME;
	flit_out->simdata.atimestamp = 0;
	flit_out->simdata.num_waits = 0;
	flit_out->simdata.num_sw = 0;
	flit_out->simdata.gtimestamp = sim_count;
	
	return flit_out;
}

///////////////////////////////////////////////////////////////////////////
/// Method to create a tail flit
/// \param pkt_id packet id
/// \param flit_id flit id
/// \return pointer to new flit
///////////////////////////////////////////////////////////////////////////
flit* ipcore::create_tail_flit(int pkt_id, int flit_id) {
	flit *flit_out = new flit;
	flit_out->pkttype = NOC;
	flit_out->vcid = 0;
	flit_out->src = tileID;
	
	flit_out->pkthdr.nochdr.flittype = TAIL;
	flit_out->pkthdr.nochdr.pktid = pkt_id;
	flit_out->pkthdr.nochdr.flitid = flit_id;
	flit_out->pkthdr.nochdr.hopcount = 0;
	
	flit_out->simdata.gtime = sc_time_stamp();
	flit_out->simdata.ctime = sc_time_stamp();
	flit_out->simdata.atime = SC_ZERO_TIME;
	flit_out->simdata.atimestamp = 0;
	flit_out->simdata.num_waits = 0;
	flit_out->simdata.num_sw = 0;
	flit_out->simdata.gtimestamp = sim_count;
	
	return flit_out;
}

///////////////////////////////////////////////////////////////////////////
/// Method to assign value to command field of a flit
/// \param inflit pointer to flit
/// \param cmd_value integer representing command
///////////////////////////////////////////////////////////////////////////
void ipcore::set_cmd(flit* inflit, int cmd_value) {
	if(inflit->pkthdr.nochdr.flittype == HEAD || inflit->pkthdr.nochdr.flittype == HDT) {
		inflit->pkthdr.nochdr.flithdr.header.datahdr.cmd = cmd_value;
	}
	else {
		inflit->pkthdr.nochdr.flithdr.payload.cmd = cmd_value;
	}
}

///////////////////////////////////////////////////////////////////////////
/// Method to assign value to integer data field of a flit
/// \param inflit pointer to flit
/// \param data_int_value integer data
///////////////////////////////////////////////////////////////////////////
void ipcore::set_data(flit* inflit, int data_int_value) {
	if(inflit->pkthdr.nochdr.flittype == HEAD || inflit->pkthdr.nochdr.flittype == HDT) {
		inflit->pkthdr.nochdr.flithdr.header.datahdr.data_int = data_int_value;
	}
	else {
		inflit->pkthdr.nochdr.flithdr.payload.data_int = data_int_value;
	}
}

///////////////////////////////////////////////////////////////////////////
/// Method to assign value to string data field of a flit
/// \param inflit pointer to flit
/// \param data_string_value string data
///////////////////////////////////////////////////////////////////////////
void ipcore::set_data(flit* inflit, string data_string_value) {
	if(inflit->pkthdr.nochdr.flittype == HEAD || inflit->pkthdr.nochdr.flittype == HDT) {
		inflit->pkthdr.nochdr.flithdr.header.datahdr.data_string = data_string_value.c_str();
	}
	else {
		inflit->pkthdr.nochdr.flithdr.payload.data_string = data_string_value.c_str();
	}
}

///////////////////////////////////////////////////////////////////////////
/// Method to assign value to both command and integer data field of a flit
/// \param inflit pointer to flit
/// \param cmd_value integer representing command
/// \param data_int_value integer data
///////////////////////////////////////////////////////////////////////////
void ipcore::set_payload(flit* inflit, int cmd_value, int data_int_value) {
	if(inflit->pkthdr.nochdr.flittype == HEAD || inflit->pkthdr.nochdr.flittype == HDT) {
		inflit->pkthdr.nochdr.flithdr.header.datahdr.cmd = cmd_value;
		inflit->pkthdr.nochdr.flithdr.header.datahdr.data_int = data_int_value;
	}
	else {
		inflit->pkthdr.nochdr.flithdr.payload.cmd = cmd_value;
		inflit->pkthdr.nochdr.flithdr.payload.data_int = data_int_value;
	}
}

///////////////////////////////////////////////////////////////////////////
/// Method to assign value to both command and string data field of a flit
/// \param inflit pointer to flit
/// \param cmd_value integer representing command
/// \param data_string_value string data
///////////////////////////////////////////////////////////////////////////
void ipcore::set_payload(flit* inflit, int cmd_value, string data_string_value) {
	if(inflit->pkthdr.nochdr.flittype == HEAD || inflit->pkthdr.nochdr.flittype == HDT) {
		inflit->pkthdr.nochdr.flithdr.header.datahdr.cmd = cmd_value;
		inflit->pkthdr.nochdr.flithdr.header.datahdr.data_string = data_string_value.c_str();
	}
	else {
		inflit->pkthdr.nochdr.flithdr.payload.cmd = cmd_value;
		inflit->pkthdr.nochdr.flithdr.payload.data_string = data_string_value.c_str();
	}
}

///////////////////////////////////////////////////////////////////////////
/// Method to return command field of a flit
/// \param inflit flit
/// \param cmd_value integer variable in which command value is returned
///////////////////////////////////////////////////////////////////////////
void ipcore::get_cmd(flit inflit, int &cmd_value) {
	if(inflit.pkthdr.nochdr.flittype == HEAD || inflit.pkthdr.nochdr.flittype == HDT) {
		cmd_value = inflit.pkthdr.nochdr.flithdr.header.datahdr.cmd;
	}
	else {
		cmd_value = inflit.pkthdr.nochdr.flithdr.payload.cmd;
	}
}

///////////////////////////////////////////////////////////////////////////
/// Method to return integer data field of a flit
/// \param inflit flit
/// \param data_int_value integer variable in which integer data value is returned
///////////////////////////////////////////////////////////////////////////
void ipcore::get_data(flit inflit, int &data_int_value) {
	if(inflit.pkthdr.nochdr.flittype == HEAD || inflit.pkthdr.nochdr.flittype == HDT) {
		data_int_value = inflit.pkthdr.nochdr.flithdr.header.datahdr.data_int;
	}
	else {
		data_int_value = inflit.pkthdr.nochdr.flithdr.payload.data_int;
	}
}

///////////////////////////////////////////////////////////////////////////
/// Method to return string data field of a flit
/// \param inflit flit
/// \param data_string_value string variable in which string data value is returned
///////////////////////////////////////////////////////////////////////////
void ipcore::get_data(flit inflit, string &data_string_value) {
	if(inflit.pkthdr.nochdr.flittype == HEAD || inflit.pkthdr.nochdr.flittype == HDT) {
		data_string_value = inflit.pkthdr.nochdr.flithdr.header.datahdr.data_string;
	}
	else {
		data_string_value = inflit.pkthdr.nochdr.flithdr.payload.data_string;
	}
}

///////////////////////////////////////////////////////////////////////////
/// Method to return both command field and integer data field of a flit
/// \param inflit flit
/// \param cmd_value integer variable in which command value is returned
/// \param data_int_value integer variable in which integer data value is returned
///////////////////////////////////////////////////////////////////////////
void ipcore::get_payload(flit inflit, int &cmd_value, int &data_int_value) {
	if(inflit.pkthdr.nochdr.flittype == HEAD || inflit.pkthdr.nochdr.flittype == HDT) {
		cmd_value = inflit.pkthdr.nochdr.flithdr.header.datahdr.cmd;
		data_int_value = inflit.pkthdr.nochdr.flithdr.header.datahdr.data_int;
	}
	else {
		cmd_value = inflit.pkthdr.nochdr.flithdr.payload.cmd;
		data_int_value = inflit.pkthdr.nochdr.flithdr.payload.data_int;
	}
}

///////////////////////////////////////////////////////////////////////////
/// Method to return both command field and string data field of a flit
/// \param inflit flit
/// \param cmd_value integer variable in which command value is returned
/// \param data_string_value string variable in which string data value is returned
///////////////////////////////////////////////////////////////////////////
void ipcore::get_payload(flit inflit, int &cmd_value, string &data_string_value) {
	if(inflit.pkthdr.nochdr.flittype == HEAD || inflit.pkthdr.nochdr.flittype == HDT) {
		cmd_value = inflit.pkthdr.nochdr.flithdr.header.datahdr.cmd;
		data_string_value = inflit.pkthdr.nochdr.flithdr.header.datahdr.data_string;
	}
	else {
		cmd_value = inflit.pkthdr.nochdr.flithdr.payload.cmd;
		data_string_value = inflit.pkthdr.nochdr.flithdr.payload.data_string;
	}
}

///////////////////////////////////////////////////////////////////////////
/// Method to finalize statistics and close log
///////////////////////////////////////////////////////////////////////////
void ipcore::closeLogs() {
    if(total_packets_recived != 0)
		avg_latency = (double)total_latency/total_packets_recived;
	if(total_flits_recived != 0) {
		avg_latency_flit = (double)total_latency/total_flits_recived;
		avg_num_waits    = (double)num_waits/total_flits_recived;
		avg_num_sw    = (double)num_sw/total_flits_recived;
	}
	total_cycles = time_last_flit_in - time_first_flit_in + 1;
	if(total_cycles != 0)
		avg_throughput = (double)(total_flits_recived * FLITSIZE * 8) / (total_cycles * CLK_PERIOD);	// Gbps
	
    results_log<<tileID<<"\t"<<"Core\t\t"<<total_packets_recived<<"\t\t"<<total_flits_recived<<"\t\t"<<avg_latency;
    results_log<<"\t\t"<<avg_latency_flit<<"\t\t"<<avg_throughput<<endl;
}
