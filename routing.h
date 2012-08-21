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
#ifndef ROUTING_H
#define ROUTING_H

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
#include "Node.h"
#include <math.h>
#include "configuration.h"
#include "multiple.h"
#include "triple.h"

using std::ifstream; // input file stream

using namespace std;

// typedef struct Tri{
// int src;
// int dst;
// int time;
// }Tri;


class routing
{
public:
	routing(){};
	routing(int, int, configuration *);	//no_msgs,TTL, ts
	~routing(){};
	configuration *config;	//an configuration object
	list<traffic> traf_list[100];	//let's generate a traffic profile array for nodes: 21-99: 79 nodes --> we define the size of array large enough
	map<int,int> curr_cost;	//we want to track the cost for each message so far
 	int no_msgs;
// 	int node;	//no of nodes in the network. node id can be anything between 21 and 99
 	int TTL;	// what is the total TTL for all traffic in seconds : 9 hours!
 	int ts;
	int te;	//setting starting time and ending time for the emulation: 9:00AM on 24th --> 57600 sec <=
	int hopttl;	//the initial time-to-live hop count limit: for MCP forwarding scheme
	int mcp;	//no of copies upper bound: for MCP forwarding scheme
	int relayed_msgs;	//this is a counter for counting the number of messages which have been delivered by relay nodes, so it reflects how effective was the routing!
	int non_addressed[15];
	int Waiting(int);		//Waiting scheme
	int Epidemic(int);		//Epidemic scheme
	int GreedyA(int, double);		//Social-Greedy I scheme
	int GreedyB(int, int, int, int, double);		//Social-Greedy II scheme
	int GreedyC(int, double);		//Social-Greedy III scheme
	int MCP(int, int, int);	//MCP mechanism where we pass the hop ttl as well ass no of copies bound
	int TwoHop(int, int, int, int);	//two hop forwarding algorithm
	int Label(int);		//LABEL scheme
	int Bubble(int, int, int);		//Bubble scheme
	int RankSec(int, int, int, int);//, int, double, int);	//Rank Secretary Problem
	int RandomSec(int, int, int, double);	//the radom version of RankSec
	int Delegation(int, int, int, int);	//Delegatiom Forwarding 
	int nDelegation(int, int, int, int, double);	//New Delegatiom Forwarding
	int GreedyD(int, int, int, int, double, int);	//Social-Greedy III scheme
	int Random(int, double);			//Random routing
	int Spread(int);	//spreading info in a MSN
	int timeout(int, int, int, ofstream&);	//check the tables of two users and removes all of 
	int timeout(int, int, ofstream&);
//	list<message> buffer_list[79];	//let's generate a buffer array for nodes: 21-99: 79 nodes, every nodes keep all of its message in its buffer
	Node nodes[100];		//we have 79 nodes --> we define the size of array large enough!
	double closeness[100][100];	// for greedy scheme --> we define the size of array large enough!!
	double contP[100][100];	//contact probabilities
	contactG cg;		//for labels: LABEL scheme
	int fill_labels();	//fill labels for cg for LABEL scheme 
	int find_social_dist(unsigned char);	//this methods fill out closeness table to be used for greedy routing!
	int find_focus_dist(unsigned char, int);	//foci based social distance
	int find_cont_prob(unsigned char, double);	//finds contact probabilities
	int random_social_dist();	//method which fills distances between node uniformly at random
	int print_traffic();	//prints the traffic profile
	int print_buffers();	//print the nodes buffer contents
	int write_profile(ofstream&, ofstream &, int, int);	//a function which write the delay and cost distribution for all traffic, the last two argument are simulation number and routing scheme code
	int create_traffic(int, int, int);	//create the traffic profile and stores in nodes buffers
	int cleanup();			//this method cleans up buffer and traffic profile link lists
	int fixbug();	//this function is written to take care of a very stupid bug that we have right now for node[0].buffer_list when the no of messaeg is low (e.g 1) and there is not any message in node 1's buffer however it doesnt say its buffer is empty!! so it tries to read it and then crash!!! --> I couldnt find any explanation for it
	double get_rand()
//	returns random number in range of 0 to 1 
	{
   	  //initialize random seed:
// 	  srand ( time(NULL) );
 	  double x = (double) random()/ (double) RAND_MAX;   // n is a random number in range of 0 - RAND_MAX 
  	 return(x);
	}
	int enumShortPaths(int);
	list <multiple> enumPaths;	//a list for enmerating all paths for (src,dst,time, score) with the path delay
	map <triple, int> mPaths;	//a map for keep tracking of paths
private:
};
#endif


