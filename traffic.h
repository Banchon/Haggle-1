/***************************************************************************
 *   Copyright (C) 2009 by Kazem   *
 *   root@HERMES   *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU General Public License for more details.                          *
 *                                                                         *
 *   You should have received a copy of the GNU General Public License     *
 *   along with this program; if not, write to the                         *
 *   Free Software Foundation, Inc.,                                       *
 *   59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.             *
 ***************************************************************************/
#ifndef TRAFFIC_H
#define TRAFFIC_H
// Standard Template Library example using a class.

#include <iostream>
using namespace std;

// The List STL template requires overloading operators =, == and <.

class traffic
{

friend ostream &operator<<(ostream &, const traffic &);

public:
	int sndr_id;	//who is the sender of this video
	int rcvr_id;	//the receiver id of this traffic
	int gen_t;	//the generated time of this traffic
	int success;	//the flag that shows this video has been delivered successfully or not: 1:successful and 0:failed
	int hops;	//no of hops it took for this video to reah the destination
	int cost;	//how many transmission we had for this video until we have reached the destination 
	int delay;	//delay for delivery in seconds
	int rcv_t;	//received time
        int nfr;	//what is the maximum number of forwarding for this message for a random scheme
	int id;		//traffic id=message id
	traffic();
	//traffic(const traffic &);
	traffic(const int &, const int &, const int &);	//we only pass the sender and receiver id as well as the time when the traffic has been generated
	~traffic(){};
	traffic &operator=(const traffic &traf);
	int operator==(const traffic &traf) const;
//	int delivered(conts int &, const int &);
private:
};
#endif
