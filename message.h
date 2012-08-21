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
#ifndef MESSAGE_H
#define MESSAGE_H
// Standard Template Library example using a class.

#include <iostream>
#include "traffic.h"
#include <list>

using namespace std;

// The List STL template requires overloading operators =, == and <.

class message
{

friend ostream &operator<<(ostream &, const message &);

public:
	int id;	//the id of the message
	int sid;	//sender id
	int rid;	//receiver id
	int gen_t;	//the time message has been created
	//traffic* tr;	//a pointer to traffic class wich has the receiver id as well and other information such as the creation time for this message and the sender id
	int no_hops;	//the nof hops which message has travered so far
	int cost;	//no of times this message has been transmitted
	int mcp;	//no of copies for mcp algorithm
	int hopttl;	//hopcount ttl for mcp
	bool gRank;	//Global Ranking flag for Bubble routing
	double maxSim;	//maximum similarity so far: GreedyB
	int maxRank;	//maximum rank which this message has been passed to!
	bool delivered;	//a flag which shows if a message has been delivered to its receiver or not, its default = false
	list<int> relay_list;	//each message keeps a list to store the relay nodes
	list<int> time_list;	//the time every relay happens
	message();
//	message(const message &);
	message(const int &, const traffic &);
	~message(){};
	message &operator=(const message &msg);
	int operator==(const message &msg) const;
	double timestamp;	//used for spreading info
private:
};
#endif
