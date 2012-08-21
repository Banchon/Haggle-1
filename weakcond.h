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
#ifndef WEAKCOND_H
#define WEAKCOND_H

#include <list>
#include <iostream>
#include <mysql++.h>
#include <cstdlib>
#include "distr.h"
#include "rgmodel.h"
#include <fstream> // file stream        
#include "contactG.h"
#include "message.h"
#include "traffic.h"
#include "Node.h"
#include <math.h>
#include "configuration.h"
#include "multiple.h"
#include "triple.h"
#include "quadruple.h"

using std::ifstream; // input file stream

using namespace std;

class weakcond
{
public:
	weakcond(double ts, double te, double rate_val, double t_wup_val, configuration *cfg){
		tStart = ts;
		tEnd = te;
		config = cfg;
		nodeNum = config->node;
		rate = rate_val;
		SS_flag = 0;
		t_wup = t_wup_val;
		clock = 0.0;
		totalCont = 0;
		utilCont = 0;
		//we have to generate a list of N nodes
		Node node;
		for(int i = 0; i < nodeNum; i++){	
			node.id = config->s_nid + i;	//let's set the node id
			nodes.push_back(node);		//ass the node to the system
		}
		 log_file.open ("/home/kazem/Desktop/traces/mobility/spread.dat",ios::trunc);	//open the log file
	}
	// Destructor
  	~weakcond() { log_file.close(); }
	int nodeNum;	//number of nodes in the system
	double clock;	//current time
	double tStart;	//starting time for simulation
	double tEnd;	//end of simulation time
	configuration *config;	//an configuration object
	list<Node> nodes;	//list of system nodes
	double rate;		//rate with which service provider generates the contents
	int spreadNews(int, bool, double);	//spread the information scheme
	int setupDB();		//setup the DB for reading the contacts according to their start time
	mysqlpp::StoreQueryResult queryRes;	//the result of our query to mysql db is goint to be cached here
	double t_wup;	//warm up value for calculating the MEA, we are assuming by this time the system is in steady state
	int SS_flag;	//this is a flag which indicates if are in steady state or not. If so, we can start computing the maximum expected age or not
	int init_MEA();		//initialize all required data structures for computing MEA
	int compute_EA(int, double);	//update the expected age for node id
	int compute_MEA();	//at the end of simulation, this method computes the maximum expected age for the whole system
	double Yprev[90];	//storing the last age for the corresponding node
	double Tprev[90];	//storing the last update time which corresponds to Yprev[.]
	double EA[90];	//storing expected age for each node
	double MEA;	//Maximum Expected Age
	int totalCont;	//total number of contacts
	int utilCont;	//no of utilized contacts
	bool update_Lists(list<Node>::iterator, list<Node>::iterator);
	int spreadMsgs(int);	//this simulates the process of spreading all messages in the whole system
	int has_received(int, int, int, int);	//check if a node has received all messages
	int set_watched_ids(int);	//set the list of nodes that we want to watch their buffers
	int watched_id;		//the watched id
	ofstream log_file;		//log file
private:
	int print_Lists(int);
	int setupMsgs(double);	//setup all nodes' buffers
};
#endif
