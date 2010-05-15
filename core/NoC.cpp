
/*
 * NoC.cpp
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
/// \file NoC.cpp
/// \brief Creates network topology
///
/// This file creates a 2-dimensional topology of network tiles and interconnects them.
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#include "NoC.h"
#include <string>
#include <fstream>
#include <iostream>

////////////////////////////////////////////////////////////////////////////////////////////////////
/// Constructor to generate topology.
/// It creates 2-d toroidal or non- toroidal mesh of m x n network tiles depending on input from user.
/// \param NoC NoC name
/// \param num_rows number of rows in topology
/// \param num_cols number of columns in topology
/// \param isProgBar is progress bar drawing or not
/// .
/// The constructor implements the following:
/// - interconnect network tiles
/// - assign tile IDs
/// - connect clock signal to clock input port of tiles.
////////////////////////////////////////////////////////////////////////////////////////////////////
NoC::NoC(sc_module_name NoC, UI num_rows, UI num_cols, bool isProgBar): sc_module(NoC) {

	rows            = num_rows;
	cols            = num_cols;
    sim_count       = 0;
    drawProgressBar = isProgBar;
	
	for(UI i = 0; i < rows; i++) {
		for(UI j = 0; j < cols; j++) {
			char tileName[19];
			sprintf(tileName, "nwtile[%d][%d]",i,j);
			UI id = j + i * cols;
            
            if (ADDITIONAL_INFO)
                cout<<"Creating tile "<<id<<" "<<tileName<<endl;
			
			switch(TOPO) {
			
			case TORUS:
				nwtile[i][j] = new NWTile<NUM_NB, NUM_IC, NUM_OC>(tileName, id);	// create a tile
				(ptr nwtile[i][j])->switch_cntrl(*nw_clock);		// connect clock
				break;
			

			case MESH:
				if(corner(id)) {
					nwtile[i][j] = new NWTile<NUM_NB_C, NUM_IC_C, NUM_OC_C>(tileName, id);
					(ptr_c nwtile[i][j])->switch_cntrl(*nw_clock);
				}
				else if(border(id)) {
					nwtile[i][j] = new NWTile<NUM_NB_B, NUM_IC_B, NUM_OC_B>(tileName, id);
					(ptr_b nwtile[i][j])->switch_cntrl(*nw_clock);
				}
				else {
					nwtile[i][j] = new NWTile<NUM_NB, NUM_IC, NUM_OC>(tileName, id);
					(ptr nwtile[i][j])->switch_cntrl(*nw_clock);
				}
				break;
			}
		}
	}

	for(UI i = 0; i < rows; i++) {
		for(UI j = 0; j < cols; j++) {

			UI id = j + i * cols;
			UI id_S = j + ((i+1) % rows) * cols;
			UI id_E = (j+1) % cols + i * cols;

			switch(TOPO) {
			
			case TORUS:
				//connect data line to South neihbor
				(ptr nwtile[i][j])->op_port[1](sigs[i][j].sig_toS);
				(ptr nwtile[(i+1)%rows][j])->ip_port[0](sigs[i][j].sig_toS);
	
				//connect credit line to South neihbor
				for(UI k = 0; k < NUM_VCS; k++) {
					(ptr nwtile[(i+1)%rows][j])->credit_in[0][k](sigs[i][j].cr_sig_toS[k]);
					(ptr nwtile[i][j])->credit_out[1][k](sigs[i][j].cr_sig_toS[k]);
				}
                
                //connect stress line to South neihbor
				(ptr nwtile[i][j])->stress_value_out[1](sigs[i][j].sv_sig_toS);
				(ptr nwtile[(i+1)%rows][j])->stress_value_in[0](sigs[i][j].sv_sig_toS);
                
                //connect congestion flags line to South neihbor
				(ptr nwtile[i][j])->congestion_flag_out[1](sigs[i][j].cf_sig_toS);
				(ptr nwtile[(i+1)%rows][j])->congestion_flag_in[0](sigs[i][j].cf_sig_toS);
                
                //connect congestion status line to South neihbor
				for(UI k = 0; k < NUM_VCS; k++) {
					(ptr nwtile[(i+1)%rows][j])->congestion_status_in[0][k](sigs[i][j].cs_sig_toS[k]);
					(ptr nwtile[i][j])->congestion_status_out[1][k](sigs[i][j].cs_sig_toS[k]);
				}
	
				// connect data line from South neighbor
				(ptr nwtile[i][j])->ip_port[1](sigs[i][j].sig_fromS);
				(ptr nwtile[(i+1)%rows][j])->op_port[0](sigs[i][j].sig_fromS);
	
				// connect credit line from South neighbor
				for(UI k = 0; k < NUM_VCS; k++) {
					(ptr nwtile[(i+1)%rows][j])->credit_out[0][k](sigs[i][j].cr_sig_fromS[k]);
					(ptr nwtile[i][j])->credit_in[1][k](sigs[i][j].cr_sig_fromS[k]);
				}
                
                // connect stress line from South neighbor
				(ptr nwtile[i][j])->stress_value_in[1](sigs[i][j].sv_sig_fromS);
				(ptr nwtile[(i+1)%rows][j])->stress_value_out[0](sigs[i][j].sv_sig_fromS);
                
                // connect congestion flags line from South neighbor
				(ptr nwtile[i][j])->congestion_flag_in[1](sigs[i][j].cf_sig_fromS);
				(ptr nwtile[(i+1)%rows][j])->congestion_flag_out[0](sigs[i][j].cf_sig_fromS);
                
                // connect congestion status line from South neighbor
				for(UI k = 0; k < NUM_VCS; k++) {
					(ptr nwtile[(i+1)%rows][j])->congestion_status_out[0][k](sigs[i][j].cs_sig_fromS[k]);
					(ptr nwtile[i][j])->congestion_status_in[1][k](sigs[i][j].cs_sig_fromS[k]);
				}
	
				// connect data line from East neighbor
				(ptr nwtile[i][j])->ip_port[2](sigs[i][j].sig_fromE);
				(ptr nwtile[i][(j+1)%cols])->op_port[3](sigs[i][j].sig_fromE);
	
				// connect credit line from East neighbor
				for(UI k = 0; k < NUM_VCS; k++) {
					(ptr nwtile[i][(j+1)%cols])->credit_out[3][k](sigs[i][j].cr_sig_fromE[k]);
					(ptr nwtile[i][j])->credit_in[2][k](sigs[i][j].cr_sig_fromE[k]);
				}
                
                // connect stress line from East neighbor
				(ptr nwtile[i][j])->stress_value_in[2](sigs[i][j].sv_sig_fromE);
				(ptr nwtile[i][(j+1)%cols])->stress_value_out[3](sigs[i][j].sv_sig_fromE);
                
                // connect congestion flags line from East neighbor
				(ptr nwtile[i][j])->congestion_flag_in[2](sigs[i][j].cf_sig_fromE);
				(ptr nwtile[i][(j+1)%cols])->congestion_flag_out[3](sigs[i][j].cf_sig_fromE);
                
                // connect congestion status line from East neighbor
				for(UI k = 0; k < NUM_VCS; k++) {
					(ptr nwtile[i][(j+1)%cols])->congestion_status_out[3][k](sigs[i][j].cs_sig_fromE[k]);
					(ptr nwtile[i][j])->congestion_status_in[2][k](sigs[i][j].cs_sig_fromE[k]);
				}
	
				//connect data line to East neighbor
				(ptr nwtile[i][j])->op_port[2](sigs[i][j].sig_toE);
				(ptr nwtile[i][(j+1)%cols])->ip_port[3](sigs[i][j].sig_toE);
	
				//connect credit line to East neighbor
				for(UI k = 0; k < NUM_VCS; k++) {
					(ptr nwtile[i][(j+1)%cols])->credit_in[3][k](sigs[i][j].cr_sig_toE[k]);
					(ptr nwtile[i][j])->credit_out[2][k](sigs[i][j].cr_sig_toE[k]);
				}
                
                //connect stress line to East neighbor
				(ptr nwtile[i][j])->stress_value_out[2](sigs[i][j].sv_sig_toE);
				(ptr nwtile[i][(j+1)%cols])->stress_value_in[3](sigs[i][j].sv_sig_toE);
                
                //connect congestion flags line to East neighbor
				(ptr nwtile[i][j])->congestion_flag_out[2](sigs[i][j].cf_sig_toE);
				(ptr nwtile[i][(j+1)%cols])->congestion_flag_in[3](sigs[i][j].cf_sig_toE);
                
                //connect congestion status line to East neighbor
				for(UI k = 0; k < NUM_VCS; k++) {
					(ptr nwtile[i][(j+1)%cols])->congestion_status_in[3][k](sigs[i][j].cs_sig_toE[k]);
					(ptr nwtile[i][j])->congestion_status_out[2][k](sigs[i][j].cs_sig_toE[k]);
				}
                
				break;
				

			case MESH:
				if(!borderS(nwtile[i][j]->tileID)) {
	
					// connect data, credit, stress, congestion flags and congestion status line to South neihbor
					if(corner(id)) {	// corner tile
						(ptr_c nwtile[i][j])->op_port[nwtile[i][j]->portS](sigs[i][j].sig_toS);
						(ptr_c nwtile[i][j])->ip_port[nwtile[i][j]->portS](sigs[i][j].sig_fromS);
                        
						for(UI k = 0; k < NUM_VCS; k++) {
							(ptr_c nwtile[i][j])->credit_out[nwtile[i][j]->portS][k](sigs[i][j].cr_sig_toS[k]);
							(ptr_c nwtile[i][j])->credit_in[nwtile[i][j]->portS][k](sigs[i][j].cr_sig_fromS[k]);
						}
                        
                        (ptr_c nwtile[i][j])->stress_value_out[nwtile[i][j]->portS](sigs[i][j].sv_sig_toS);
						(ptr_c nwtile[i][j])->stress_value_in[nwtile[i][j]->portS](sigs[i][j].sv_sig_fromS);
                        
                        (ptr_c nwtile[i][j])->congestion_flag_out[nwtile[i][j]->portS](sigs[i][j].cf_sig_toS);
						(ptr_c nwtile[i][j])->congestion_flag_in[nwtile[i][j]->portS](sigs[i][j].cf_sig_fromS);
                        
                        for(UI k = 0; k < NUM_VCS; k++) {
							(ptr_c nwtile[i][j])->congestion_status_out[nwtile[i][j]->portS][k](sigs[i][j].cs_sig_toS[k]);
							(ptr_c nwtile[i][j])->congestion_status_in[nwtile[i][j]->portS][k](sigs[i][j].cs_sig_fromS[k]);
						}
					}
					else if(border(id)) {	// border tile
						(ptr_b nwtile[i][j])->op_port[nwtile[i][j]->portS](sigs[i][j].sig_toS);
						(ptr_b nwtile[i][j])->ip_port[nwtile[i][j]->portS](sigs[i][j].sig_fromS);
                        
						for(UI k = 0; k < NUM_VCS; k++) {
							(ptr_b nwtile[i][j])->credit_out[nwtile[i][j]->portS][k](sigs[i][j].cr_sig_toS[k]);
							(ptr_b nwtile[i][j])->credit_in[nwtile[i][j]->portS][k](sigs[i][j].cr_sig_fromS[k]);
						}
                        
                        (ptr_b nwtile[i][j])->stress_value_out[nwtile[i][j]->portS](sigs[i][j].sv_sig_toS);
						(ptr_b nwtile[i][j])->stress_value_in[nwtile[i][j]->portS](sigs[i][j].sv_sig_fromS);
                        
                        (ptr_b nwtile[i][j])->congestion_flag_out[nwtile[i][j]->portS](sigs[i][j].cf_sig_toS);
						(ptr_b nwtile[i][j])->congestion_flag_in[nwtile[i][j]->portS](sigs[i][j].cf_sig_fromS);
                        
                        for(UI k = 0; k < NUM_VCS; k++) {
							(ptr_b nwtile[i][j])->congestion_status_out[nwtile[i][j]->portS][k](sigs[i][j].cs_sig_toS[k]);
							(ptr_b nwtile[i][j])->congestion_status_in[nwtile[i][j]->portS][k](sigs[i][j].cs_sig_fromS[k]);
						}
					}
					else {		// generic tile
						(ptr nwtile[i][j])->op_port[nwtile[i][j]->portS](sigs[i][j].sig_toS);
						(ptr nwtile[i][j])->ip_port[nwtile[i][j]->portS](sigs[i][j].sig_fromS);
                        
						for(UI k = 0; k < NUM_VCS; k++) {
							(ptr nwtile[i][j])->credit_out[nwtile[i][j]->portS][k](sigs[i][j].cr_sig_toS[k]);
							(ptr nwtile[i][j])->credit_in[nwtile[i][j]->portS][k](sigs[i][j].cr_sig_fromS[k]);
						}
                        
                        (ptr nwtile[i][j])->stress_value_out[nwtile[i][j]->portS](sigs[i][j].sv_sig_toS);
						(ptr nwtile[i][j])->stress_value_in[nwtile[i][j]->portS](sigs[i][j].sv_sig_fromS);
                        
                        (ptr nwtile[i][j])->congestion_flag_out[nwtile[i][j]->portS](sigs[i][j].cf_sig_toS);
						(ptr nwtile[i][j])->congestion_flag_in[nwtile[i][j]->portS](sigs[i][j].cf_sig_fromS);
                        
                        for(UI k = 0; k < NUM_VCS; k++) {
							(ptr nwtile[i][j])->congestion_status_out[nwtile[i][j]->portS][k](sigs[i][j].cs_sig_toS[k]);
							(ptr nwtile[i][j])->congestion_status_in[nwtile[i][j]->portS][k](sigs[i][j].cs_sig_fromS[k]);
						}
					}
	
					// connect data, credit, stress, congestion flags and congestion status line from South neighbor
					if(corner(id_S)) {	// South neighbor is corner tile
						(ptr_c nwtile[(i+1)%rows][j])->ip_port[nwtile[(i+1)%rows][j]->portN](sigs[i][j].sig_toS);
						(ptr_c nwtile[(i+1)%rows][j])->op_port[nwtile[(i+1)%rows][j]->portN](sigs[i][j].sig_fromS);
                        
						for(UI k = 0; k < NUM_VCS; k++) {
							(ptr_c nwtile[(i+1)%rows][j])->credit_in[nwtile[(i+1)%rows][j]->portN][k](sigs[i][j].cr_sig_toS[k]);
							(ptr_c nwtile[(i+1)%rows][j])->credit_out[nwtile[(i+1)%rows][j]->portN][k](sigs[i][j].cr_sig_fromS[k]);
						}
                        
                        (ptr_c nwtile[(i+1)%rows][j])->stress_value_in[nwtile[(i+1)%rows][j]->portN](sigs[i][j].sv_sig_toS);
						(ptr_c nwtile[(i+1)%rows][j])->stress_value_out[nwtile[(i+1)%rows][j]->portN](sigs[i][j].sv_sig_fromS);
                        
                        (ptr_c nwtile[(i+1)%rows][j])->congestion_flag_in[nwtile[(i+1)%rows][j]->portN](sigs[i][j].cf_sig_toS);
						(ptr_c nwtile[(i+1)%rows][j])->congestion_flag_out[nwtile[(i+1)%rows][j]->portN](sigs[i][j].cf_sig_fromS);
                        
                        for(UI k = 0; k < NUM_VCS; k++) {
							(ptr_c nwtile[(i+1)%rows][j])->congestion_status_in[nwtile[(i+1)%rows][j]->portN][k](sigs[i][j].cs_sig_toS[k]);
							(ptr_c nwtile[(i+1)%rows][j])->congestion_status_out[nwtile[(i+1)%rows][j]->portN][k](sigs[i][j].cs_sig_fromS[k]);
						}
					}
					else if(border(id_S)) {	// South neigbor is border tile
						(ptr_b nwtile[(i+1)%rows][j])->ip_port[nwtile[(i+1)%rows][j]->portN](sigs[i][j].sig_toS);
						(ptr_b nwtile[(i+1)%rows][j])->op_port[nwtile[(i+1)%rows][j]->portN](sigs[i][j].sig_fromS);
                        
						for(UI k = 0; k < NUM_VCS; k++) {
							(ptr_b nwtile[(i+1)%rows][j])->credit_in[nwtile[(i+1)%rows][j]->portN][k](sigs[i][j].cr_sig_toS[k]);
							(ptr_b nwtile[(i+1)%rows][j])->credit_out[nwtile[(i+1)%rows][j]->portN][k](sigs[i][j].cr_sig_fromS[k]);
						}
                        
                        (ptr_b nwtile[(i+1)%rows][j])->stress_value_in[nwtile[(i+1)%rows][j]->portN](sigs[i][j].sv_sig_toS);
						(ptr_b nwtile[(i+1)%rows][j])->stress_value_out[nwtile[(i+1)%rows][j]->portN](sigs[i][j].sv_sig_fromS);
                        
                        (ptr_b nwtile[(i+1)%rows][j])->congestion_flag_in[nwtile[(i+1)%rows][j]->portN](sigs[i][j].cf_sig_toS);
						(ptr_b nwtile[(i+1)%rows][j])->congestion_flag_out[nwtile[(i+1)%rows][j]->portN](sigs[i][j].cf_sig_fromS);
                        
                        for(UI k = 0; k < NUM_VCS; k++) {
							(ptr_b nwtile[(i+1)%rows][j])->congestion_status_in[nwtile[(i+1)%rows][j]->portN][k](sigs[i][j].cs_sig_toS[k]);
							(ptr_b nwtile[(i+1)%rows][j])->congestion_status_out[nwtile[(i+1)%rows][j]->portN][k](sigs[i][j].cs_sig_fromS[k]);
						}
					}
					else {	// South neigbor is generic tile
						(ptr nwtile[(i+1)%rows][j])->ip_port[nwtile[(i+1)%rows][j]->portN](sigs[i][j].sig_toS);
						(ptr nwtile[(i+1)%rows][j])->op_port[nwtile[(i+1)%rows][j]->portN](sigs[i][j].sig_fromS);
                        
						for(UI k = 0; k < NUM_VCS; k++) {
							(ptr nwtile[(i+1)%rows][j])->credit_in[nwtile[(i+1)%rows][j]->portN][k](sigs[i][j].cr_sig_toS[k]);
							(ptr nwtile[(i+1)%rows][j])->credit_out[nwtile[(i+1)%rows][j]->portN][k](sigs[i][j].cr_sig_fromS[k]);
						}
                        
                        (ptr nwtile[(i+1)%rows][j])->stress_value_in[nwtile[(i+1)%rows][j]->portN](sigs[i][j].sv_sig_toS);
						(ptr nwtile[(i+1)%rows][j])->stress_value_out[nwtile[(i+1)%rows][j]->portN](sigs[i][j].sv_sig_fromS);
                        
                        (ptr nwtile[(i+1)%rows][j])->congestion_flag_in[nwtile[(i+1)%rows][j]->portN](sigs[i][j].cf_sig_toS);
						(ptr nwtile[(i+1)%rows][j])->congestion_flag_out[nwtile[(i+1)%rows][j]->portN](sigs[i][j].cf_sig_fromS);
                        
                        for(UI k = 0; k < NUM_VCS; k++) {
							(ptr nwtile[(i+1)%rows][j])->congestion_status_in[nwtile[(i+1)%rows][j]->portN][k](sigs[i][j].cs_sig_toS[k]);
							(ptr nwtile[(i+1)%rows][j])->congestion_status_out[nwtile[(i+1)%rows][j]->portN][k](sigs[i][j].cs_sig_fromS[k]);
						}
					}
				}
	
				if(!borderE(nwtile[i][j]->tileID)) {	// connect to East neighbor only if current tile is not on East border
	
					// connect data, credit, stress, congestion flags and congestion status line from East neighbor
					if(corner(id)) {	// corner tile
						(ptr_c nwtile[i][j])->ip_port[nwtile[i][j]->portE](sigs[i][j].sig_fromE);
						(ptr_c nwtile[i][j])->op_port[nwtile[i][j]->portE](sigs[i][j].sig_toE);
		
						for(UI k = 0; k < NUM_VCS; k++) {
							(ptr_c nwtile[i][j])->credit_in[nwtile[i][j]->portE][k](sigs[i][j].cr_sig_fromE[k]);
							(ptr_c nwtile[i][j])->credit_out[nwtile[i][j]->portE][k](sigs[i][j].cr_sig_toE[k]);
						}
                        
                        (ptr_c nwtile[i][j])->stress_value_in[nwtile[i][j]->portE](sigs[i][j].sv_sig_fromE);
						(ptr_c nwtile[i][j])->stress_value_out[nwtile[i][j]->portE](sigs[i][j].sv_sig_toE);
                        
                        (ptr_c nwtile[i][j])->congestion_flag_in[nwtile[i][j]->portE](sigs[i][j].cf_sig_fromE);
						(ptr_c nwtile[i][j])->congestion_flag_out[nwtile[i][j]->portE](sigs[i][j].cf_sig_toE);
                        
                        for(UI k = 0; k < NUM_VCS; k++) {
							(ptr_c nwtile[i][j])->congestion_status_in[nwtile[i][j]->portE][k](sigs[i][j].cs_sig_fromE[k]);
							(ptr_c nwtile[i][j])->congestion_status_out[nwtile[i][j]->portE][k](sigs[i][j].cs_sig_toE[k]);
						}
					}
					else if(border(id)) {	// border tile
						(ptr_b nwtile[i][j])->ip_port[nwtile[i][j]->portE](sigs[i][j].sig_fromE);
						(ptr_b nwtile[i][j])->op_port[nwtile[i][j]->portE](sigs[i][j].sig_toE);
		
						for(UI k = 0; k < NUM_VCS; k++) {
							(ptr_b nwtile[i][j])->credit_in[nwtile[i][j]->portE][k](sigs[i][j].cr_sig_fromE[k]);
							(ptr_b nwtile[i][j])->credit_out[nwtile[i][j]->portE][k](sigs[i][j].cr_sig_toE[k]);
						}
                        
                        (ptr_b nwtile[i][j])->stress_value_in[nwtile[i][j]->portE](sigs[i][j].sv_sig_fromE);
						(ptr_b nwtile[i][j])->stress_value_out[nwtile[i][j]->portE](sigs[i][j].sv_sig_toE);
                        
                        (ptr_b nwtile[i][j])->congestion_flag_in[nwtile[i][j]->portE](sigs[i][j].cf_sig_fromE);
						(ptr_b nwtile[i][j])->congestion_flag_out[nwtile[i][j]->portE](sigs[i][j].cf_sig_toE);
                        
                        for(UI k = 0; k < NUM_VCS; k++) {
							(ptr_b nwtile[i][j])->congestion_status_in[nwtile[i][j]->portE][k](sigs[i][j].cs_sig_fromE[k]);
							(ptr_b nwtile[i][j])->congestion_status_out[nwtile[i][j]->portE][k](sigs[i][j].cs_sig_toE[k]);
						}
					}
					else {	// generic tile
						(ptr nwtile[i][j])->ip_port[nwtile[i][j]->portE](sigs[i][j].sig_fromE);
						(ptr nwtile[i][j])->op_port[nwtile[i][j]->portE](sigs[i][j].sig_toE);
		
						for(UI k = 0; k < NUM_VCS; k++) {
							(ptr nwtile[i][j])->credit_in[nwtile[i][j]->portE][k](sigs[i][j].cr_sig_fromE[k]);
							(ptr nwtile[i][j])->credit_out[nwtile[i][j]->portE][k](sigs[i][j].cr_sig_toE[k]);
						}
                        
                        (ptr nwtile[i][j])->stress_value_in[nwtile[i][j]->portE](sigs[i][j].sv_sig_fromE);
						(ptr nwtile[i][j])->stress_value_out[nwtile[i][j]->portE](sigs[i][j].sv_sig_toE);
                        
                        (ptr nwtile[i][j])->congestion_flag_in[nwtile[i][j]->portE](sigs[i][j].cf_sig_fromE);
						(ptr nwtile[i][j])->congestion_flag_out[nwtile[i][j]->portE](sigs[i][j].cf_sig_toE);
                        
                        for(UI k = 0; k < NUM_VCS; k++) {
							(ptr nwtile[i][j])->congestion_status_in[nwtile[i][j]->portE][k](sigs[i][j].cs_sig_fromE[k]);
							(ptr nwtile[i][j])->congestion_status_out[nwtile[i][j]->portE][k](sigs[i][j].cs_sig_toE[k]);
						}
					}
	
					// connect data, credit, stress, congestion flags and congestion status line to East neighbor
					if(corner(id_E)) {	// East neighbor is corner tile
						(ptr_c nwtile[i][(j+1)%cols])->op_port[nwtile[i][(j+1)%cols]->portW](sigs[i][j].sig_fromE);
						(ptr_c nwtile[i][(j+1)%cols])->ip_port[nwtile[i][(j+1)%cols]->portW](sigs[i][j].sig_toE);
                        
						for(UI k = 0; k < NUM_VCS; k++) {
							(ptr_c nwtile[i][(j+1)%cols])->credit_out[nwtile[i][(j+1)%cols]->portW][k](sigs[i][j].cr_sig_fromE[k]);
							(ptr_c nwtile[i][(j+1)%cols])->credit_in[nwtile[i][(j+1)%cols]->portW][k](sigs[i][j].cr_sig_toE[k]);
						}
                        
                        (ptr_c nwtile[i][(j+1)%cols])->stress_value_out[nwtile[i][(j+1)%cols]->portW](sigs[i][j].sv_sig_fromE);
						(ptr_c nwtile[i][(j+1)%cols])->stress_value_in[nwtile[i][(j+1)%cols]->portW](sigs[i][j].sv_sig_toE);
                        
                        (ptr_c nwtile[i][(j+1)%cols])->congestion_flag_out[nwtile[i][(j+1)%cols]->portW](sigs[i][j].cf_sig_fromE);
						(ptr_c nwtile[i][(j+1)%cols])->congestion_flag_in[nwtile[i][(j+1)%cols]->portW](sigs[i][j].cf_sig_toE);
                        
                        for(UI k = 0; k < NUM_VCS; k++) {
							(ptr_c nwtile[i][(j+1)%cols])->congestion_status_out[nwtile[i][(j+1)%cols]->portW][k](sigs[i][j].cs_sig_fromE[k]);
							(ptr_c nwtile[i][(j+1)%cols])->congestion_status_in[nwtile[i][(j+1)%cols]->portW][k](sigs[i][j].cs_sig_toE[k]);
						}
					}
					else if(border(id_E)) {	// East neighbor is border tile
						(ptr_b nwtile[i][(j+1)%cols])->op_port[nwtile[i][(j+1)%cols]->portW](sigs[i][j].sig_fromE);
						(ptr_b nwtile[i][(j+1)%cols])->ip_port[nwtile[i][(j+1)%cols]->portW](sigs[i][j].sig_toE);
                        
						for(UI k = 0; k < NUM_VCS; k++) {
							(ptr_b nwtile[i][(j+1)%cols])->credit_out[nwtile[i][(j+1)%cols]->portW][k](sigs[i][j].cr_sig_fromE[k]);
							(ptr_b nwtile[i][(j+1)%cols])->credit_in[nwtile[i][(j+1)%cols]->portW][k](sigs[i][j].cr_sig_toE[k]);
						}
                        
                        (ptr_b nwtile[i][(j+1)%cols])->stress_value_out[nwtile[i][(j+1)%cols]->portW](sigs[i][j].sv_sig_fromE);
						(ptr_b nwtile[i][(j+1)%cols])->stress_value_in[nwtile[i][(j+1)%cols]->portW](sigs[i][j].sv_sig_toE);
                        
                        (ptr_b nwtile[i][(j+1)%cols])->congestion_flag_out[nwtile[i][(j+1)%cols]->portW](sigs[i][j].cf_sig_fromE);
						(ptr_b nwtile[i][(j+1)%cols])->congestion_flag_in[nwtile[i][(j+1)%cols]->portW](sigs[i][j].cf_sig_toE);
                        
                        for(UI k = 0; k < NUM_VCS; k++) {
							(ptr_b nwtile[i][(j+1)%cols])->congestion_status_out[nwtile[i][(j+1)%cols]->portW][k](sigs[i][j].cs_sig_fromE[k]);
							(ptr_b nwtile[i][(j+1)%cols])->congestion_status_in[nwtile[i][(j+1)%cols]->portW][k](sigs[i][j].cs_sig_toE[k]);
						}
					}
					else {	// East neighbor is generic tile
						(ptr nwtile[i][(j+1)%cols])->op_port[nwtile[i][(j+1)%cols]->portW](sigs[i][j].sig_fromE);
						(ptr nwtile[i][(j+1)%cols])->ip_port[nwtile[i][(j+1)%cols]->portW](sigs[i][j].sig_toE);
                        
						for(UI k = 0; k < NUM_VCS; k++) {
							(ptr nwtile[i][(j+1)%cols])->credit_out[nwtile[i][(j+1)%cols]->portW][k](sigs[i][j].cr_sig_fromE[k]);
							(ptr nwtile[i][(j+1)%cols])->credit_in[nwtile[i][(j+1)%cols]->portW][k](sigs[i][j].cr_sig_toE[k]);
						}
                        
                        (ptr nwtile[i][(j+1)%cols])->stress_value_out[nwtile[i][(j+1)%cols]->portW](sigs[i][j].sv_sig_fromE);
						(ptr nwtile[i][(j+1)%cols])->stress_value_in[nwtile[i][(j+1)%cols]->portW](sigs[i][j].sv_sig_toE);
                        
                        (ptr nwtile[i][(j+1)%cols])->congestion_flag_out[nwtile[i][(j+1)%cols]->portW](sigs[i][j].cf_sig_fromE);
						(ptr nwtile[i][(j+1)%cols])->congestion_flag_in[nwtile[i][(j+1)%cols]->portW](sigs[i][j].cf_sig_toE);
                        
                        for(UI k = 0; k < NUM_VCS; k++) {
							(ptr nwtile[i][(j+1)%cols])->congestion_status_out[nwtile[i][(j+1)%cols]->portW][k](sigs[i][j].cs_sig_fromE[k]);
							(ptr nwtile[i][(j+1)%cols])->congestion_status_in[nwtile[i][(j+1)%cols]->portW][k](sigs[i][j].cs_sig_toE[k]);
						}
					}
				}
				break;
			}
		}
	}

	SC_THREAD(entry);	// Thread entry() sensitive to clock
	sensitive << switch_cntrl.pos();
}

///////////////////////////////////////////////////////////
/// This thread keeps track of global simulation count.
/// It also closes logfiles upon completion of simulation.
////////////////////////////////////////////////////////////
void NoC::entry() {
	while(true) {
		sim_count = 0;
		while(sim_count < SIM_NUM) {
			wait();
			sim_count++;
            progress_bar_draw((double)SIM_NUM, (double)sim_count, 40);
            
            if (sim_count == 1) {  // modeling misfunctional
               // set_router_fail(9);
                set_router_fail(10);
                set_router_fail(11);
            }
		}
        
		for(UI i=0; i < rows; i++) {
			for(UI j=0; j < cols; j++) {
				switch(TOPO) {
				
				case TORUS:
					(ptr nwtile[i][j])->closeLogs();
					break;

				case MESH:
					if(corner(j + i*cols))
						(ptr_c nwtile[i][j])->closeLogs();
					else if(border(j + i*cols))
						(ptr_b nwtile[i][j])->closeLogs();
					else (ptr nwtile[i][j])->closeLogs();
					break;
				}
			}
		}
        
        printf("\r\n");
		sc_stop();
	}//end while
}//end entry

//////////////////////////////////////////
/// Method to turn off router
/// \param tileID ID of tile witch to turn off
/// \return result
/////////////////////////////////////////
bool NoC::set_router_fail(UI tileID) {
    set_router_fail_dir(tileID, N, true, true);
    set_router_fail_dir(tileID, E, true, true);
    set_router_fail_dir(tileID, W, true, true);
    set_router_fail_dir(tileID, S, true, true);
    set_router_fail_dir(tileID, C, true, true);
}

///////////////////////////////////////////////////////////////////
/// changes router's output channel state to fail or working
/// \param tileID  ID of tile witch channel is to turn off
/// \param dir output channel direction
/// \param fail new state (true - fail, false - working)
/// \param ack_like_real behave like used DyBM algo (turn off all tile if necessary)
/// \return result
///////////////////////////////////////////////////////////////////
bool NoC::set_router_fail_dir(UI tileID, UI dir, bool fail, bool ack_like_real) {
    if (tileID >= num_tiles)
        return false;
        
    if (dir > C)
        return false;
      
    int cur_xco = tileID / num_cols;
	int cur_yco = tileID % num_cols;
    
    if ((ptr nwtile[cur_xco][cur_yco])->get_router_fail_dir(dir) == fail)
        return true;
  
    (ptr nwtile[cur_xco][cur_yco])->set_router_fail_dir(dir, fail);
    
    if (!ack_like_real)
        return true;
    
    // ack like real now!
    if (dir == C) {
        return turn_off_ipcore(tileID);
    }
    
    bool res = true;
    switch (dir) {
        case N: case S: {
                    if ((ptr nwtile[cur_xco][cur_yco])->get_router_fail_dir(E) || (ptr nwtile[cur_xco][cur_yco])->get_router_fail_dir(W))
                        res &= turn_off_ipcore_and_links(tileID);
        }; break;
        case E: case W: {
                    if ((ptr nwtile[cur_xco][cur_yco])->get_router_fail_dir(N) || (ptr nwtile[cur_xco][cur_yco])->get_router_fail_dir(S))
                        res &= turn_off_ipcore_and_links(tileID);
        }; break;
        default: break;
    }
    
   /* for (UI i = 0; i < num_rows; i++) 
        for (UI j = 0; j < num_cols; j++) {
            UI tmpID = i * num_cols + j;
            if ((((ptr nwtile[i][j])->get_router_fail_dir(N) || (ptr nwtile[i][j])->get_router_fail_dir(S)) &&
                 ((ptr nwtile[i][j])->get_router_fail_dir(E) || (ptr nwtile[i][j])->get_router_fail_dir(W))) && 
               !corner(tmpID))
                   turn_off_ipcore_and_links(tmpID);
        }*/
    
    return res;
}

