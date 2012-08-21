/***************************************************************************
 *   Copyright (C) 2011 by Kazem   *
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

#ifndef ML_H
#define ML_H

#include <list>

#include <iostream>
#include <mysql++.h>
#include <cstdlib>
#include <fstream> // file stream        
#include "contactG.h"
#include <list>
#include <math.h>
#include <time.h>
#include <map>
#include "feature.h"

using std::ifstream; // input file stream

using namespace std;

class ml
{
public:
	ml(){
	};
	~ml(){
		//clear all data structures
		Nodes.clear();
		features.clear();
		degrees.clear();
		ncn.clear();
	};
	int extract_features(contactG &, int, char *);	//a method for extracting features
	int generate_M(contactG &, int, char *);		//generate the adjacency matrix for contact graph
	int generate_partial_M(contactG &, int, char *);	//generate the partial adjacency matrix
	int write_M(int, char *);									//this function writes the matrix M into a file
private:
	list<int> Nodes;
	int nodes_num;							//no of nodes in cg
	map<int,int> degrees;					// a map from node id to their degrees
	map<pair<int,int>,int> ncn;				// a map from node ids to their ncn's
	list<feature> features;
	map< pair<int,int>, int > M;			// a map from nodes pair to edge existence
	map<int, int> id_map;					//a map for id --> matrix index
	int extract_deg_features(contactG &);	//extract degree based features
	int extract_ncn_features(contactG &);	//computes the ncn among all possible pairs
	int store_all_features(contactG &);		//store all features into a list
	int print_feature_vectors();			//print all features
	int export_features(int, char *);		//export the feature matrix and output vector to a file
};
#endif
