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

#include <iostream>
#include "traffic.h"

using namespace std;

// The List STL template requires overloading operators =, == and <.


traffic::traffic()   // Constructor
{
	sndr_id = -1;	
	rcvr_id = -1;	
	gen_t = -1;	
	success = -1;	
	hops = -1;	
	cost = 0;	
	delay = -1;
	rcv_t = -1;
	id = 0;
}

traffic::traffic(const int &sid, const int &rid, const int &gen){
	sndr_id = sid;	
	rcvr_id = rid;	
	gen_t = gen;	
	success = -1;	//not successful initially	
	hops = 0;	
	cost = 0;
	delay = 0;
	rcv_t = 0;
	id = 0;	
}
ostream &operator<<(ostream &output, const traffic &trf)
{
   output << trf.sndr_id << trf.rcvr_id << trf.hops << trf.cost << endl;
   return output;
}


int traffic::operator==(const traffic &trf) const
{
   if( this->sndr_id != trf.sndr_id) return 0;
   if( this->rcvr_id != trf.rcvr_id) return 0;
   if( this->gen_t != trf.gen_t) return 0;

   return 1;
}
/*
int traffic::delivered(const traffic *trf, const int &snd, const int &rcv, const int &gen, conts int &d){
	
}
*/
