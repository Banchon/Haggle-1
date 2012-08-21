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
#ifndef CONTACTG_H
#define CONTACTG_H
#include <list>
#include "configuration.h"
#include "common.h"
#include "Sim_type.h"
#include "quadruple.h"
#include <boost/graph/adjacency_list.hpp>
#include <boost/graph/adjacency_list.hpp>
#include <boost/graph/kruskal_min_spanning_tree.hpp>
#include <boost/graph/connected_components.hpp>

using namespace std;
using namespace boost;

typedef std::pair<int, int> E;

typedef adjacency_list < vecS, vecS, undirectedS, property < vertex_name_t,
		std::string >, property < edge_name_t, int > > Graph;
typedef property_map < Graph, vertex_name_t >::type node_id_map_t;
//we need a map for edges which stores the no of contacts between id1 and id2 as the edge label
typedef property_map < Graph, edge_name_t >::type cont_no_map_t;
typedef graph_traits < Graph >::vertex_descriptor Vertex;
typedef std::map < std::string, Vertex > IDVertexMap;

class contactG
{
public:
//	contactG(configuration *);	//CONSTRUCTOR
	configuration *config;	//an configuration object
	int label_no;			// no of communities detected from questionarie forms
	list<int> comm_members[35];	//it is the link list for communities detected from questionarie forms, map: communities --> members, we have 35 communities in infocom 2006
	list<int> labels[100];	//what are the labels for a person 
	list<int> ranks[100];	//what are the ranks of a node: how many unique nodes the current node has encountered in the past T hour! 
	list<int> tgAdjTable[100];	//adjacency table of the threshold graph
	map<int, int> mapping;	//the mapping between communities id and their position on the link list
	int findComm();		//fills out the community_membr link list
	int threshG(int, int, int);	//this method finds the real communities from contact graph
	bool chkComm(int, int, int);	//
	int findLabels();	//method which returns the labels for each id
	int updateRanks(int, int, int, int);	//this method updates the ranks (centrality) of nodes during the simulation for Bubble RAP algorithm where we pass the starttime, currenttime as well as the T (window length)
	int findLSize();	//returns the number of communities according to quationary forms
	bool label(int, int);	//check if id1 and id2 have the same labels
	int rank(int);		//this method returns the rank of given user
	bool search_list(list<int>, int);	//search mem in lst list: return true if it's found false otherwise
//buggy version
//	int generateG(int, int, bool);	//construct the temporal graph	
	int generateG(int, int, int, bool);	//construct the temporal graph: new version!
	list<int> _vertices;	//we keep the nodes of every snapshot of temporal graph in this list  for postprocessing
	map<E, int> _edges;	//we keep the edges set of every snapshot of temporal graph in this map for postprocessing
	double simCalc(contactG, bool);	//this function calculates the similarity of two graphs by comparing their vertices and edges sets
	int clearG();	//delete _edges and _vertices
	int findDens(int, int);
	int generateG(int t0, int t1);	//generate a contact graph
	Graph g_;	//the contact graph
	double CC(int);	//this method calculates the clustering factor of G
	node_id_map_t node_id;
	cont_no_map_t cont_no;
	
	int generateUknownG(int t0, int t1);	//generate the unkown part of the contact graph
	int clearUnknownCG();	//delete edges and vertices
	node_id_map_t uk_node_id;
	cont_no_map_t uk_cont_no;
	Graph uk_g_;	//the uknwon part of contact graph
	IDVertexMap uk_nodes_;	//a map for nodes (id --> vertex)
	map<E, int> _uk_edges;
	
	//a map for keeping the id of added vertices
	IDVertexMap nodes_;	//a map for nodes (id --> vertex)
	Sim_type neighSim(int, int, int, int, ofstream &, int);	//finds the similarities between nodes' neighbor sets
	double sAdAd(int, int, int, int);	//Adamic-Adar measure
	int lastContact(int, int, int, int);	//this method tries to fill the gap on contact graph by taking advantage of the latest contact time
	int filter(list<int> *, list<int> *);	//this method receives the predicted contact times and real contact times and filter them
	list<int> eNodes;	//external node list
	map<int,int> extM;	//a bitmap for external nodes
	int genExt(int);	//generates the list of external nodes
	int clearCG();		//clear the current contact graph
	bool IsContact(int, int, int, int);
	int generateG(int, int, bool, ofstream&);
	void allPaths(vector<int>, int, int, int, ofstream&);
	list< pair<int, int> >	adjT[100];	//adjacency table for paths enumeration
//	list< pair<int, int> >	Paths[100][100];	//the data structure to store all the first k shortest paths
	list< list< pair<int, int> > >	Paths[100];	//the data structure to store all the first k shortest paths
	int nPaths;	//how many paths to each destination
	list< list< pair<int, int> > >	tempX[100];//extended paths from each i
	list< list< pair<int, int> > >	X[100];	//this is the extended path for postprocessing
	int initPaths(int, int);	//init Paths
	int showPaths(ofstream&);	//show all extended paths 
	int processPaths(int, int, ofstream&);	//remove non valid paths and store the valid paths into Paths
	map <quadruple, int> rContMap;	//the real contacts which have been matched with a prediction!
//	unsigned long long int factorial(unsigned long long int);	//calculates the factorial
	long double factorial(unsigned);
	int generateRandomG(int, double);
	int degree_(int);	//this method returns the degree of the given node
	int ncn(int, int);	//find the ncn between two given nodes
	bool edge_exists(int, int);	//returns true if there is an edge between two given nodes
	char _Rank_query [2000];
private:
};
#endif

