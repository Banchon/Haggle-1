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
#ifndef NODE_H
#define NODE_H

#include <list>

#include <iostream>
#include <mysql++.h>
#include <cstdlib>
#include "parser.h"
#include "distr.h"
#include "rgmodel.h"
#include "plot.h"
#include <fstream> // file stream        
#include "statistics.h"
#include "emulation.h"
#include "contactG.h"
#include <list>
#include "message.h"
#include "traffic.h"
#include <math.h>

using std::ifstream; // input file stream

using namespace std;

class Node
{
public:
	Node(){
		metNodes = 0;
		maxRank = 0;
		maxSeq = 0;
		threshold = 29;
		candN = -1;
		seqNo = 0;
		randSeq = 0;
		contact_no = 0;
	};
	~Node(){};
	int id;	//node id
	list<message> buffer_list;	//let's generate a buffer array for nodes: 21-99: 79 nodes, every nodes keep all of its message in its buffer
	int maxRank;	//used by RankSec, what is the maximum ranked node which has been met upto stopping time by the current node!
	int maxSeq;	//used by RankSec, what is the maximum ranked node which has been met by the current node!
	int metNodes;	//how many node the current node should contact and then start fprwarding its messages
	int candN;	//what quality has been chosen by RankSec as the candidate node for passing the message
	int seqNo;	//we need to keep track of the sequence fo all met node for RandomSec scheme
	int randSeq;	//the seq no for each node: RandomSeq passes the message to the node with sequence of randSeq
	int threshold;	//the no of nodes which we have to met to start routing process
	map<int, int> nodesMet;	//every node has to keep the pair of nodes quality (or node id) and no (practically 1) which has been met so far : RankSec
	int contact_no;	//the total contant no of current node which is used by Delegation
	map<int,int> Forego;	//a map of nodes that we are going to forego their contacts in the future
	map<int,int> nForego;	//a map of nodes that we dont skip their contacts in the future
	map<int,int> Ngh;	//a map of nodes that we have met directly
private:
};
#endif
