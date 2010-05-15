
/*
 * flit.h
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
/// \file flit.h
/// \brief Defines flow control unit(flit) for NoC
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#ifndef _FLIT_INC_
#define _FLIT_INC_

#include "systemc.h"
#include "../config/constants.h"
#include "routing_fault_info.h"
#include <sys/stat.h>
#include <sys/types.h>
#include <string>

/// required for stl
using namespace std;

////////////////////////////////////////////////
/// \brief routing header for source routing algorithm
////////////////////////////////////////////////
struct source_hdr {
	UI route;	///< route code
};

//////////////////////////////////////////////////////
/// \brief routing header for destination based routing algorithm.
///
/// It can be used for any other routing algorithms which are a function of only the destination.
//////////////////////////////////////////////////////
struct rt_dst_hdr {
	UI dst;		///< destination tile ID
};

////////////////////////////////////////////////////
/// \brief routing header for Ant based routing algorithm
////////////////////////////////////////////////////
struct AntNet_hdr {
	UI dst;		///< destination tileID
	UI route;	///< route code for reverse path
};

////////////////////////////////////////////////
/// \brief routing header in flit
///
/// One of the available headers, depending on routing algorithm
////////////////////////////////////////////////
union routing_hdr {
	source_hdr sourcehdr;	///< source routing header
	rt_dst_hdr dsthdr;	    ///< destination routing header
	AntNet_hdr AntNethdr;	///< Ant routing header
};

////////////////////////////////////////////////
/// \brief payload in head/hdt flit
///
/// Size: 1 byte
////////////////////////////////////////////////
struct payload_hdr {
	int cmd;			        ///< command
	int data_int;			    ///< integer data
	const char *data_string;	///< string data
};

////////////////////////////////////////////////
/// \brief flit header in head/hdt flit
////////////////////////////////////////////////
struct flit_head {
	routing_type	rtalgo;		///< routing algorithm 
	routing_hdr 	rthdr;		///< routing header
    routing_fault_info rtfi;    ///< routing fault info
	payload_hdr	datahdr;	    ///< payload
};

////////////////////////////////////////////////
/// \brief flit header in data/tail flit
////////////////////////////////////////////////
struct flit_data {
	int cmd;			        ///< command
	int data_int;			    ///< integer data
	const char *data_string;	///< string data
};

///////////////////////////////////////////////
/// \brief flit header, depending on type of flit (hdt, head, data, tail)
//////////////////////////////////////////////
union flit_hdr {
	flit_head header;	///< flit header in hdt/head flit
	flit_data payload;	///< flit header in data/tail flit
};

////////////////////////////////////////////////
/// \brief packet header for ant packets
////////////////////////////////////////////////
struct ant_hdr {
public:
	ant_type 	    anttype;    ///< type of ant
	routing_type 	rtalgo;		///< routing algorithm
	routing_hdr 	rthdr;		///< routing header
};

////////////////////////////////////////////////
/// \brief packet header for noc packets
////////////////////////////////////////////////
struct noc_hdr {
public:
	UI		pktid;		    ///< packet id
	UI		flitid;		    ///< flit id
	UI      hopcount;       ///< hop count passed by flit
	flit_type	flittype;	///< flit type (HDT, HEAD, DATA, TAIL)
	flit_hdr 	flithdr;	///< flit header (depending on flit type)
};

////////////////////////////////////////////////
/// \brief packet header (depending on packet type)
////////////////////////////////////////////////
union pkt_hdr {
	ant_hdr anthdr;
	noc_hdr nochdr;
};

////////////////////////////////////////////////
/// \brief header to record simulation data
////////////////////////////////////////////////
struct sim_hdr {     
	sc_time gtime;		///< flit generation time (in time units)
	sc_time atime;		///< flit arrival time (in time units)
	sc_time ctime;		///< instantaneous time (in time units)
	ULL	gtimestamp;	    ///< flit generation timestamp (in clock cycle)
	ULL	atimestamp;	    ///< flit arrival timestamp at reciever (in clock cycle)
	ULL	ICtimestamp;	///< input channel time stamp (in clock cycles)
	ULL	num_waits;	    ///< number of clock cycles spent waiting in buffer
	ULL	num_sw;		    ///< number of switches traversed
};

////////////////////////////////////////////////
/// \brief flit data structure
///
/// This is the structure that represents flow control unit(flit) in NoC
////////////////////////////////////////////////
struct flit {
	pkt_type pkttype;	///< packet type (ANT, NOC)
	sim_hdr	 simdata;	///< simulation header
	pkt_hdr  pkthdr;	///< packet header (depending on packet type)
	UI	 vcid;		    ///< virtual channel id
	UI	 src;		    ///< source tileID

	/// \brief overloading equality operator
	inline bool operator == (const flit& temp) const { 
		if(temp.pkttype != pkttype || temp.simdata.gtime != simdata.gtime || temp.simdata.atime != simdata.atime || temp.simdata.ctime != simdata.ctime || temp.src != src)
			return false;
		switch(temp.pkttype) {
			case ANT: break;
			case NOC:
				if(temp.pkthdr.nochdr.pktid != pkthdr.nochdr.pktid          || 
                   temp.pkthdr.nochdr.flitid != pkthdr.nochdr.flitid        || 
                   temp.pkthdr.nochdr.flittype != pkthdr.nochdr.flittype    || 
                   temp.pkthdr.nochdr.flithdr.header.rtfi.fail != pkthdr.nochdr.flithdr.header.rtfi.fail)
				return false;
				break;
		}
		return true;
	}
};

/// \brief overloading extraction operator for flit
inline ostream&
operator << ( ostream& os, const flit& temp ) {
	if(temp.pkttype == ANT)
		os<<"ANT PACKET";
	else if(temp.pkttype == NOC) {
		os<<"NOC PACKET, ";
		switch(temp.pkthdr.nochdr.flittype) {
			case HEAD: os<<"HEAD flit, "; break;
			case DATA: os<<"DATA flit, "; break;
			case TAIL: os<<"TAIL flit, "; break;
			case HDT: os<<"HDT flit, "; break;
		}
		os<<"src: "<<temp.src<<" pktid: "<<temp.pkthdr.nochdr.pktid<<" flitid: "<<temp.pkthdr.nochdr.flitid;
		os<<" hopcount: "<<temp.pkthdr.nochdr.hopcount;
	}
    os<<" vcid: "<<temp.vcid<<endl;
	return os;
}

/// \brief overloading extraction operator for simulation header
inline ostream&
operator << ( ostream& os, const sim_hdr& temp ) {
	os<<"gtimestamp: "<<temp.gtimestamp<<" gtime: "<<temp.gtime;
	os<<"\natimestamp: "<<temp.atimestamp<<" atime: "<<temp.atime<<endl;
	return os;
}

/// \brief overloading sc_trace for flit
inline void sc_trace( sc_trace_file*& tf, const flit& a, const std::string& name) {
	//sc_trace( tf, a.pkttype, name+".pkttype");
	sc_trace(tf, a.src, name+".src");
	sc_trace(tf, a.pkthdr.nochdr.pktid, name+".pktid");
	sc_trace(tf, a.pkthdr.nochdr.flitid, name+".flitid");
	sc_trace(tf, a.pkthdr.nochdr.hopcount, name+".hopcount");
	//sc_trace( tf, a.freeBuf, name+".freeBuf");
}

#endif
