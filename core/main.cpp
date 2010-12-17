
/*
 * main.cpp
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
 /*! \mainpage NIRGAM Quick Code Index
 *
 * NIRGAM is a systemC based discrete event, cycle accurate simulator for research in Network on Chip(NoC). 
 * It provides substantial support to experiment with NoC design in terms of routing algorithms and applications on various topologies.
 * \image html Nirgam_arch.jpg
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
 * Portions of changes by Alexander Rumyanthev (darkstreamray@gmail.com) SPbSU ITMO 2010.
 */

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/// \file main.cpp
/// \brief Begins and ends simulation
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#include <time.h>
#include "NoC.h"
#include "../config/default.h"

ofstream eventlog;
ofstream results_log;
//sc_trace_file *tracefile;	// use to generate vcd trace
sc_clock *nw_clock;
string app_libname[MAX_NUM_TILES];
string DIRS_NAMES[6];

int sc_main(int argc, char *argv[]) {

	cout<<"-------------------------------------------------------------------------------"<<endl;
	cout<<"  NIRGAM: Simulator for NoC Interconnect RoutinG and Application Modeling\n";
    cout<<"  Author: Lavina Jain\n";
    cout<<"  Portion of changes by Alexander Rumyanthev (darkstreamray@gmail.com)\n";
    cout<<"  NIRGAM v 2.0 Build "<< __DATE__<<" "<<__TIME__<<"\n";
	cout<<"-------------------------------------------------------------------------------"<<endl;
	string DIRNAME = string("sim1");

	// open event log file	
	string event_filename = string("log/nirgam/event.log");
	eventlog.open(event_filename.c_str());
	if(!eventlog.is_open()){
		cout<<"cannot open event.log"<<endl;
	}
    
    // DIRS_NAME fill
    DIRS_NAMES[0] = string("N");
    DIRS_NAMES[1] = string("S");
    DIRS_NAMES[2] = string("E");
    DIRS_NAMES[3] = string("W");
    DIRS_NAMES[4] = string("C");
    DIRS_NAMES[5] = string("ND");
	
	// read simulator configuration (nirgam.config)
	string nirgam_filename = string("config/nirgam.config");
	ifstream fil1;
	fil1.open(nirgam_filename.c_str());
	
	if(!fil1.is_open()){
		cout<<"nirgam.config does not exist, using default parameter values."<<endl;
	}
	else{
		cout <<"\nLoading parameters from nirgam.config file...";
		while(!fil1.eof()){
			string name;
			fil1 >> name ;
			if(name=="LOG"){
				UI value; fil1 >> value; LOG = value;
			}
            if(name=="ADDITIONAL_INFO"){
				UI value; fil1 >> value; ADDITIONAL_INFO = ((value == 0) ? false : true);
			}
            if(name=="MATLAB_MAKE_IMAGES"){
				UI value; fil1 >> value; MATLAB_MAKE_IMAGES = ((value == 0) ? false : true);
			}
			else if(name=="SIM_NUM"){
				ULL value; fil1 >> value; SIM_NUM = value;
			}
			else if(name=="WARMUP"){
				ULL value; fil1 >> value; WARMUP = value;
			}
			else if(name=="TG_NUM"){
				ULL value; fil1 >> value; TG_NUM = value;
			}
			else if(name=="RT_ALGO"){
				fil1 >> name;
				if(name == "XY")
					RT_ALGO = XY;
				else if(name == "OE")
					RT_ALGO = OE;
				else if(name == "SOURCE")
					RT_ALGO = SOURCE;
                else if(name == "DyXY")
					RT_ALGO = DyXY;
                else if(name == "DyAD_OE")
					RT_ALGO = DyAD_OE;
                else if(name == "West_First")
					RT_ALGO = West_First;
                else if(name == "North_Last")
					RT_ALGO = North_Last;
                else if(name == "Negative_First")
					RT_ALGO = Negative_First;
                else if(name == "DyBM")
					RT_ALGO = DyBM;
                else if(name == "DyXY_FT")
					RT_ALGO = DyXY_FT;
                else
                    RT_ALGO = XY;
			}
            else if (name=="TR_TYPE") {
                fil1 >> name;
                if (name == "RANDOM")
                    TR_TYPE = RANDOM;
                else if (name == "CONGESTION")
                    TR_TYPE = CONGESTION;
                else
                    TR_TYPE = RANDOM;
            }
            else if (name=="IAT_TYPE") {
                fil1 >> name;
                if (name == "SEQUENCE")
                    IAT_TYPE = SEQUENCE;
                else if (name == "RR")
                    IAT_TYPE = RR;
                else if (name == "AA")
                    IAT_TYPE = AA; 
                else
                    IAT_TYPE = RR;
            }
            else if(name=="HOP_LEVEL"){
				UI value; fil1 >> value; HOP_LEVEL = value;
			}
            else if(name=="HOP_USE"){
				UI value; fil1 >> value; HOP_USE = ((value == 0) ? false : true);
			}
            else if(name=="CONGESTION_LEVEL"){
				UI value; fil1 >> value; CONGESTION_LEVEL = value;
			}
            else if(name=="CONGESTION_PRIORITY"){
				UI value; fil1 >> value; CONGESTION_PRIORITY = value;
			}
            else if(name=="CONGESTION_AFFECT_VC"){
				UI value; fil1 >> value; CONGESTION_AFFECT_VC = ((value == 0) ? false : true);
			}
            else if(name=="CONGESTION_USE"){
				UI value; fil1 >> value; CONGESTION_USE = ((value == 0) ? false : true);
			}
	    else if(name=="WRITE_THROUGH_OUTPORT"){
				UI value; fil1 >> value; WRITE_THROUGH_OUTPORT = ((value == 0) ? false : true);
			}
			else if(name=="DIRNAME"){
				string value; fil1 >> value; DIRNAME = value;
			}
			else if(name=="TOPOLOGY"){
				fil1 >> name;
				if(name == "MESH")
					TOPO = MESH;
				else if(name == "TORUS")
					TOPO = TORUS;
			}
			else if(name=="NUM_ROWS"){
				UI value; fil1 >> value; num_rows = value;
			}
			else if(name=="NUM_COLS"){
				UI value; fil1 >> value; num_cols = value;
			}
			else if(name=="NUM_BUFS"){
				UI value; fil1 >> value; NUM_BUFS = value; HALF_NUM_BUFS = ((NUM_BUFS * NUM_VCS) / 2) + ((NUM_BUFS * NUM_VCS) % 2);
			}
			else if(name=="FLITSIZE"){
				UI value; fil1 >> value; FLITSIZE = value;
			}
			else if(name=="HEAD_PAYLOAD"){
				UI value; fil1 >> value; HEAD_PAYLOAD = value;
			}
			else if(name=="DATA_PAYLOAD"){
				UI value; fil1 >> value; DATA_PAYLOAD = value;
			}
			else if(name=="CLK_FREQ"){
				double value; fil1 >> value; CLK_FREQ = value;
				CLK_PERIOD = (1/CLK_FREQ);
			}
		}
		cout<<"done"<<endl;
	}
	fil1.close();

	num_tiles = num_rows * num_cols;	// compute number of tiles
	nw_clock = new sc_clock("NW_CLOCK",CLK_PERIOD,SC_NS);	// create global clock
	
	// open log and result files
	DIRNAME = string("results/") + DIRNAME;
	string cmd = string("rm -rf ") + DIRNAME;
	system(cmd.c_str());
	
	cmd = string("mkdir ") + DIRNAME;
	system(cmd.c_str());
	string temp = cmd + string("/stats");
	system(temp.c_str());
	temp = cmd + string("/graphs");
	system(temp.c_str());
    if (MATLAB_MAKE_IMAGES) {
        temp = cmd + string("/matlab_images");
        system(temp.c_str());
    }
		
	string ylatency_filename = string("log/gnuplot/ylatency");
	ofstream ylatency_log;
	ylatency_log.open(ylatency_filename.c_str());
	if(!ylatency_log.is_open())
		cout<<"Cannot open ylatency"<<endl;
	
	string xlatency_filename = string("log/gnuplot/xlatency");
	ofstream xlatency_log;
	xlatency_log.open(xlatency_filename.c_str());
	if(!xlatency_log.is_open())
		cout<<"Cannot open xlatency"<<endl;
	
	string ylatency_flit_filename = string("log/gnuplot/ylatency_flit");
	ofstream ylatency_flit_log;
	ylatency_flit_log.open(ylatency_flit_filename.c_str());
	if(!ylatency_flit_log.is_open())
		cout<<"Cannot open ylatency_flit"<<endl;
	
	string xlatency_flit_filename = string("log/gnuplot/xlatency_flit");
	ofstream xlatency_flit_log;
	xlatency_flit_log.open(xlatency_flit_filename.c_str());
	if(!xlatency_flit_log.is_open())
		cout<<"Cannot open xlatency"<<endl;
	
	string ytput_filename = string("log/gnuplot/ytput");
	ofstream ytput_log;
	ytput_log.open(ytput_filename.c_str());
	if(!ytput_log.is_open())
		cout<<"Cannot open ytput"<<endl;
	
	string xtput_filename = string("log/gnuplot/xtput");
	ofstream xtput_log;
	xtput_log.open(xtput_filename.c_str());
	if(!xtput_log.is_open())
		cout<<"Cannot open xtput"<<endl;
	
	string mat_lat_pkt_filename = string("log/matlab/latency_pkt");
	ofstream mat_lat_pkt_log;
	mat_lat_pkt_log.open(mat_lat_pkt_filename.c_str());
	if(!mat_lat_pkt_log.is_open())
		cout<<"Cannot open "<<mat_lat_pkt_filename<<endl;
	
	string mat_lat_flit_filename = string("log/matlab/latency_flit");
	ofstream mat_lat_flit_log;
	mat_lat_flit_log.open(mat_lat_flit_filename.c_str());
	if(!mat_lat_flit_log.is_open())
		cout<<"Cannot open "<<mat_lat_flit_filename<<endl;
	
	string mat_tput_filename = string("log/matlab/throughput");
	ofstream mat_tput_log;
	mat_tput_log.open(mat_tput_filename.c_str());
	if(!mat_tput_log.is_open())
		cout<<"Cannot open "<<mat_tput_filename<<endl;
        
    string mat_lat_core_pkt_filename = string("log/matlab/core_latency_pkt");
	ofstream mat_lat_core_pkt_log;
	mat_lat_core_pkt_log.open(mat_lat_core_pkt_filename.c_str());
	if(!mat_lat_core_pkt_log.is_open())
		cout<<"Cannot open "<<mat_lat_core_pkt_filename<<endl;
	
	string mat_lat_core_flit_filename = string("log/matlab/core_latency_flit");
	ofstream mat_lat_core_flit_log;
	mat_lat_core_flit_log.open(mat_lat_core_flit_filename.c_str());
	if(!mat_lat_core_flit_log.is_open())
		cout<<"Cannot open "<<mat_lat_core_flit_filename<<endl;
	
	string mat_core_tput_filename = string("log/matlab/core_throughput");
	ofstream mat_core_tput_log;
	mat_core_tput_log.open(mat_core_tput_filename.c_str());
	if(!mat_core_tput_log.is_open())
		cout<<"Cannot open "<<mat_core_tput_filename<<endl;
	
	string results_file= DIRNAME + string("/stats/sim_results");
	results_log.open(results_file.c_str());
	if(!results_log.is_open())
		cout<<"Cannot open "<<results_file.c_str()<<endl;
	results_log<<"Tile\t"<<"Output\t\t"<<"Total no.\t"<<"Total no.\t"<<"avg. latency\t"<<"avg. latency\t"<<"average\n";
	results_log<<"ID\t"<<"channel\t\t"<<"of packets\t"<<"of flits\t"<<"per packet\t"<<"per flit\t"<<"throughput\n";
	results_log<<"\t\t\t\t\t\t\t(clock cycles)\t(clock cycles)\t(Gbps)\n";

	/* example to show how to generate vcd trace */
	//string trace_filename = string("log/vcdtrace/trace");
	//tracefile = sc_create_vcd_trace_file(trace_filename.c_str());

	// read application configuration (application.config)
	for(UI i = 0; i < MAX_NUM_TILES; i++) {
		app_libname[i] = string("NULL");
	}
	string app_filename = string("config/application.config");
	ifstream app_fil;
	app_fil.open(app_filename.c_str());
	if(!app_fil.is_open()){
		cout<<"Error: File application.config could not be opened." << endl;
		exit(-1);
	}
	UI id;
	string libname;
	while(!app_fil.eof()) {
		app_fil >> id >> libname;
		app_libname[id] = libname;
	}
	app_fil.close();
	
	// create NoC model
	NoC noc("noc", num_rows, num_cols);
	// connect clock
	noc.switch_cntrl(*nw_clock);
		
	cout << "Network setup!" <<endl;
	cout<<"Start NIRGAM simulation!" << endl;
	cout<<"---------------------------------------------------------------------------"<<endl;

    ULL time_elapsed = clock();
	//sc_trace(tracefile, noc.switch_cntrl, "clk");
	sc_start();	// begin simulation

	// generate matlab log
	double null = 0.0;
	for(UI i = 0; i < num_cols; i++) {
		for(UI j = 0; j < num_rows; j++) {
			mat_lat_pkt_log<<"\t"<<null;
			mat_lat_flit_log<<"\t"<<null;
			mat_tput_log<<"\t"<<null;
			
			if(j != num_rows - 1) {
        if (noc.nwtile[j][i] != NULL) {
            mat_lat_pkt_log<<"\t"<<(noc.nwtile[j][i])->return_latency(S);
            mat_lat_flit_log<<"\t"<<(noc.nwtile[j][i])->return_latency_flit(S);
            mat_tput_log<<"\t"<<(noc.nwtile[j][i])->return_avg_tput(S);
            
            mat_lat_pkt_log<<"\t"<<(noc.nwtile[j+1][i])->return_latency(N);
            mat_lat_flit_log<<"\t"<<(noc.nwtile[j+1][i])->return_latency_flit(N);
            mat_tput_log<<"\t"<<(noc.nwtile[j+1][i])->return_avg_tput(N);
        }
        else {
            mat_lat_pkt_log<<"\t0.0";
            mat_lat_flit_log<<"\t0.0";
            mat_tput_log<<"\t0.0";
            
            mat_lat_pkt_log<<"\t0.0";
            mat_lat_flit_log<<"\t0.0";
            mat_tput_log<<"\t0.0";
        }
			}
		}
		mat_lat_pkt_log<<endl;
		mat_lat_flit_log<<endl;
		mat_tput_log<<endl;
			
		
		if(i != num_cols - 1) {
			for(UI j = 0; j < num_rows; j++) {
        if (noc.nwtile[j][i] != NULL) {
            mat_lat_pkt_log<<"\t"<<(noc.nwtile[j][i])->return_latency(E);
            mat_lat_flit_log<<"\t"<<(noc.nwtile[j][i])->return_latency_flit(E);
            mat_tput_log<<"\t"<<(noc.nwtile[j][i])->return_avg_tput(E);
        }
        else {
            mat_lat_pkt_log<<"\t0.0";
            mat_lat_flit_log<<"\t0.0";
            mat_tput_log<<"\t0.0";
        }
        if(j != num_rows - 1) {
            mat_lat_pkt_log<<"\t"<<null<<"\t"<<null;
            mat_lat_flit_log<<"\t"<<null<<"\t"<<null;
            mat_tput_log<<"\t"<<null<<"\t"<<null;
        }
			}
			mat_lat_pkt_log<<endl;
			mat_lat_flit_log<<endl;
			mat_tput_log<<endl;
					
			
			for(UI j = 0; j < num_rows; j++) {
        if (noc.nwtile[j][i+1] != NULL) {
            mat_lat_pkt_log<<"\t"<<(noc.nwtile[j][i+1])->return_latency(W);
            mat_lat_flit_log<<"\t"<<(noc.nwtile[j][i+1])->return_latency_flit(W);
            mat_tput_log<<"\t"<<(noc.nwtile[j][i+1])->return_avg_tput(W);
        }
        else {
            mat_lat_pkt_log<<"\t0.0";
            mat_lat_flit_log<<"\t0.0";
            mat_tput_log<<"\t0.0";
        }
				
				if(j != num_rows - 1) {
					mat_lat_pkt_log<<"\t"<<null<<"\t"<<null;
					mat_lat_flit_log<<"\t"<<null<<"\t"<<null;
					mat_tput_log<<"\t"<<null<<"\t"<<null;
			
				}
			}
			mat_lat_pkt_log<<endl;
			mat_lat_flit_log<<endl;
			mat_tput_log<<endl;
		}
	}
    
    for (UI i = 0; i < num_cols; i++)
        for (UI j = 0; j < num_rows; j++) {
            if (noc.nwtile[i][j] != NULL) {
                mat_lat_core_pkt_log<<(noc.nwtile[i][j])->return_latency_core()<<endl;
                mat_lat_core_flit_log<<(noc.nwtile[i][j])->return_latency_flit_core()<<endl;
                mat_core_tput_log<<(noc.nwtile[i][j])->return_avg_tput_core()<<endl;
            }
            else {
                mat_lat_core_pkt_log<<"0.0"<<endl;
                mat_lat_core_flit_log<<"0.0"<<endl;
                mat_core_tput_log<<"0.0"<<endl;
            }
        }
	
	// generate gnuplot logs
	for(UI i = 0; i < num_cols; i++) {
		for(UI j = 0; j < num_rows - 1; j++) {
			double x = (double)i;
			double y = (double)j + 0.2 + j;
			double null = 0.0;
			ylatency_log<<"\t"<<x+0.1<<"\t"<<y<<"\t"<<null<<endl;
			ylatency_log<<"\t"<<(x + 0.1)<<"\t"<<y<<"\t"<<(noc.nwtile[j+1][i])->return_latency(N)<<endl;
			ylatency_log<<"\t"<<(x + 0.3)<<"\t"<<y<<"\t"<<(noc.nwtile[j+1][i])->return_latency(N)<<endl;
			ylatency_log<<"\t"<<(x+0.3)<<"\t"<<y<<"\t"<<null<<endl;
			ylatency_log<<"\t"<<(x+0.7)<<"\t"<<y<<"\t"<<null<<endl;
			ylatency_log<<"\t"<<(x + 0.7)<<"\t"<<y<<"\t"<<(noc.nwtile[j][i])->return_latency(S)<<endl;
			ylatency_log<<"\t"<<(x+0.9)<<"\t"<<y<<"\t"<<(noc.nwtile[j][i])->return_latency(S)<<endl;
			ylatency_log<<"\t"<<(x+0.9)<<"\t"<<y<<"\t"<<null<<endl;
			ylatency_log<<endl;
			
			ylatency_flit_log<<"\t"<<x+0.1<<"\t"<<y<<"\t"<<null<<endl;
			ylatency_flit_log<<"\t"<<(x + 0.1)<<"\t"<<y<<"\t"<<(noc.nwtile[j+1][i])->return_latency_flit(N)<<endl;
			ylatency_flit_log<<"\t"<<(x + 0.3)<<"\t"<<y<<"\t"<<(noc.nwtile[j+1][i])->return_latency_flit(N)<<endl;
			ylatency_flit_log<<"\t"<<(x+0.3)<<"\t"<<y<<"\t"<<null<<endl;
			ylatency_flit_log<<"\t"<<(x+0.7)<<"\t"<<y<<"\t"<<null<<endl;
			ylatency_flit_log<<"\t"<<(x + 0.7)<<"\t"<<y<<"\t"<<(noc.nwtile[j][i])->return_latency_flit(S)<<endl;
			ylatency_flit_log<<"\t"<<(x+0.9)<<"\t"<<y<<"\t"<<(noc.nwtile[j][i])->return_latency_flit(S)<<endl;
			ylatency_flit_log<<"\t"<<(x+0.9)<<"\t"<<y<<"\t"<<null<<endl;
			ylatency_flit_log<<endl;
			
			ytput_log<<"\t"<<x+0.1<<"\t"<<y<<"\t"<<null<<endl;
			ytput_log<<"\t"<<(x + 0.1)<<"\t"<<y<<"\t"<<(noc.nwtile[j+1][i])->return_avg_tput(N)<<endl;
			ytput_log<<"\t"<<(x + 0.3)<<"\t"<<y<<"\t"<<(noc.nwtile[j+1][i])->return_avg_tput(N)<<endl;
			ytput_log<<"\t"<<(x+0.3)<<"\t"<<y<<"\t"<<null<<endl;
			ytput_log<<"\t"<<(x+0.7)<<"\t"<<y<<"\t"<<null<<endl;
			ytput_log<<"\t"<<(x + 0.7)<<"\t"<<y<<"\t"<<(noc.nwtile[j][i])->return_avg_tput(S)<<endl;
			ytput_log<<"\t"<<(x+0.9)<<"\t"<<y<<"\t"<<(noc.nwtile[j][i])->return_avg_tput(S)<<endl;
			ytput_log<<"\t"<<(x+0.9)<<"\t"<<y<<"\t"<<null<<endl;
			ytput_log<<endl;
						
			y = (double)j + 0.8 + j;
			
			ylatency_log<<"\t"<<x+0.1<<"\t"<<y<<"\t"<<null<<endl;
			ylatency_log<<"\t"<<(x + 0.1)<<"\t"<<y<<"\t"<<(noc.nwtile[j+1][i])->return_latency(N)<<endl;
			ylatency_log<<"\t"<<(x + 0.3)<<"\t"<<y<<"\t"<<(noc.nwtile[j+1][i])->return_latency(N)<<endl;
			ylatency_log<<"\t"<<(x+0.3)<<"\t"<<y<<"\t"<<null<<endl;
			ylatency_log<<"\t"<<(x+0.7)<<"\t"<<y<<"\t"<<null<<endl;
			ylatency_log<<"\t"<<(x + 0.7)<<"\t"<<y<<"\t"<<(noc.nwtile[j][i])->return_latency(S)<<endl;
			ylatency_log<<"\t"<<(x+0.9)<<"\t"<<y<<"\t"<<(noc.nwtile[j][i])->return_latency(S)<<endl;
			ylatency_log<<"\t"<<(x+0.9)<<"\t"<<y<<"\t"<<null<<endl;
			ylatency_log<<endl<<endl;
			
			ylatency_flit_log<<"\t"<<x+0.1<<"\t"<<y<<"\t"<<null<<endl;
			ylatency_flit_log<<"\t"<<(x + 0.1)<<"\t"<<y<<"\t"<<(noc.nwtile[j+1][i])->return_latency_flit(N)<<endl;
			ylatency_flit_log<<"\t"<<(x + 0.3)<<"\t"<<y<<"\t"<<(noc.nwtile[j+1][i])->return_latency_flit(N)<<endl;
			ylatency_flit_log<<"\t"<<(x+0.3)<<"\t"<<y<<"\t"<<null<<endl;
			ylatency_flit_log<<"\t"<<(x+0.7)<<"\t"<<y<<"\t"<<null<<endl;
			ylatency_flit_log<<"\t"<<(x + 0.7)<<"\t"<<y<<"\t"<<(noc.nwtile[j][i])->return_latency_flit(S)<<endl;
			ylatency_flit_log<<"\t"<<(x+0.9)<<"\t"<<y<<"\t"<<(noc.nwtile[j][i])->return_latency_flit(S)<<endl;
			ylatency_flit_log<<"\t"<<(x+0.9)<<"\t"<<y<<"\t"<<null<<endl;
			ylatency_flit_log<<endl<<endl;
		
			ytput_log<<"\t"<<x+0.1<<"\t"<<y<<"\t"<<null<<endl;
			ytput_log<<"\t"<<(x + 0.1)<<"\t"<<y<<"\t"<<(noc.nwtile[j+1][i])->return_avg_tput(N)<<endl;
			ytput_log<<"\t"<<(x + 0.3)<<"\t"<<y<<"\t"<<(noc.nwtile[j+1][i])->return_avg_tput(N)<<endl;
			ytput_log<<"\t"<<(x+0.3)<<"\t"<<y<<"\t"<<null<<endl;
			ytput_log<<"\t"<<(x+0.7)<<"\t"<<y<<"\t"<<null<<endl;
			ytput_log<<"\t"<<(x + 0.7)<<"\t"<<y<<"\t"<<(noc.nwtile[j][i])->return_avg_tput(S)<<endl;
			ytput_log<<"\t"<<(x+0.9)<<"\t"<<y<<"\t"<<(noc.nwtile[j][i])->return_avg_tput(S)<<endl;
			ytput_log<<"\t"<<(x+0.9)<<"\t"<<y<<"\t"<<null<<endl;
			ytput_log<<endl<<endl;
			
		}
	}
	
	for(UI i = 0; i < num_rows; i++) {
		for(UI j = 0; j < num_cols - 1; j++) {
			double x = (double)i + i;
			double y = (double)j + 0.2;
			double null = 0.0;
      
      if (noc.nwtile[i][j] == NULL)
          continue;
			
			xlatency_log<<"\t"<<y<<"\t"<<x+0.1<<"\t"<<null<<endl;
			xlatency_log<<"\t"<<y<<"\t"<<(x + 0.1)<<"\t"<<(noc.nwtile[i][j])->return_latency(E)<<endl;
			xlatency_log<<"\t"<<y<<"\t"<<(x + 0.3)<<"\t"<<(noc.nwtile[i][j])->return_latency(E)<<endl;
			xlatency_log<<"\t"<<y<<"\t"<<(x+0.3)<<"\t"<<null<<endl;
			xlatency_log<<"\t"<<y<<"\t"<<(x+0.7)<<"\t"<<null<<endl;
			xlatency_log<<"\t"<<y<<"\t"<<(x + 0.7)<<"\t"<<(noc.nwtile[i][j+1])->return_latency(W)<<endl;
			xlatency_log<<"\t"<<y<<"\t"<<(x+0.9)<<"\t"<<(noc.nwtile[i][j+1])->return_latency(W)<<endl;
			xlatency_log<<"\t"<<y<<"\t"<<(x+0.9)<<"\t"<<null<<endl;
			xlatency_log<<endl;
			
			xlatency_flit_log<<"\t"<<y<<"\t"<<x+0.1<<"\t"<<null<<endl;
			xlatency_flit_log<<"\t"<<y<<"\t"<<(x + 0.1)<<"\t"<<(noc.nwtile[i][j])->return_latency_flit(E)<<endl;
			xlatency_flit_log<<"\t"<<y<<"\t"<<(x + 0.3)<<"\t"<<(noc.nwtile[i][j])->return_latency_flit(E)<<endl;
			xlatency_flit_log<<"\t"<<y<<"\t"<<(x+0.3)<<"\t"<<null<<endl;
			xlatency_flit_log<<"\t"<<y<<"\t"<<(x+0.7)<<"\t"<<null<<endl;
			xlatency_flit_log<<"\t"<<y<<"\t"<<(x + 0.7)<<"\t"<<(noc.nwtile[i][j+1])->return_latency_flit(W)<<endl;
			xlatency_flit_log<<"\t"<<y<<"\t"<<(x+0.9)<<"\t"<<(noc.nwtile[i][j+1])->return_latency_flit(W)<<endl;
			xlatency_flit_log<<"\t"<<y<<"\t"<<(x+0.9)<<"\t"<<null<<endl;
			xlatency_flit_log<<endl;
			
			xtput_log<<"\t"<<y<<"\t"<<x+0.1<<"\t"<<null<<endl;
			xtput_log<<"\t"<<y<<"\t"<<(x + 0.1)<<"\t"<<(noc.nwtile[i][j])->return_avg_tput(E)<<endl;
			xtput_log<<"\t"<<y<<"\t"<<(x + 0.3)<<"\t"<<(noc.nwtile[i][j])->return_avg_tput(E)<<endl;
			xtput_log<<"\t"<<y<<"\t"<<(x+0.3)<<"\t"<<null<<endl;
			xtput_log<<"\t"<<y<<"\t"<<(x+0.7)<<"\t"<<null<<endl;
			xtput_log<<"\t"<<y<<"\t"<<(x + 0.7)<<"\t"<<(noc.nwtile[i][j+1])->return_avg_tput(W)<<endl;
			xtput_log<<"\t"<<y<<"\t"<<(x+0.9)<<"\t"<<(noc.nwtile[i][j+1])->return_avg_tput(W)<<endl;
			xtput_log<<"\t"<<y<<"\t"<<(x+0.9)<<"\t"<<null<<endl;
			xtput_log<<endl;
			
			y = (double)j + 0.8;
			
			xlatency_log<<"\t"<<y<<"\t"<<x+0.1<<"\t"<<null<<endl;
			xlatency_log<<"\t"<<y<<"\t"<<(x + 0.1)<<"\t"<<(noc.nwtile[i][j])->return_latency(E)<<endl;
			xlatency_log<<"\t"<<y<<"\t"<<(x + 0.3)<<"\t"<<(noc.nwtile[i][j])->return_latency(E)<<endl;
			xlatency_log<<"\t"<<y<<"\t"<<(x+0.3)<<"\t"<<null<<endl;
			xlatency_log<<"\t"<<y<<"\t"<<(x+0.7)<<"\t"<<null<<endl;
			xlatency_log<<"\t"<<y<<"\t"<<(x + 0.7)<<"\t"<<(noc.nwtile[i][j+1])->return_latency(W)<<endl;		
			xlatency_log<<"\t"<<y<<"\t"<<(x+0.9)<<"\t"<<(noc.nwtile[i][j+1])->return_latency(W)<<endl;
			xlatency_log<<"\t"<<y<<"\t"<<(x+0.9)<<"\t"<<null<<endl;
			xlatency_log<<endl<<endl;
			
			xlatency_flit_log<<"\t"<<y<<"\t"<<x+0.1<<"\t"<<null<<endl;
			xlatency_flit_log<<"\t"<<y<<"\t"<<(x + 0.1)<<"\t"<<(noc.nwtile[i][j])->return_latency_flit(E)<<endl;
			xlatency_flit_log<<"\t"<<y<<"\t"<<(x + 0.3)<<"\t"<<(noc.nwtile[i][j])->return_latency_flit(E)<<endl;
			xlatency_flit_log<<"\t"<<y<<"\t"<<(x+0.3)<<"\t"<<null<<endl;
			xlatency_flit_log<<"\t"<<y<<"\t"<<(x+0.7)<<"\t"<<null<<endl;
			xlatency_flit_log<<"\t"<<y<<"\t"<<(x + 0.7)<<"\t"<<(noc.nwtile[i][j+1])->return_latency_flit(W)<<endl;
			xlatency_flit_log<<"\t"<<y<<"\t"<<(x+0.9)<<"\t"<<(noc.nwtile[i][j+1])->return_latency_flit(W)<<endl;
			xlatency_flit_log<<"\t"<<y<<"\t"<<(x+0.9)<<"\t"<<null<<endl;
			xlatency_flit_log<<endl<<endl;
			
			xtput_log<<"\t"<<y<<"\t"<<x+0.1<<"\t"<<null<<endl;
			xtput_log<<"\t"<<y<<"\t"<<(x + 0.1)<<"\t"<<(noc.nwtile[i][j])->return_avg_tput(E)<<endl;
			xtput_log<<"\t"<<y<<"\t"<<(x + 0.3)<<"\t"<<(noc.nwtile[i][j])->return_avg_tput(E)<<endl;
			xtput_log<<"\t"<<y<<"\t"<<(x+0.3)<<"\t"<<null<<endl;
			xtput_log<<"\t"<<y<<"\t"<<(x+0.7)<<"\t"<<null<<endl;
			xtput_log<<"\t"<<y<<"\t"<<(x + 0.7)<<"\t"<<(noc.nwtile[i][j+1])->return_avg_tput(W)<<endl;		
			xtput_log<<"\t"<<y<<"\t"<<(x+0.9)<<"\t"<<(noc.nwtile[i][j+1])->return_avg_tput(W)<<endl;
			xtput_log<<"\t"<<y<<"\t"<<(x+0.9)<<"\t"<<null<<endl;
			xtput_log<<endl<<endl;
			
		}
	}
	
    //compute all travelled packets/flits and overall avg latency
	ULL tiles_count = num_rows * num_cols;
    ULL noc_total_flits = 0;
    ULL noc_total_packets = 0;
    ULL noc_total_latency = 0;
    ULL noc_total_latency_core = 0;
    ULL noc_total_flits_send = 0;
    ULL noc_total_packets_send = 0;
    ULL noc_total_flits_recv = 0;
    ULL noc_total_packets_recv = 0;
    ULL noc_unrouted_wc_latency = 0;
	double noc_wc_latency_core = 0.0;
    double noc_bufs_util = 0.0;
    double noc_vcs_util = 0.0;
    double noc_unrouted_avg_latency = 0.0;
    for(UI i = 0; i < num_rows; i++) {
		for(UI j = 0; j < num_cols; j++) {
      if (noc.nwtile[i][j] == NULL)
          continue;
      
			noc_total_packets += (noc.nwtile[i][j])->return_total_packets();
			noc_total_flits += (noc.nwtile[i][j])->return_total_flits();
            noc_total_latency += (noc.nwtile[i][j])->return_total_latency();
            noc_total_latency_core += (noc.nwtile[i][j])->return_total_latency_core();
            noc_total_flits_send += (noc.nwtile[i][j])->return_send_flits_number();
            noc_total_packets_send += (noc.nwtile[i][j])->return_send_packets_number();
            noc_total_flits_recv += (noc.nwtile[i][j])->return_recv_flits_number();
            noc_total_packets_recv += (noc.nwtile[i][j])->return_recv_packets_number();
			noc_wc_latency_core += (noc.nwtile[i][j])->return_wc_latency_flit_core();
            noc_bufs_util += (noc.nwtile[i][j])->return_bufs_util();
            noc_vcs_util += (noc.nwtile[i][j])->return_vcs_util();
            noc_unrouted_avg_latency += (noc.nwtile[i][j])->return_avr_latency_unrouted();
            if ((noc.nwtile[i][j])->return_wc_latency_unrouted() > noc_unrouted_wc_latency)
                noc_unrouted_wc_latency = (noc.nwtile[i][j])->return_wc_latency_unrouted();
		}
	}
	
	double noc_latency = (double)noc_total_latency / noc_total_flits;
    double noc_latency_core = (double)noc_total_latency_core / noc_total_flits_recv;
    double noc_latency_packet = (double)noc_total_latency / noc_total_packets;
    double noc_latency_core_packet = (double)noc_total_latency_core / noc_total_packets_recv;
	noc_wc_latency_core = noc_wc_latency_core / tiles_count;
    noc_bufs_util = noc_bufs_util / tiles_count;
    noc_vcs_util = noc_vcs_util / tiles_count;
    noc_unrouted_avg_latency = noc_unrouted_avg_latency / tiles_count;
	
    string turn_routing_type_str = " TR_TYPE = ";
    switch (TR_TYPE) {
        case RANDOM: turn_routing_type_str += string("RANDOM "); break;
        case CONGESTION: turn_routing_type_str += string("CONGESTION "); break;
        default: turn_routing_type_str += string("Not Defined! "); break;
    }
    
    results_log<<"\nSend packets      = "<<noc_total_packets_send<<"  and flits = "<<noc_total_flits_send<<endl;
    results_log<<"Recieved packets  = "<<noc_total_packets_recv<<"  and flits = "<<noc_total_flits_recv<<endl;
    results_log<<"Travelled packets  = "<<noc_total_packets<<" and flits = "<<noc_total_flits<<endl;
    
    cout<<"\nSend packets      = "<<noc_total_packets_send<<"  and flits = "<<noc_total_flits_send<<endl;
    cout<<"Recieved packets  = "<<noc_total_packets_recv<<"  and flits = "<<noc_total_flits_recv<<endl;
    cout<<"Travelled packets = "<<noc_total_packets<<" and flits = "<<noc_total_flits<<endl;
    
    if (ADDITIONAL_INFO) {
        results_log<<"RT_ALGO = ";
        cout<<"RT_ALGO = ";
        switch (RT_ALGO) {
            case SOURCE: results_log<<"SOURCE "; cout<<"SOURCE "; break;
            case XY: results_log<<"XY "; cout<<"XY "; break;
            case OE: results_log<<"OE "; cout<<"OE "; break;
            case DyXY: results_log<<"DyXY "; cout<<"DyXY "; break;
            case DyAD_OE: results_log<<"DyAD_OE "; cout<<"DyAD_OE "; break;
            case West_First: results_log<<"West_First"<<turn_routing_type_str; cout<<"West_First"<<turn_routing_type_str; break;
            case North_Last: results_log<<"North_Last"<<turn_routing_type_str; cout<<"North_Last"<<turn_routing_type_str; break;
            case Negative_First: results_log<<"Negative_First"<<turn_routing_type_str; cout<<"Negative_First"<<turn_routing_type_str; break;
            case DyBM: results_log<<"DyBM "; cout<<"DyBM "; break;
            case DyXY_FT: results_log<<"DyXY_FT "; cout<<"DyXY_FT "; break;
            default: results_log<<"Not Defined! "; cout<<"Not Defined! "; break;
        }
        cout<<"IAT_TYPE = ";
        results_log<<"IAT_TYPE = ";
        switch (IAT_TYPE) {
            case SEQUENCE: results_log<<"SEQUENCE "; cout<<"SEQUENCE "; break;
            case RR: results_log<<"RR "; cout<<"RR "; break;
            case AA: results_log<<"AA CONGESTION_LEVEL = "<<CONGESTION_LEVEL<<" CONGESTION_PRIORITY = "<<CONGESTION_PRIORITY<<endl;
                     results_log<<"AA CONGESTION_AFFECT_VC = "<<((CONGESTION_AFFECT_VC == false) ? "false" : "true")<<" CONGESTION_USE = "<<((CONGESTION_USE == false) ? "false" : "true")<<endl;
                     results_log<<"AA HOP_LEVEL = "<<HOP_LEVEL<<" HOP_USE = "<<((HOP_USE == false) ? "false" : "true")<<endl;
                     cout<<"AA CONGESTION_LEVEL = "<<CONGESTION_LEVEL<<" CONGESTION_PRIORITY = "<<CONGESTION_PRIORITY<<endl;
                     cout<<"AA CONGESTION_AFFECT_VC = "<<((CONGESTION_AFFECT_VC == false) ? "false" : "true")<<" CONGESTION_USE = "<<((CONGESTION_USE == false) ? "false" : "true")<<endl;
                     cout<<"AA HOP_LEVEL = "<<HOP_LEVEL<<" HOP_USE = "<<((HOP_USE == false) ? "false" : "true")<<endl;
                     break;
            default: results_log<<"Not Defined!"; cout<<"Not Defined!"; break;
        }
        
        results_log<<"NUM_BUFS = "<<NUM_BUFS<<" NUM_VCS = "<<NUM_VCS<<endl;
        cout<<"NUM_BUFS = "<<NUM_BUFS<<" NUM_VCS = "<<NUM_VCS<<endl; 
        results_log<<"WARMUP = "<<WARMUP<<" TG_NUM = "<<TG_NUM<<" SIM_NUM = "<<SIM_NUM<<endl; 
        cout<<"WARMUP = "<<WARMUP<<" TG_NUM = "<<TG_NUM<<" SIM_NUM = "<<SIM_NUM<<endl; 
        results_log<<"WRITE_THROUGH_OUTPORT = "<<((WRITE_THROUGH_OUTPORT == false) ? "false" : "true")<<endl;
        cout<<"WRITE_THROUGH_OUTPORT = "<<((WRITE_THROUGH_OUTPORT == false) ? "false" : "true")<<endl;
    }
    
    ULL unrouted_packets = noc_total_packets_send - noc_total_packets_recv;
    ULL unrouted_flits   = noc_total_flits_send - noc_total_flits_recv;
    if ((unrouted_packets > 0) || (unrouted_flits > 0)) {
        results_log<<"\n!! Unrouted packets = "<<unrouted_packets<<" and flits = "<<unrouted_flits<<endl;
        results_log<<"Average unrouted flit latency    (in clock cycles per flit) = "<<noc_unrouted_avg_latency<<endl;
        results_log<<"Worst-case unrouted flit latency (in clock cycles per flit) = "<<noc_unrouted_wc_latency<<endl;
        results_log<<"Other statistical data is not 100% correct. ONLY routed flits taked into account."<<endl<<endl;
    }
    
    results_log<<"Overall average NoC latency       (in clock cycles per flit)   = "<<noc_latency_core<<endl;
    results_log<<"Overall average NoC latency       (in clock cycles per packet) = "<<noc_latency_core_packet<<endl;
	results_log<<"Worst-case NoC latency            (in clock cycles per flit)   = "<<noc_wc_latency_core<<endl;
	results_log<<"Overall average router latency    (in clock cycles per flit)   = "<<noc_latency<<endl;
    results_log<<"Overall average router latency    (in clock cycles per packet) = "<<noc_latency_packet<<endl;
    
    // compute overall avg throughput
    double noc_total_tput_N = 0;
    double noc_total_tput_S = 0;
    double noc_total_tput_W = 0;
    double noc_total_tput_E = 0;
    double noc_total_tput = 0;
    double noc_total_tput_core = 0;
    ULL noc_total_channels_NS = ((num_rows - 1) * num_cols);
    ULL noc_total_channels_EW = ((num_cols - 1) * num_rows);
    for(UI i = 0; i < num_rows; i++) {
		for(UI j = 0; j < num_cols; j++) {
            if (noc.nwtile[i][j] == NULL)
                continue;
            
            if (noc.nwtile[i][j]->getportid(N) != ND)
                noc_total_tput_N += (noc.nwtile[i][j])->return_avg_tput(N);
            if (noc.nwtile[i][j]->getportid(S) != ND)
                noc_total_tput_S += (noc.nwtile[i][j])->return_avg_tput(S);
            if (noc.nwtile[i][j]->getportid(W) != ND)
                noc_total_tput_W += (noc.nwtile[i][j])->return_avg_tput(W);
            if (noc.nwtile[i][j]->getportid(E) != ND)
                noc_total_tput_E += (noc.nwtile[i][j])->return_avg_tput(E);    
            noc_total_tput_core += (noc.nwtile[i][j])->return_avg_tput_core();
        }
	}
    noc_total_tput_N = (double)noc_total_tput_N / noc_total_channels_NS;
    noc_total_tput_S = (double)noc_total_tput_S / noc_total_channels_NS;
    noc_total_tput_W = (double)noc_total_tput_W / noc_total_channels_EW;
    noc_total_tput_E = (double)noc_total_tput_E / noc_total_channels_EW;
    
    noc_total_tput = (noc_total_tput_N + noc_total_tput_S + noc_total_tput_W + noc_total_tput_E) / 4;
    noc_total_tput_core = (double)noc_total_tput_core / tiles_count;
    
    results_log<<"\nOverall average NoC throughput             (Gbps)              = "<<noc_total_tput_core<<endl;
    results_log<<"Overall average router throughput          (Gbps)              = "<<noc_total_tput<<endl;
    results_log<<"Overall average router throughput in North (Gbps)              = "<<noc_total_tput_N<<endl;
    results_log<<"Overall average router throughput in South (Gbps)              = "<<noc_total_tput_S<<endl;
    results_log<<"Overall average router throughput in West  (Gbps)              = "<<noc_total_tput_W<<endl;
    results_log<<"Overall average router throughput in East  (Gbps)              = "<<noc_total_tput_E<<endl;
	
	
	double noc_wc_num_waits  = 0.0;
	double noc_wc_num_sw     = 0.0;
	double noc_avg_num_waits = 0.0;
	double noc_avg_num_sw    = 0.0;
	for(UI i = 0; i < num_rows; i++) {
		for(UI j = 0; j < num_cols; j++) {
      if (noc.nwtile[i][j] == NULL)
          continue;
      
			noc_wc_num_waits += (noc.nwtile[i][j])->return_wc_num_waits();
			noc_wc_num_sw += (noc.nwtile[i][j])->return_wc_num_sw();
			noc_avg_num_waits += (noc.nwtile[i][j])->return_avg_num_waits();
			noc_avg_num_sw += (noc.nwtile[i][j])->return_avg_num_sw();
		}
	}
	
	noc_wc_num_waits = (double)noc_wc_num_waits / tiles_count;
	noc_wc_num_sw = (double)noc_wc_num_sw / tiles_count;
	noc_avg_num_waits = (double)noc_avg_num_waits / tiles_count;
	noc_avg_num_sw = (double)noc_avg_num_sw / tiles_count;
	
	results_log<<"\nWorst-case NoC number of waits      (in clock cycles per flit) = "<<noc_wc_num_waits<<endl;
    results_log<<"Overall average NoC number of waits (in clock cycles per flit) = "<<noc_avg_num_waits<<endl;
    results_log<<"Worst-case NoC number of hops       (number)                   = "<<noc_wc_num_sw<<endl;
    results_log<<"Overall average NoC number of hops  (number)                   = "<<noc_avg_num_sw<<endl;
	
    results_log<<"\nAverage buffers utilization      (in percent) = "<<noc_bufs_util
               <<" and virtual channels utilization (in percent)   = "<<noc_vcs_util<<endl; 
	results_log<<"\nEfficienty of NoC buffers policy (in percent) = "<<(double)(100 - ((noc_avg_num_waits / noc_latency_core) * 100))
               <<" { if avg. NoC latency (in clock cycles per flit) = "<<noc_latency_core<<" }"<<endl;
	
	// close log files
	//sc_close_vcd_trace_file(tracefile);
	eventlog.close();
	ylatency_log.close();
	xlatency_log.close();
	ylatency_flit_log.close();
	xlatency_flit_log.close();
	ytput_log.close();
	xtput_log.close();
	mat_lat_pkt_log.close();
	mat_lat_flit_log.close();
	mat_tput_log.close();
    mat_lat_core_pkt_log.close();
	mat_lat_core_flit_log.close();
	mat_core_tput_log.close();
	results_log.close();
	
	// generate gnuplot graphs
	system("mkdir results/graphs");
	system("gnuplot gnuplot/results.gnu");
	cmd = string("mv results/graphs/* ") + DIRNAME + string("/graphs");
	system(cmd.c_str());
	system("rm -rf results/graphs");
    time_elapsed = (clock() - time_elapsed)/CLOCKS_PER_SEC;
    UI  sec, min, hour;
    hour = time_elapsed / 3600;
    min  = (time_elapsed / 60) - (hour * 60); 
    sec  = time_elapsed % 60;
	
	cout<<"---------------------------------------------------------------------------"<<endl;
	cout<<"Simulation complete!"<<endl;
    printf("Time elapsed: %02d:%02d:%02d\n", hour, min, sec);
	string str_event_log = string("log/nirgam/event.log");
	cout<<"Event Log created in "<<str_event_log<<endl;
	string result_dir = DIRNAME;
	cout<<"Simulation results and gnuplot graphs stored in "<<result_dir<<endl;
	string matlab_logs = string("log/matlab");
	cout<<"Input data files for matlab created in "<<matlab_logs<<endl;
	string matlab_script = string("matlab");
	cout<<"Run \"plot_graphs\" and \"plot_core_graphs\" from directory "<<matlab_script<<" to generate graphs in matlab"<<endl;
	cout<<"Exit NIRGAM!"<<endl;
	cout<<"---------------------------------------------------------------------------"<<endl;
    
    if (MATLAB_MAKE_IMAGES) {
        cout<<endl;
        system("mkdir matlab/images");
        string str_matlab_images = DIRNAME + string("/matlab_images");
        cout<<"Matlab will now make images and saved it to "<<str_matlab_images<<endl;
        system("matlab -nodisplay < matlab/images_scripts/plot_graphs_images.m");
        system("matlab -nodisplay < matlab/images_scripts/plot_core_graphs_images.m");
        cmd = string("mv matlab/images/* ") + str_matlab_images;
        system(cmd.c_str());
        system("rm -rf matlab/images");
        cout<<endl;
    }
	return 0;
}