//////////////////////////////////////////
/// Method to turn off ipcore 
/// \param tileID ID of tile witch ip core turn off
/// \return result
/////////////////////////////////////////
bool NoC::turn_off_ipcore(UI tileID) {
    if (tileID >= num_tiles)
        return false;
        
    int cur_xco = tileID / num_cols;
	int cur_yco = tileID % num_cols;
    
    for(UI i=0; i < num_rows; i++) {
        for(UI j=0; j < num_cols; j++) {
            if (!((i == cur_xco) && (j == cur_yco)))
                (ptr nwtile[i][j])->set_creating_flits_state(tileID, false);
        }
    }
    (ptr nwtile[cur_xco][cur_yco])->set_creating_flits_state(num_tiles, false);
    
    return true;
}

//////////////////////////////////////////
/// Method to turn off certain overall tile and channels to it
/// \param tileID ID of tile witch to turn off
/// \return result
/////////////////////////////////////////
bool NoC::turn_off_ipcore_and_links(UI tileID) {
    int cur_xco = tileID / num_cols;
	int cur_yco = tileID % num_cols;
    
    if (tileID >= num_tiles)
        return false;
        
    if (!turn_off_ipcore(tileID))
        return false;   
        
    if ((ptr nwtile[cur_xco][cur_yco])->is_router_shutdown())
        return true;
        
    (ptr nwtile[cur_xco][cur_yco])->set_router_fail_dir(N, true);
    (ptr nwtile[cur_xco][cur_yco])->set_router_fail_dir(E, true);
    (ptr nwtile[cur_xco][cur_yco])->set_router_fail_dir(W, true);
    (ptr nwtile[cur_xco][cur_yco])->set_router_fail_dir(S, true);
    
    if (!borderW(tileID)) {
        (ptr nwtile[cur_xco][cur_yco - 1])->set_router_fail_dir(E, true);
        if (is_turn_off_tile(cur_xco, cur_yco - 1))
            turn_off_ipcore_and_links(cur_xco * num_cols + cur_yco - 1);
    }
    if (!borderE(tileID)) {
        (ptr nwtile[cur_xco][cur_yco + 1])->set_router_fail_dir(W, true);
        if (is_turn_off_tile(cur_xco, cur_yco + 1))
            turn_off_ipcore_and_links(cur_xco * num_cols + cur_yco + 1);
    }
    if (!borderS(tileID)) {
        (ptr nwtile[cur_xco + 1][cur_yco])->set_router_fail_dir(N, true);
        if (is_turn_off_tile(cur_xco + 1, cur_yco))
            turn_off_ipcore_and_links((cur_xco + 1) * num_cols + cur_yco);
    }
    if (!borderN(tileID)) {
        (ptr nwtile[cur_xco - 1][cur_yco])->set_router_fail_dir(S, true);
        if (is_turn_off_tile(cur_xco - 1, cur_yco))
            turn_off_ipcore_and_links((cur_xco - 1) * num_cols + cur_yco);
    }
         
 //   if (LOG >= 1) {
        eventlog<<"Turn off tile: "<<tileID;
        eventlog<<" N: "<<(ptr nwtile[cur_xco][cur_yco])->get_router_fail_dir(N);
        eventlog<<" E: "<<(ptr nwtile[cur_xco][cur_yco])->get_router_fail_dir(E);
        eventlog<<" W: "<<(ptr nwtile[cur_xco][cur_yco])->get_router_fail_dir(W);
        eventlog<<" S: "<<(ptr nwtile[cur_xco][cur_yco])->get_router_fail_dir(S)<<endl;
 //   }
        
    return true;
}

