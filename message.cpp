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
// Standard Template Library example using a class.

#include <iostream>
#include "message.h"

using namespace std;

// The List STL template requires overloading operators =, == and <.


message::message()   // Constructor
{
   id = 0;
   sid = 0;	//sender id
   rid = 0;	//receiver id
   gen_t = 0;	
   no_hops = 0;
   cost = 0;
   mcp = 0;
   hopttl = 0;
   maxSim = 0;
   maxRank = 0;	//we had a bug in our code because this value was not initilized to zero!
   timestamp = 0.0;
}

ostream &operator<<(ostream &output, const message &msg)
{
   output << msg.id << endl;
   return output;
}


int message::operator==(const message &msg) const
{
   if( this->id != msg.id) return 0;
   if( this->sid != msg.sid) return 0;
   if( this->rid != msg.rid) return 0;
   if( this->gen_t != msg.gen_t) return 0;

   return 1;
}

message& message::operator=(const message &msg)
{
   this->id = msg.id;
   this->sid = msg.sid;
   this->rid = msg.rid;
   this->gen_t = msg.gen_t;
   return *this;
}
