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
#ifndef DISTR_H
#define DISTR_H

#include <iostream>
#include <fstream>
#include <list>
#include "configuration.h"
#include "contactG.h"

using std::ifstream; // input file stream
using namespace std;

class distr
{
public:
	distr(configuration *);	//CONSTRUCTOR
	configuration *config;	//an configuration object
	int contactdist();	//contact distribution
	int intercontactdist();	//inter-contact distribution
	int intercontactdist(char *, char *);
	int intercontactdist(char *);
	int no_of_contacts(char *, char *);	//no of contacts between two nodes
	int no_of_contacts(char *, char *, int, int, int);	//no of contacts between two nodes during a time interval
	int no_of_contacts(char *, int, int, int);	//we can use this function to find the contact rate of a given node
	int real_no_of_contacts(char *, char *, int, int);
	int contact_duration(char *, char *);
	int contact_duration(char *, char *, int, int);
	double closeness(char *, char *, unsigned char);
	double socialdist(char *, char *, unsigned char);
	double sim_watts(char *, char *, unsigned char);
	int Ranks(int, int, int, int);	//a function which calculates and updates the rank of nodes (centralities) duting time
	int node_sequence(int, int, int, int, int);	//finding the sequence of nodes quality which a specific node meets during the conference
	int total_degrees(int, int);	//this function calculates the total degree in an interval
	int locations(int, int);	//a loaclization function
	list<int> ranks_t[100];	//what are the ranks of a node: how many unique nodes the current node during the time
//	list<double> time_r[100];	//time sequence for ranks
	list<int> locations_t[100];	//what are the locations of nodes: what static nodes they have met
	list<int> times[100];	//time sequence for locations
	contactG cg;		//for calculating Ranks
	map<int, int> contact;	//a contact between two users: (no_cont,cont_dur)
	map<int, int> extcont;	//a map for external nodes which contains their id as well as no of contacts
	int extcount(int, int);
	int visited_locs(int, int);	//location where visited by a user over a period of time
};
#endif
