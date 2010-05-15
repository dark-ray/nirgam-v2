
/*
 * fifo.cpp
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
/// \file fifo.cpp
/// \brief Implements first-in-first-out buffer
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#include "flit.h"
#include "fifo.h"

////////////////////////////////////////////////////////
/// Method to insert a flit in fifo
/// \param data_flit input flit
////////////////////////////////////////////////////////
void fifo::flit_in(const flit& data_flit) {
	regs[pntr++] = data_flit;	        // insert flit in register array
  	empty = false;	                    // set empty status to false
  	if(pntr == num_bufs) full = true;	// if fifo is full, set full status to true
	if(LOG >= 4)
		eventlog<<"\ntime: "<<sc_time_stamp()<<" Inserting flit, pntr: "<<pntr<<" num_bufs: "<<num_bufs<<" full: "<<full;
}

///////////////////////////////////////////////////////
/// Method to read a flit from fifo
/// \return readed flit
///
/// - Remove flit from front of queue and return it
///////////////////////////////////////////////////////
flit fifo::flit_out() {
	flit temp;
	temp = regs[0];		// read flit in front of queue
  	if(--pntr==0) empty = true;
  	else
		for(UI i = 0; i < pntr; i++)
			regs[i] = regs[i+1];
	if(pntr!=num_bufs) full = false;
    if(LOG >= 4)
		eventlog<<"\ntime: "<<sc_time_stamp()<<" Removing flit, pntr: "<<pntr<<"  ";
    if(LOG >= 6)
        eventlog<<temp;
	return(temp);
}

///////////////////////////////////////////////////////
/// Method to read a flit from fifo without removing it
/// \param silent don't log this operation
/// \return readed flit
///////////////////////////////////////////////////////
flit fifo::flit_read(bool silent) {
	flit temp;
	temp = regs[0];		        // read flit in front of queue
	if((LOG >= 4) && !silent)
		eventlog<<"\ntime: "<<sc_time_stamp()<<" Reading flit, pntr: "<<pntr<<"  ";
  	if((LOG >= 6) && !silent)
        eventlog<<temp;
    return(temp);
}

///////////////////////////////////////////////////////////////////////////
/// Method to push back a flit in its original position in fifo
/// \param pack input flit
///////////////////////////////////////////////////////////////////////////
void fifo::flit_push(flit pack) {
	pntr++;
  	empty = false;
  	if(pntr == num_bufs) full = true;
	for(UI i = pntr; i > 0; i--)
		regs[i] = regs[i-1];
	regs[0] = pack;
	if(LOG >= 4)
		eventlog<<"\ntime: "<<sc_time_stamp()<<" Pushing back flit, pntr: "<<pntr;
}

//////////////////////////////////////////////////////////////////////////
/// Method to debug read flit from fifo such as that normal array
/// \param ind index of flit in fifo
/// \return readed flit
///
/// Flit will not be removed from fifo
//////////////////////////////////////////////////////////////////////////
flit fifo::flit_debug_read(UI ind) {
    if (ind > pntr)
        return regs[0];
    
    return regs[ind];
}

///////////////////////////////////////////////////////////////////////////
/// Method to increment number of waited clocks in any flits at fifo
///////////////////////////////////////////////////////////////////////////
void fifo::inc_flits_num_waits() {
	for (UI i = 0; i < pntr; i++ )
		regs[i].simdata.num_waits++;
}