//////////////////////////////////////////
/// Method to determine is need to turn off tile (x,y)
/// \param x x position
/// \param y y position
/// \return result
/////////////////////////////////////////
bool NoC::is_turn_off_tile(UI x, UI y) {
    UI tmpID = x * num_cols + y;
    UI tmpNum = 0;
    
    if ((ptr nwtile[x][y])->is_router_shutdown())
        return false;
    
    if (!corner(tmpID) && !border(tmpID)) {
        if (((ptr nwtile[x][y])->get_router_fail_dir(N) || (ptr nwtile[x][y])->get_router_fail_dir(S)) &&
            ((ptr nwtile[x][y])->get_router_fail_dir(E) || (ptr nwtile[x][y])->get_router_fail_dir(W)))
                return true;
    }
    else {
        if ((ptr nwtile[x][y])->get_router_fail_dir(N))
            tmpNum++;
            
        if ((ptr nwtile[x][y])->get_router_fail_dir(S))
            tmpNum++;
            
        if ((ptr nwtile[x][y])->get_router_fail_dir(E))
            tmpNum++;
            
        if ((ptr nwtile[x][y])->get_router_fail_dir(W))
            tmpNum++;
            
        if (tmpNum > 2)
            return true;
            
    }
}

///////////////////////////////////////////////////////////////////
/// function that draw progress bar
/// \param total_to_count total count of something
/// \param now_count current count of something
/// \param total_dotz total count of dots to be displayed
/// 
/// Based on parameters function draws fancy progress meter
///////////////////////////////////////////////////////////////////
void NoC::progress_bar_draw(double total_to_count, double now_count, UI total_dotz) {
    time_t rawtime;
    struct tm * timeinfo;
    char buffer [20];

    time( &rawtime );
    timeinfo = localtime( &rawtime );
    strftime(buffer, 20, "%X %x", timeinfo);  // current date and time

    double fraction_ready = now_count / total_to_count;
    
    // part of the progress bar that's already "full"
    UI dotz = round(fraction_ready * total_dotz);

    // create the "bar"
    UI ii=0;
    printf("%20s %3.0f%% [", buffer, (fraction_ready * 100) );
    
    // part that's "full" already
    for ( ; ii < dotz; ii++) {
        printf("=");
    }
    // remaining part (spaces)
    for ( ; ii < total_dotz; ii++) {
        printf(" ");
    }
    // and back to line begin and flush
    printf("]\r");
    
    fflush(stdout);
}

