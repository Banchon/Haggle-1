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
#include <mysql++.h>
#include <iostream>
#include <cstdlib>
#include <stdio.h>
#include <string>
#include "rgmodel.h"
#include "emulation.h"
#include "contactG.h"
#include <fstream> // file stream      
#include "distr.h"  
//#include <boost/config.hpp>
#include <vector>
#include <algorithm>
#include <utility>

using std::ifstream; // input file stream
using namespace std;
using namespace boost;

//this method simply calculates the factorial of n
// unsigned long long int contactG::factorial(unsigned long long int n)
//  {
//   if (n<=1)
// 	return(1);
//   else
// 	n = n*factorial(n-1);
// 	return(n);
//  }
long double contactG::factorial(unsigned n)
{
    return tgammal(n + 1);
}
// contactG::contactG(configuration *cfg){
// 	//let's setup the configuration for contactG class
// 	config = cfg;
// }
//this method construct the temporal graph in the interval [t0,t1]. In this graph, every node (static or mobile) is a graph node and edges are the contact times between two nodes 
//old version
//int contactG::generateG(int t0, int t1, bool print){
//new version: sample the graph @ time step t: Gt
//method: 0-->time sampling and 1-->interval sampling
int contactG::generateG(int t0, int t1, int method, bool print){
	typedef adjacency_list < vecS, vecS, undirectedS,
	no_property, property < edge_weight_t, int > > Graph;
	typedef graph_traits < Graph >::edge_descriptor Edge;
	typedef graph_traits < Graph >::vertex_descriptor Vertex;
	map<E, int> edges_;	//a map table for existing edges: edge:weight(no of contacts)

	map<int, int>::iterator vv, rr1, rr2;
	map<int, int> vertices_;	//a map for vertices
	map<int, int> rev_vertices_;	//a reverse map for vertices
	int node_inx = 0, inx1, inx2;	//for node id

	E* edge_array;
 	double* weights;
	int num_nodes = 0;

	char _query [6000];
	char str[100];
	bzero(_query, 6000);
	mysqlpp::Connection conn(false);

	// Connect to database 
	if (conn.connect("infocom06", "localhost", "root", "mysql")) {
		// Retrieve a subset of the sample stock table set up by resetdb
		// and display it

		//we will go through all existing contacts between nodes in a community during an interval and insert all of them into another table called simulation table, later on we will use simulation table to emulate message dlivery in our contact graph 
		char _query [200];
		bzero(_query, 200);
		if(method == 0)
		//sampling time method
		sprintf (_query, "select user1, user2 from contactspan where starttime <= %d and %d <= endtime order by starttime", t0, t0);
		else
		//sampling interval method
		sprintf (_query, "select user1, user2 from contactspan where (starttime >= %d and starttime <= %d) or (endtime >= %d and endtime <= %d) order by starttime", t0, t1, t0, t1);

		if(print) cout << _query << endl;
		
		//let's generate simulation table. every row in the simulation table represents an event: a communication between user1 and user2 at starttime until endtime in which direction!
		mysqlpp::Query query = conn.query(_query);

		if (mysqlpp::StoreQueryResult res = query.store()) {
			int max = 0;
			map<E, int>::iterator ii, jj;
			if(print) cout << "no of rows: " << res.num_rows() << endl;
			for (size_t i = 0; i < res.num_rows(); ++i) {

				//going through db and finding contact duration time for each contact and store them inside map table
				int user1 = res[i][0];	//user1
				int user2 = res[i][1];	//user2	
				//keep recording vertices lists
				vv = vertices_.find(user1);

				if((*vv).first != user1){
//					cout << '\t' << "inside" << user1 << endl;
					vertices_.insert(std::make_pair(user1, node_inx));
					rev_vertices_.insert(std::make_pair(node_inx, user1));
					_vertices.push_back(user1);	//let's tore this node
					inx1 = node_inx;	//this node id
					node_inx++;
				}else	inx1 = (*vv).second;	//let's get node id1
					
				
				vv = vertices_.find(user2);

				if((*vv).first != user2){
//					cout << '\t' << "inside" << user2 << endl;
					vertices_.insert(std::make_pair(user2, node_inx));
					rev_vertices_.insert(std::make_pair(node_inx, user2));
					_vertices.push_back(user2);	//let's tore this node
					inx2 = node_inx;	//this node id
					node_inx++;
				}else	inx2 = (*vv).second;	//let's get node id2
								
				if(print) cout << '\t' << user1 << "<--->" << user2 << endl;

				ii = edges_.find(E(inx1, inx2));
				jj = edges_.find(E(inx2, inx1));

				if((*ii).first != E(inx1, inx2) && (*jj).first != E(inx2, inx1)){
//					cout << '\t' << "add: (" << inx1 << "," << inx2 << ")"  << endl;
					edges_.insert(std::make_pair(E(inx1, inx2), 1));
					_edges.insert(std::make_pair(E(user1, user2), 1));
				}
				else if((*ii).first != E(inx1, inx2)) (*jj).second++;
				else (*ii).second++;
				//cout << (*ii).first << ":" << (*ii).second  << endl;			
			}

	//let's allocate memory for the structures
	edge_array = (E *)malloc(sizeof(E)*edges_.size());
	weights = (double *)malloc(sizeof(double)*edges_.size());
	
	int i = 0;	
	for( map<E, int>::iterator ii=edges_.begin(); ii!=edges_.end(); ++ii){
//		cout << "(" << (*ii).first.first << "," <<  (*ii).first.second << ")" << ", weight: " << (*ii).second <<  endl;
		edge_array[i] = (*ii).first;	//save the edge
		weights[i] = (double) 1.0;	//set the weight as 1 for all edges since we are looking at temporal graph every T period
//		weights[i] = (double) 1.0/(double)(*ii).second;	//save the weight
		i++;	//update the pointer
	}

	std::size_t num_edges = edges_.size();
	if(print) cout << "the number of edges: " << num_edges << endl;
	
	for (int i=0; i < edges_.size(); i++ ){
		//we are doing reverse mapping here!
	        rr1 = rev_vertices_.find(edge_array[i].first);	//let's find the actual node id
	        rr2 = rev_vertices_.find(edge_array[i].second);	//let's find the actual node id
		if(print) cout << (*rr1).second << "-" <<  (*rr2).second << ",";
//		cout << "edges: " "(" << (*rr1).second << "," <<  (*rr2).second << ")" << ", weight: " << weights[i] << endl;
	}

	//let's find the no of nodes in G
	num_nodes = vertices_.size();
	if(print) cout << "no of nodes: " << num_nodes << endl;

// 	for( map<int, int>::iterator vv=vertices_.begin(); vv!=vertices_.end(); ++vv)
// 	{
// 		cout << (*vv).first << endl;
// 		
// 	}


	Graph g(edge_array, edge_array + num_edges, weights, num_nodes);

	property_map < Graph, edge_weight_t >::type weight = get(edge_weight, g);
 	std::vector < Edge > spanning_tree;
	kruskal_minimum_spanning_tree(g, std::back_inserter(spanning_tree));
	if(print) std::cout << "Print the edges in the MST:" << std::endl;
	  for (std::vector < Edge >::iterator ei = spanning_tree.begin(); ei != spanning_tree.end(); ++ei) {
	        rr1 = rev_vertices_.find(source(*ei, g));	//let's find the actual node id
	        rr2 = rev_vertices_.find(target(*ei, g));	//let's find the actual node id

		if(print) std::cout << (*rr1).second << "-" << (*rr2).second << ",";
    		//  << " with weight of " << weight[*ei] << std::endl;
  }

//let's calculte the no of components in G
    std::vector<int> component(num_vertices(g));
    int num = connected_components(g, &component[0]);
    
    std::vector<int>::size_type hh;
    if(print) cout << "Total number of components: " << num << endl;
    for (hh = 0; hh != component.size(); ++hh){
      rr1 = rev_vertices_.find(hh);	//let's find the actual node if
      
      if(print) cout << "Vertex " << hh << "(" << (*rr1).second << ")" << " is in component " << component[hh] << endl;
    }
    if(print) cout << endl;

    if(print) cout << "no of nodes (II): " << num_vertices(g) << endl;


  std::ofstream fout("/home/kazem/Desktop/figs/kruskal-eg.dot");
  fout << "graph A {\n"
    << " rankdir=LR\n"
    << " size=\"3,3\"\n"
    << " ratio=\"filled\"\n"
    << " edge[style=\"bold\"]\n" << " node[shape=\"circle\"]\n";

  graph_traits<Graph>::edge_iterator eiter, eiter_end;

  for (tie(eiter, eiter_end) = edges(g); eiter != eiter_end; ++eiter) {
    fout << source(*eiter, g) << " -- " << target(*eiter, g);
    if (std::find(spanning_tree.begin(), spanning_tree.end(), *eiter)
        != spanning_tree.end())
      fout << "[color=\"black\", label=\"" << get(edge_weight, g, *eiter)
           << "\"];\n";
    else
      fout << "[color=\"gray\", label=\"" << get(edge_weight, g, *eiter)
           << "\"];\n";
  }
  fout << "}\n";


		}
		else {
			cerr << "Failed to get item list: " << query.error() << endl;
			return 1;
		}


	}
	else {
		cerr << "DB connection failed: " << conn.error() << endl;
		return 1;
	}

}

//this function calculates the similarity of two graphs by comparing their vertices and edges sets
//input: two consecutive snapshot of temporal connectivity graph
//return value: edge Similarity of G1&G2
double contactG::simCalc(contactG G, bool print){
	if(print){
		cout << "G1: no of nodes: " << _vertices.size() << ", no of edges: " << (int) _edges.size() << endl;
		cout << "G2: no of nodes: " << G._vertices.size() << ", no of edges: " << (int) G._edges.size() << endl;
	}

	int eUnion = _edges.size() + G._edges.size();	//initial union set size of E1 and E2
	int eInter = 0;	//intersection set of E1 and E2
	//let's find the size of intersection set of E1 and E2
	for( map<E, int>::iterator ii=_edges.begin(); ii!=_edges.end(); ++ii)
		for( map<E, int>::iterator jj=G._edges.begin(); jj!=G._edges.end(); ++jj){
//			if((*ii).first == (*jj).first){
			//the four nodes: (u1,u2) and (u3,u4)
			int u1 = (*ii).first.first;
			int u2 = (*ii).first.second;
			int u3 = (*jj).first.first;
			int u4 = (*jj).first.second;
			//let's ignore the direction
			if((u1 == u3 && u2 == u4) || (u1 == u4 && u2 == u3)){
				eInter++;
				break;				
				//cout << "(" << (*ii).first.first << "," <<  (*ii).first.second << ")" <<  endl;
		//		edge_array[i] = (*ii).first;	//save the edge
			}
	}

	list<int>::iterator it1, it2;
	int vUnion = _vertices.size() + G._vertices.size();	//initial union set size of V1 and V2
	int vInter = 0;	//intersection set of V1 and V2
	//let's find the size of intersection set of V1 and V2
	for ( it1 = _vertices.begin() ; it1 != _vertices.end(); it1++ )
		for ( it2 = G._vertices.begin() ; it2 != G._vertices.end(); it2++ )
			if( *it1 == *it2){
//				cout << *it1 << " ";
				vInter++;
				break;
			}
	if (print){
		cout << "EComm: " << eInter << ", ESim: " << (double) eInter / (double) (eUnion - eInter) << endl;

		cout << "VComm: " << vInter << ", VSim: " << (double) vInter / (double) (vUnion - vInter) << endl;
	}

	return (double) eInter / (double) (eUnion - eInter);
}
//this function deletes the snapshot edges and vertices structure and enacles us to have a time sequence graph
int contactG::clearG(){
	_vertices.clear();
	_edges.clear();
}

//let's find the number of communities from interest tables according questionarie forms
int contactG::findLSize(){
	list<int> l_list;	//label list
	map<int, int>::iterator ii;	//iterator for mapping table which shows the mapping between communities and index

	mysqlpp::Connection conn(false);

	/* Connect to database */
	if (conn.connect("infocom06", "localhost", "root", "mysql")) {
		// Retrieve a subset of the sample stock table set up by resetdb
		// and display it
		mysqlpp::Query query = conn.query("select affiliation from interests where affiliation!=''");

		if (mysqlpp::StoreQueryResult res = query.store()) {
			cout << "We have:" << endl;
			int index = 0;
			for (size_t i = 0; i < res.num_rows(); ++i) {
				ii = mapping.find(atoi(res[i][0]));

				if((*ii).first != atoi(res[i][0])){
					mapping.insert(std::make_pair(atoi(res[i][0]), index));	//let's map this community to an index
					index++;
				}

			}

		}
		else {
			cerr << "Failed to get item list: " << query.error() << endl;
			return 1;
		}
	}
	else {
		cerr << "DB connection failed: " << conn.error() << endl;
		return 1;
	}

	for( map<int, int>::iterator ii=mapping.begin(); ii!=mapping.end(); ++ii)
		cout << (*ii).first << " : " << (*ii).second << endl;
		
	cout << "size of map table: " << mapping.size() << endl;

 return mapping.size();
}

//let's find the number and the list of communities from interest tables according questionarie forms
//this method fills the comm_members list which is the mapping from communities to their members
int contactG::findComm(){
//	int s= findLSize();
	map<int, int>::iterator ii;	//iterator for mapping table which shows the mapping between communities and index

	mysqlpp::Connection conn(false);

	// Connect to database 
	if (conn.connect("infocom06", "localhost", "root", "mysql")) {
		// Retrieve a subset of the sample stock table set up by resetdb
		// and display it
		mysqlpp::Query query = conn.query("select affiliation,name from interests where affiliation!=''");

		if (mysqlpp::StoreQueryResult res = query.store()) {
			cout << "We have:" << endl;
			for (size_t i = 0; i < res.num_rows(); ++i) {
				char v1[50];
				bzero(v1, 50); 
				strncpy(v1, res[i][0], 50);
				char * pch;
				//cout << "affiliation: " << v1 << endl;
				pch = strtok (v1,",");
				while (pch != NULL)
				{
					//cout << "pch: " << pch << endl;
					if (pch != NULL){
						int index = atoi(pch) - 1;	//the index on link list
						//cout << "index: " << index << endl;
						comm_members[index].push_back(atoi(res[i][1]));	//push the member name into link list table
					}
					//cout << "pch: (1): " << pch << endl;
					pch = strtok (NULL, ",");
					//if (pch) cout << "pch: (2): " << pch << endl;					
				}	
			}
		}
		else {
			cerr << "Failed to get item list: " << query.error() << endl;
			return 1;
		}
	}
	else {
		cerr << "DB connection failed: " << conn.error() << endl;
		return 1;
	}


 list<int>::iterator k;	// an iterator for the link list

 //let's print the total traffic scheme
 cout << "community member list:" << endl;
int singles = 0;	//he many communities only have a single member
 for (int j = 0; j < 35; j++){
	if (comm_members[j].size() == 1) singles++;
	cout << "community: " << j+1 << ", has folowing members: " << endl;
 for(k = comm_members[j].begin(); k != comm_members[j].end(); ++k) 
	cout << (*k) << ", " ;
 cout << endl;
}
cout << "no of communities with single member: " << singles << endl;
return 0;
}

//this method fills label link list containing the label list for each id
int contactG::findLabels(){

	mysqlpp::Connection conn(false);

	// Connect to database 
	if (conn.connect("infocom06", "localhost", "root", "mysql")) {
		// Retrieve a subset of the sample stock table set up by resetdb
		// and display it
		mysqlpp::Query query = conn.query("select affiliation,name from interests where affiliation!=''");

		if (mysqlpp::StoreQueryResult res = query.store()) {
			cout << "We have:" << endl;
			for (size_t i = 0; i < res.num_rows(); ++i) {
				char v1[50];
				bzero(v1, 50); 
				strncpy(v1, res[i][0], 50);
				char * pch;
				//cout << "affiliation: " << v1 << endl;
				pch = strtok (v1,",");
				while (pch != NULL)
				{
					//cout << "pch: " << pch << endl;
					if (pch != NULL){
						int index = atoi(res[i][1]) - 21;	//the index on link list
						//cout << "index: " << index << endl;
						labels[index].push_back(atoi(pch));	//push the member name into link list table
					}
					//cout << "pch: (1): " << pch << endl;
					pch = strtok (NULL, ",");
					//if (pch) cout << "pch: (2): " << pch << endl;					
				}	
			}
		}
		else {
			cerr << "Failed to get item list: " << query.error() << endl;
			return 1;
		}
	}
	else {
		cerr << "DB connection failed: " << conn.error() << endl;
		return 1;
	}


 list<int>::iterator k;	// an iterator for the link list

 //let's print the total traffic scheme
 cout << "label lists:" << endl;
 for (int j = 0; j < 79; j++){
	cout << "person with id : " << j+21 << ", has the folowing labels: " << endl;
 for(k = labels[j].begin(); k != labels[j].end(); ++k) 
	cout << (*k) << ", " ;
 cout << endl;
}
return 0;
}
//find mem object in lst list
bool contactG::search_list(list<int> lst, int mem){
//let's print the total traffic scheme
 cout << "search in link list to find community: " << mem << endl;
 
 list<int>::iterator k;	// an iterator for the link list
 
 for(k = lst.begin(); k != lst.end(); ++k){
	if ((*k) == mem)	return true; 	
 }
return false;
}

//this method returns true if id1 and id2 belong to the same group
bool contactG::label(int id1, int id2){

list<int>::iterator k, l;	// an iterator for the link list

 //let's print the total traffic scheme
// cout << "label lists:" << endl;
 for(k = labels[id1-21].begin(); k != labels[id1-21].end(); ++k) 
	for(l = labels[id2-21].begin(); l != labels[id2-21].end(); ++l)
		if((*k) == (*l))	return true;

return false;
}

//this method updates the rank link list containing the rank list for each node
//arguments:
//sTime: start time of simulation, cTime: current time of simulation, wLength: window length for which we count the number of unique encounters
//method: 0 --> count all unique contacts for all internal and external nodes
//method: 1--> suppose the external nodes can only be seen
int contactG::updateRanks(int sTime, int cTime, int wLength, int method){
	list<int> cList;	// a list for storing the distinct user1 (those with sensors)
	mysqlpp::Connection conn(false);
	// Connect to database 
	if (conn.connect(config->db, "localhost", "root", "mysql")) {
		// Retrieve a subset of the sample stock table set up by resetdb
		// and display it
		for (int i = config->s_nid; i <= config->e_nid; i++)
		{
			char id[10];	//user id
			bzero (id, 10);
			sprintf(id, "%d", i);
			char _query [2500];
			bzero(_query, 2500);
			int max = (cTime - wLength > sTime ? cTime - wLength : sTime);
			//let's find the number of distinct encounters in the previous wLength seconds for all users
			//should we count all contacts or only contacts which are long enough
			if(method == 0){//only unique contacts for nodes with id <= 99
			sprintf (_query, "select distinct user2 from %s where user2 >= %d and user2 <= %d and starttime >= %d and starttime <= %d and user1 = %s;", config->table, config->s_nid, config->e_nid, max, cTime, id);
			}else if(method == 1){//only unique contacts
			if(extM.find(i) == extM.end())	//internal nodes
			//since id is an internal node with sensor, it can see other nodes
			sprintf (_query, "select distinct user2 from %s where user2 >= %d and user2 <= %d and starttime >= %d and starttime <= %d and user1 = %s;", config->table, config->s_nid, config->e_nid, max, cTime, id);
			else//id is an external node which olny can be seen. since id is an internal node wo sensosrs, it only can be seen by internal nodes which have sensors. 
			sprintf (_query, "select distinct user1 from %s where user1 >= %d and user1 <= %d and starttime >= %d and starttime <= %d and user2 = %s;", config->table, config->s_nid, config->e_nid, max, cTime, id);
			}
			else if(method == 2){//all contacts
	// 			if(extM.find(i) == extM.end())	//internal nodes
	// 			//since id is an internal node with sensor, it can see other nodes
	// 			sprintf (_query, "select user2 from %s where user2 >= %d and user2 <= %d and starttime >= %d and starttime <= %d and user1 = %s;", config->table, config->s_nid, config->e_nid, max, cTime, id);
	// 			else
				if(extM.find(i) != extM.end()){//id is an external node which only can be seen. since id is an internal node wo sensosrs, it only can be seen by internal nodes which have sensors. 
			//	printf("%d is external node!\n", i);
				sprintf (_query, "select user1 from %s where user1 >= %d and user1 <= %d and starttime >= %d and starttime <= %d and user2 = %s", config->table, config->s_nid, config->e_nid, max, cTime, id);
//				strncpy(&_query[strlen(_query)], _Rank_query, strlen(_Rank_query));
			//	printf("query: %s\n", _query);
				}else continue;
			}
			else if(method == 3){//all contacts so far
			if(extM.find(i) == extM.end())	//internal nodes
			//since id is an internal node with sensor, it can see other nodes
			sprintf (_query, "select user2 from %s where user2 >= %d and user2 <= %d and starttime >= %d and starttime <= %d and user1 = %s;", config->table, config->s_nid, config->e_nid, sTime, cTime, id);
			else//id is an external node which ony van be seen. since id is an internal node wo sensosrs, it only can be seen by internal nodes which have sensors. 
			sprintf (_query, "select user1 from %s where user1 >= %d and user1 <= %d and starttime >= %d and starttime <= %d and user2 = %s;", config->table, config->s_nid, config->e_nid, sTime, cTime, id);
			}

// 			cout << _query << endl;		
			mysqlpp::Query query = conn.query(_query);
	
			if (mysqlpp::StoreQueryResult res = query.store()) {
				if(res.num_rows() != 0){
				if(method == 0){
					if(!ranks[i-config->s_nid].empty())	ranks[i -config->s_nid].clear();	//let's clear the list for the new update of rank
					ranks[i-config->s_nid].push_back(res.num_rows());
				}else if(method == 1 || method == 2 || method == 3){
					cList.clear();
					for (size_t j = 0; j < res.num_rows(); ++j) {
						int user1 = atoi(res[j][0]);
						//if user1 has sensor then it can log contact
						if(extM.find(user1) == extM.end())	cList.push_back(user1);
					}
					if(!ranks[i-config->s_nid].empty())	ranks[i -config->s_nid].clear();	//let's clear the list for the new update of rank
					ranks[i-config->s_nid].push_back(cList.size());
				}/*
				else if (method == 2){
					if(!ranks[i-config->s_nid].empty())	ranks[i -config->s_nid].clear();	//let's clear the list for the new update of rank
					ranks[i-config->s_nid].push_back(res.num_rows());
				}*/
				}
				else{
					if(!ranks[i-config->s_nid].empty()) ranks[i -config->s_nid].clear();	//let's clear the list for the new update of rank
					ranks[i-config->s_nid].push_back(0);	//push rank number into link list table
				}
			}
			else {
				cerr << "Failed to get item list: " << query.error() << endl;
				return 1;
			}
		}
	}
	else {
		cerr << "DB connection failed: " << conn.error() << endl;
		return 1;
	}
/*
 list<int>::iterator k;	// an iterator for the link list

 cout << "rank lists:" << endl;
 for (int j = 0; j < 79; j++){
	cout << "user id : " << j + 21 << " has rank: ";
	for(k = ranks[j].begin(); k != ranks[j].end(); ++k) 
		cout << (*k) ;
	cout << endl;
}

*/
return 0;
}

//this method returns the rank of user id
int contactG::rank(int id){
/*
 if(id == 59){
 	cout << "user id : " << id << " has rank: " << ranks[id - 21].front() ;
 	cout << endl;
 }
*/

 return ranks[id - config->s_nid].front();
}

//this method finds the threshold graph from contact graph where starttime=sTime, endtim=eTime and threshold=thresh
int contactG::threshG(int sTime, int eTime, int Tresh){
	mysqlpp::Connection conn(false);

	// Connect to database 
	if (conn.connect("infocom06", "localhost", "root", "mysql")) {
		// Retrieve a subset of the sample stock table set up by resetdb
		// and display it

		for (int i = 21; i <= 99; i++)
		{
			tgAdjTable[i-21].push_back(i);
			for (int j = i+1; j <= 99; j++){
				char _query [400];
				bzero(_query, 400);
				//let's find the number of distinct encounters in the previous wLength seconds for all users
				sprintf (_query, "select sum(endtime-starttime) from contactspan where starttime > %d and starttime < %d and user1=%d and user2=%d;", sTime, eTime, i, j);
		
				//printf ("the query: %s\n", _query);
	
				mysqlpp::Query query = conn.query(_query);
		
				if (mysqlpp::StoreQueryResult res = query.store()) {
					if(res.num_rows() != 0){
						//cout << "the edge between " << i << " and " << j << " 's weight: " << res[0][0] << endl;
						//if(!ranks[i-21].empty())	ranks[i -21].clear();	//let's clear the list for the new update of rank
						if(atoi(res[0][0]) > Tresh){
							//cout << i << " <--> " << j << endl;
							tgAdjTable[i-21].push_back(j);	//push node j into link list table of i
							tgAdjTable[j-21].push_back(i);	//push node i into link list table of node j
							cout << "g.addEdge(\"" << i << "\",\"" << j << "\");" << endl;
						}				
					}
				}
				else {
					cerr << "Failed to get item list: " << query.error() << endl;
					return 1;
				}
			}
		}
	}
	else {
		cerr << "DB connection failed: " << conn.error() << endl;
		return 1;
	}

 list<int>::iterator k;	// an iterator for the link list

 cout << "adjacency lists:" << endl;
 for (int j = 0; j < 79; j++){
	//cout << j + 21 << "--> ";
	if (!tgAdjTable[j].empty())	cout << "g.addVertex(\"" << j + 21 << "\");" << endl;
	//cout << endl;
}
return 0;
}

//gets two node id's (id1 and id2) and checks if they have k-1 common neighbors
bool contactG::chkComm(int id1, int id2, int k){
	list<int>::iterator k1, k2;	// an iterator for the link list
	bool sameC = false;	//similarity flag
	int shCnt = 0;	//no of shared neighbors

	cout << "adjacency lists:" << endl;
	for (int j = 0; j < 79; j++){
		cout << j + 21 << "--> ";
		if (!tgAdjTable[j].empty()){	
			for(k1 = tgAdjTable[j].begin(); k1 != tgAdjTable[j].end(); ++k1)
				cout << (*k1) << ",";
			cout << endl;
		}else	cout << endl;
	}

	for(k1 = tgAdjTable[id1-21].begin(); k1 != tgAdjTable[id1-21].end(); ++k1)
		for(k2 = tgAdjTable[id2-21].begin(); k2 != tgAdjTable[id2-21].end(); ++k2){  
			//cout << (*k1) << " , " << (*k2) << endl;
			if ((*k1) == (*k2)){
				shCnt++;
				cout << shCnt << endl;
			}
			if (shCnt == k-1)	sameC = true;
		}
	return sameC;
}

//let's look at contact density
//we start finding the contact density per hour from sTime until sTime+T
int contactG::findDens(int sTime, int T){
	list<int> contD;	//record no of contacts per hour
	int i = 0;
	char _query [500];
	mysqlpp::Connection conn(false);

	/* Connect to database */
	if (conn.connect("infocom06", "localhost", "root", "mysql")) {
		// Retrieve a subset of the sample stock table set up by resetdb
		// and display it
		while(i < T){

		bzero(_query, 500);
		
		sprintf (_query, "select count(*) from contactspan where starttime > %d and starttime < %d", sTime+i*3600, sTime+(i+1)*3600);

		mysqlpp::Query query = conn.query(_query);

		if (mysqlpp::StoreQueryResult res = query.store())
			contD.push_back(res[0][0]);
		else {
			cerr << "Failed to get item list: " << query.error() << endl;
			return 1;
		}
		i++;
		}
	}
	else {
		cerr << "DB connection failed: " << conn.error() << endl;
		return 1;
	}
 
  list<int>::iterator it;

  cout << "mylist contains:" << endl;
  for ( it=contD.begin() ; it != contD.end(); it++ )
    cout << " " << *it;
  cout << endl;
  return 0;
}

/*
In this method we create the contact graph G=(V,E) during [t0,t1]. We draw an edge between a pair of node if there is at least one contact between them during [t0,t1]. Note that for V we only consider the internal nodes.
*/
int contactG::generateG(int t0, int t1){
	list<int>::iterator it;	//map list iterator 
	std::cout << "Generating G in [" << t0 << "," << t1 << "]" << std::endl;
	//we need a map for keeping the node id map for added vertices
	node_id = get(vertex_name, g_);
	cont_no = get(edge_name, g_);
	map<E, int>::iterator ii, jj;

	//Let's first construct the contact graph
	distr dist(config);
	mysqlpp::Connection conn(false);

	/* Connect to database */
	if (conn.connect(config->db, "localhost", "root", "mysql")) {
		//cout << "Connected to db!" << endl;
		char id1[ 10 ];
		char id2[ 10 ];
		char interest[ 20 ];
		
		char _query [500];
		bzero(_query, 500);

		//sampling interval method: one way is to count contacts left from earlier time
//		sprintf (_query, "select user1, user2 from %s where ((starttime >= %d and starttime <= %d) or (endtime >= %d and endtime <= %d)) and user1 <= %d and user1 >= %d and user2 <= %d and user2 >= %d order by starttime", config->table, t0, t1, t0, t1, config->e_nid, config->s_nid, config->e_nid, config->s_nid);
		//the other way is to not count earlier contacts
//		sprintf (_query, "select user1, user2 from %s where (starttime >= %d and starttime <= %d) and user1 <= %d and user1 >= %d and user2 <= %d and user2 >= %d order by starttime", config->table, t0, t1, config->e_nid, config->s_nid, config->e_nid, config->s_nid);
		//we have to add an edge between two nodes if they see each other during [t0,t1]
		sprintf (_query, "select user1, user2 from %s where ((starttime >= %d and starttime <= %d) or (endtime >= %d and endtime <= %d) or (starttime <= %d and endtime >= %d)) and user1 <= %d and user1 >= %d and user2 <= %d and user2 >= %d order by starttime", config->table, t0, t1, t0, t1, t0, t1, config->e_nid, config->s_nid, config->e_nid, config->s_nid);

//		sprintf (_query, "select user1, user2 from %s where (starttime >= %d and starttime <= %d) and user1 <= %d and user1 >= %d and user2 <= %d and user2 >= %d and ((endtime-starttime) > 0) order by starttime", config->table, t0, t1, config->e_nid, config->s_nid, config->e_nid, config->s_nid);

//		cout << _query << endl;
		mysqlpp::Query query = conn.query(_query);
		if (mysqlpp::StoreQueryResult res = query.store()) {
			for (size_t i = 0; i < res.num_rows(); ++i){
				int user1 = res[i][0];
				int user2 = res[i][1];
				//let's check if user1 is an internal node or not
				int T = 0;
								  
				if(extM.find(user1) != extM.end())	T = 1;		//ID1 is an external node
/*				
				for ( it = eNodes.begin(); it != eNodes.end(); it++ )
					if(*it == user1) T = 1;	//ID1 is an external node
*/
				//we only add edges for which ID1 is an internal node
				if(!T){
				bzero(id1, 10);
				bzero(id2, 10);
				sprintf(id1,"%d", user1); 
				sprintf(id2,"%d", user2);
				IDVertexMap::iterator pos;
				bool inserted_u, inserted_v, inserted_e;
				Vertex u, v;
				//first check if we already have added id1 to G or not?
				tie(pos, inserted_u) = nodes_.insert(std::make_pair(id1, Vertex()));
				if (inserted_u) {
					//if not add node id1 to G
					u = add_vertex(g_);
					node_id[u] = id1;
					pos->second = u;
				} else
					u = pos->second;
				
				//first check if we already have added id2 to G or not?
				tie(pos, inserted_v) = nodes_.insert(std::make_pair(id2, Vertex()));
				if (inserted_v) {
					//if not add id2 to G
					v = add_vertex(g_);
					node_id[v] = id2;
					pos->second = v;
				} else
					v = pos->second;

				//edge descriptor
				graph_traits < Graph >::edge_descriptor e;
				//see if there is any edge between u and v by using edge descriptor
//				tie(pos, inserted_e) = edges_.insert(std::make_pair(std::make_pair(u, v), 1));

				//the graph is undirected, so (id1, id2) = (id2, id1)
				ii = _edges.find(E(user1, user2));	//let's see if we have this an edge in the graph from user1 to user2
				jj = _edges.find(E(user2, user1));	//let's see if we have this an edge in the graph from user2 to user1

				if((*ii).first != E(user1, user2) && (*jj).first != E(user2, user1)){
//					cout << '\t' << "add: (" << user1 << "," << user2 << ")"  << endl;
					_edges.insert(std::make_pair(E(user1, user2), 1));	//add an edge to the map
					tie(e, inserted_e) = add_edge(u, v, g_);	//add the edge to the graph
					cont_no[e] = 1;	//if no edge exists, add the edge with its label
				}else if((*ii).first != E(user1, user2)) (*jj).second++;
				else (*ii).second++;
			}
			}
		}
		else {
			cerr << "Failed to get item list: " << query.error() << endl;
			return 1;
		}
	}
	else {
		cerr << "DB connection failed: " << conn.error() << endl;
		return 1;
	}
return 0;
}

int contactG::generateUknownG(int t0, int t1){
	list<int>::iterator it;	//map list iterator 
	std::cout << "Generating uknown G in [" << t0 << "," << t1 << "]" << std::endl;
	//we need a map for keeping the node id map for added vertices
	uk_node_id = get(vertex_name, uk_g_);
	uk_cont_no = get(edge_name, uk_g_);
	map<E, int>::iterator ii, jj;

	//Let's first construct the contact graph
	distr dist(config);
	mysqlpp::Connection conn(false);

	/* Connect to database */
	if (conn.connect(config->db, "localhost", "root", "mysql")) {
		//cout << "Connected to db!" << endl;
		char id1[ 10 ];
		char id2[ 10 ];
		char interest[ 20 ];
		
		char _query [500];
		bzero(_query, 500);

		//sampling interval method: one way is to count contacts left from earlier time
//		sprintf (_query, "select user1, user2 from %s where ((starttime >= %d and starttime <= %d) or (endtime >= %d and endtime <= %d)) and user1 <= %d and user1 >= %d and user2 <= %d and user2 >= %d order by starttime", config->table, t0, t1, t0, t1, config->e_nid, config->s_nid, config->e_nid, config->s_nid);
		//the other way is to not count earlier contacts
//		sprintf (_query, "select user1, user2 from %s where (starttime >= %d and starttime <= %d) and user1 <= %d and user1 >= %d and user2 <= %d and user2 >= %d order by starttime", config->table, t0, t1, config->e_nid, config->s_nid, config->e_nid, config->s_nid);
		//we have to add an edge between two nodes if they see each other during [t0,t1]
		sprintf (_query, "select user1, user2 from %s where ((starttime >= %d and starttime <= %d) or (endtime >= %d and endtime <= %d) or (starttime <= %d and endtime >= %d)) and user1 <= %d and user1 >= %d and user2 <= %d and user2 >= %d order by starttime", config->table, t0, t1, t0, t1, t0, t1, config->e_nid, config->s_nid, config->e_nid, config->s_nid);

//		sprintf (_query, "select user1, user2 from %s where (starttime >= %d and starttime <= %d) and user1 <= %d and user1 >= %d and user2 <= %d and user2 >= %d and ((endtime-starttime) > 0) order by starttime", config->table, t0, t1, config->e_nid, config->s_nid, config->e_nid, config->s_nid);

//		cout << _query << endl;
		mysqlpp::Query query = conn.query(_query);
		if (mysqlpp::StoreQueryResult res = query.store()) {
			for (size_t i = 0; i < res.num_rows(); ++i){
				int user1 = res[i][0];
				int user2 = res[i][1];
				//let's check if user1 is an internal node or not
				int T = 0;
								  
				if(extM.find(user1) == extM.end() || extM.find(user2) == extM.end())	T = 1;		//ID1 or ID2 is an internal node
/*				
				for ( it = eNodes.begin(); it != eNodes.end(); it++ )
					if(*it == user1) T = 1;	//ID1 is an external node
*/
				//we only add edges for which ID1 and ID2 are external nodes
				if(!T){
				bzero(id1, 10);
				bzero(id2, 10);
				sprintf(id1,"%d", user1); 
				sprintf(id2,"%d", user2);
				IDVertexMap::iterator pos;
				bool inserted_u, inserted_v, inserted_e;
				Vertex u, v;
				//first check if we already have added id1 to G or not?
				tie(pos, inserted_u) = uk_nodes_.insert(std::make_pair(id1, Vertex()));
				if (inserted_u) {
					//if not add node id1 to G
					u = add_vertex(uk_g_);
					uk_node_id[u] = id1;
					pos->second = u;
				} else
					u = pos->second;
				
				//first check if we already have added id2 to G or not?
				tie(pos, inserted_v) = uk_nodes_.insert(std::make_pair(id2, Vertex()));
				if (inserted_v) {
					//if not add id2 to G
					v = add_vertex(uk_g_);
					uk_node_id[v] = id2;
					pos->second = v;
				} else
					v = pos->second;

				//edge descriptor
				graph_traits < Graph >::edge_descriptor e;
				//see if there is any edge between u and v by using edge descriptor
//				tie(pos, inserted_e) = edges_.insert(std::make_pair(std::make_pair(u, v), 1));

				//the graph is undirected, so (id1, id2) = (id2, id1)
				ii = _uk_edges.find(E(user1, user2));	//let's see if we have this an edge in the graph from user1 to user2
				jj = _uk_edges.find(E(user2, user1));	//let's see if we have this an edge in the graph from user2 to user1

				if((*ii).first != E(user1, user2) && (*jj).first != E(user2, user1)){
//					cout << '\t' << "add: (" << user1 << "," << user2 << ")"  << endl;
					_uk_edges.insert(std::make_pair(E(user1, user2), 1));	//add an edge to the map
					tie(e, inserted_e) = add_edge(u, v, uk_g_);	//add the edge to the graph
					uk_cont_no[e] = 1;	//if no edge exists, add the edge with its label
				}else if((*ii).first != E(user1, user2)) (*jj).second++;
				else (*ii).second++;
			}
			}
		}
		else {
			cerr << "Failed to get item list: " << query.error() << endl;
			return 1;
		}
	}
	else {
		cerr << "DB connection failed: " << conn.error() << endl;
		return 1;
	}
return 0;
}

int contactG::generateRandomG(int num, double prob){
	list<int>::iterator it;	//map list iterator 
	char id1[ 10 ];
	char id2[ 10 ];

	std::cout << "Generating a random graph with n = " << num << " and p = " << prob << std::endl;
	//we need a map for keeping the node id map for added vertices
	node_id = get(vertex_name, g_);
	cont_no = get(edge_name, g_);
	map<E, int>::iterator ii, jj;

	//Let's first construct the contact graph
	distr dist(config);

	for(int user1 = 1; user1 <= num; user1++)		
	for(int user2 = user1+1; user2 <= num; user2++){		
		bzero(id1, 10);
		bzero(id2, 10);
		sprintf(id1,"%d", user1); 
		sprintf(id2,"%d", user2);
		IDVertexMap::iterator pos;
		bool inserted_u, inserted_v, inserted_e;
		Vertex u, v;
		//first check if we already have added id1 to G or not?
		tie(pos, inserted_u) = nodes_.insert(std::make_pair(id1, Vertex()));
		if (inserted_u) {
			//if not add node id1 to G
			u = add_vertex(g_);
			node_id[u] = id1;
			pos->second = u;
		} else
			u = pos->second;
		
		//first check if we already have added id2 to G or not?
		tie(pos, inserted_v) = nodes_.insert(std::make_pair(id2, Vertex()));
		if (inserted_v) {
			//if not add id2 to G
			v = add_vertex(g_);
			node_id[v] = id2;
			pos->second = v;
		} else
			v = pos->second;

		//edge descriptor
		double rand = get_rand();
		if(rand <= prob){
//		cout << "add and edge between " << id1 << " and " << id2 << std::endl; 
		graph_traits < Graph >::edge_descriptor e;
		//see if there is any edge between u and v by using edge descriptor

		//the graph is undirected, so (id1, id2) = (id2, id1)
		ii = _edges.find(E(user1, user2));	//let's see if we have this an edge in the graph from user1 to user2
		jj = _edges.find(E(user2, user1));	//let's see if we have this an edge in the graph from user2 to user1

		if((*ii).first != E(user1, user2) && (*jj).first != E(user2, user1)){
		//	cout << '\t' << "add: (" << user1 << "," << user2 << ")"  << endl;
			_edges.insert(std::make_pair(E(user1, user2), 1));	//add an edge to the map
			tie(e, inserted_e) = add_edge(u, v, g_);	//add the edge to the graph
			cont_no[e] = 1;	//if no edge exists, add the edge with its label
		}else if((*ii).first != E(user1, user2)) (*jj).second++;
		else (*ii).second++;
		}
	}

return 0;
}
//this function finds the clustering factor of contact graph generated by generateG (where threshold has been set as thresh for dropping edges on the contact graph)
double contactG::CC(int thresh){
graph_traits < Graph >::edge_descriptor e;
bool inserted;
 graph_traits < Graph >::vertex_iterator i, end;
Vertex u;
double cc = 0;	//clustering factor
//std::cout << "starting the process thresh:" << thresh << std::endl;
//let's go through all vertives of G
  for (boost::tie(i, end) = vertices(g_); i != end; ++i) {
	int cnt = 0;
	u = *i;
//	std::cout << "node: " << node_id[u] << "'s degree: " << out_degree(u,g_) << std::endl;
	int degree = out_degree(u,g_);	//find the degree of node u
//	std::cout << "adjacent vertices of " << node_id[u] << std::endl;
	//now let's go through node u's neighbors to count the number of present edges between them
	graph_traits<Graph>::adjacency_iterator ai_1, ai_2;
	graph_traits<Graph>::adjacency_iterator ai_end_1, ai_end_2;
	for (tie(ai_1, ai_end_1) = adjacent_vertices(u, g_);
		ai_1 != ai_end_1; ++ai_1)
	for (tie(ai_2, ai_end_2) = adjacent_vertices(u, g_);
		ai_2 != ai_end_2; ++ai_2)
		if(node_id[*ai_1] != node_id[*ai_2]){
			tie(e, inserted) = edge(*ai_1, *ai_2, g_);
			//check if there is an edge between two neighbors of u
			if (inserted) {
//			std::cout << "(" << node_id[*ai_1] << "," << node_id[*ai_2] << ") = " << cont_no[e] << " ,";
			cnt++;
		}
	}
//	std::cout << endl;
//	std::cout << "cnt: " << cnt << std::endl;
	//calculate the cc
	if(degree > 1){
//	std::cout << "cc: " << (double)cnt/double(degree*(degree-1)) << std::endl;
	cc += (double)cnt/double(degree*(degree-1));
	}
//	else if (degree == 1)cc += 1.0;	//if the node has degree 1, then the cc is 1!
}
	std::cout << "thresh: " << thresh << " vertices no: " << num_vertices(g_) << " edge no: " << num_edges(g_) << std::endl;
	std::cout << "cc: " << cc/(double)num_vertices(g_) << std::endl;
	return cc/(double)num_vertices(g_);
}

//this method finds the neighborhood similarity between nodes id1 and id2 on the current contact graph
//wFlag: 0 --> no debug and 1: debug
Sim_type contactG::neighSim(int id1, int id2, int t0, int t1, ofstream &sim, int wFlag){
Vertex u, v;
Sim_type Nsim;

//let's convert the nodes to the string format
char n1_[ 10 ];
char n2_[ 10 ];
bzero(n1_, 10);
bzero(n2_, 10);
sprintf(n1_,"%d", id1); 
sprintf(n2_,"%d", id2);

std::map < std::string, Vertex >::iterator it;
it = nodes_.find(n1_);
if(it == nodes_.end()){
	if(wFlag){
	sim << "error: no contacts for " << id1 << std::endl;
	sim << "[" << t0 << "," << t1 << "]" << std::endl;
	sim << "sim: " << -1 << std::endl;
	}
//	sim.close();
	Nsim.ncn_score = -1.0;
	Nsim.jacard_score = -1.0;
	Nsim.min_score = -1.0;

	return Nsim;
}else	u = it->second;

it = nodes_.find(n2_);
if(it == nodes_.end()){
	if(wFlag){
	sim << "error: no contacts for " << id2 << std::endl;
	sim << "[" << t0 << "," << t1 << "]" << std::endl;
	sim << "sim: " << -1 << std::endl;
	}
//	sim.close();
	Nsim.ncn_score = -1.0;
	Nsim.jacard_score = -1.0;
	Nsim.min_score = -1.0;

	return Nsim;
}else	v = it->second;


int degu_ = out_degree(u,g_);	//find the degree of node u

int degv_ = out_degree(v,g_);	//find the degree of node u

int nMin = min(degu_, degv_);	//minimum set size
int nInter = 0;	//intersection set of u and v's neighbor sets

//now let's go through node u's neighbors to count the number of present edges between them
graph_traits<Graph>::adjacency_iterator ai_1, ai_2;
graph_traits<Graph>::adjacency_iterator ai_end_1, ai_end_2;

if(wFlag){
sim << "node: " << node_id[u] << "'s degree: " << degu_ << std::endl;
sim << "node: " << node_id[v] << "'s degree: " << degv_ << std::endl;

sim << "Neighbor set of " << node_id[u] << ":" << std::endl;
for (tie(ai_1, ai_end_1) = adjacent_vertices(u, g_);
	ai_1 != ai_end_1; ++ai_1)
	sim << node_id[*ai_1] << " ";
sim << std::endl;

sim << "Neighbor set of " << node_id[v] << ":" << std::endl;
for (tie(ai_2, ai_end_2) = adjacent_vertices(v, g_);
	ai_2 != ai_end_2; ++ai_2)
	sim << node_id[*ai_2] << " ";
sim << std::endl;
}

for (tie(ai_1, ai_end_1) = adjacent_vertices(u, g_);
	ai_1 != ai_end_1; ++ai_1)
//	std::cout << node_id[*ai_1] << " ";
for (tie(ai_2, ai_end_2) = adjacent_vertices(v, g_);
	ai_2 != ai_end_2; ++ai_2)
	if(node_id[*ai_1] == node_id[*ai_2])
		nInter++;

int nUnion = (degu_ + degv_) - nInter;	//union size
double ncn_ = (double)nInter;	//intersection similarity
double jac_ = (double)nInter/(double)nUnion;	//Jacard similarity
double min_ = (double)nInter/(double)nMin;//one way for calculating the similarit or spatial closeness
//double Nsim = (double)pow(nInter,2.0)/(double)(degu_*degv_);	//geometric

Nsim.ncn_score = ncn_;
Nsim.jacard_score = jac_;
Nsim.min_score = min_;

// long double sum = 0;
// for(int i = nInter; i <= nMin; i++){
// long double prod_1 = factorial(degu_)/(factorial(i)*factorial(degu_-i));
// long double prod_2 = factorial(config->node-degu_)/(factorial(degv_-i)*factorial(config->node-degu_-degv_+i));
// long double prod_3 = (factorial(config->node)/(factorial(config->node-degv_)*factorial(degv_)));
// sum += prod_1*prod_2/prod_3;
// //std::cout << "1: " << prod_1 << ", 2: " << prod_2 << ", 3: " << prod_3 << std::endl; 
// }
//std::cout << "sum: " << sum << std::endl;
//double Nsim = -1*log(sum);
if(wFlag){
sim << "[" << t0 << "," << t1 << "]" << std::endl;
sim << "sim: " << Nsim.ncn_score << " " << Nsim.jacard_score << " " <<  Nsim.min_score << std::endl;
}
return Nsim;
}

//this calculates the score based on Adamic and Adar measure
double contactG::sAdAd(int id1, int id2, int t0, int t1){
Vertex u, v;

ofstream adam( "/home/kazem/Desktop/traces/mobility/adam.dat", ios::app );
	
// exit program if unable to create file
	
if ( !adam ) // overloaded ! operator
{
	cerr << "File adam.dat could not be opened" << endl;
	return -1;
} // end if

//let's convert the nodes to the string format
char n1_[ 10 ];
char n2_[ 10 ];
bzero(n1_, 10);
bzero(n2_, 10);
sprintf(n1_,"%d", id1); 
sprintf(n2_,"%d", id2);

std::map < std::string, Vertex >::iterator it;
it = nodes_.find(n1_);
if(it == nodes_.end()){
	std::cout << "error: no contacts for " << id1 << std::endl;
	adam << "[" << t0 << "," << t1 << "]" << std::endl;
	adam << "adad: " << -1 << std::endl;
	adam.close();
	return -1.0;
}else	u = it->second;

it = nodes_.find(n2_);
if(it == nodes_.end()){
	std::cout << "error: no contacts for " << id2 << std::endl;
	adam << "[" << t0 << "," << t1 << "]" << std::endl;
	adam << "adad: " << -1 << std::endl;
	adam.close();
	return -1.0;
}else	v = it->second;


int degu_ = out_degree(u,g_);	//find the degree of node u
adam << "node: " << node_id[u] << "'s degree: " << degu_ << std::endl;

int degv_ = out_degree(v,g_);	//find the degree of node u
adam << "node: " << node_id[v] << "'s degree: " << degv_ << std::endl;

double adad = 0;	//Adamic-Adar measure

//now let's go through node u's neighbors to count the number of present edges between them
graph_traits<Graph>::adjacency_iterator ai_1, ai_2;
graph_traits<Graph>::adjacency_iterator ai_end_1, ai_end_2;

adam << "Neighbor set of " << node_id[u] << ":" << std::endl;
for (tie(ai_1, ai_end_1) = adjacent_vertices(u, g_);
	ai_1 != ai_end_1; ++ai_1)
	adam << node_id[*ai_1] << " ";
adam << std::endl;

adam << "Neighbor set of " << node_id[v] << ":" << std::endl;
for (tie(ai_2, ai_end_2) = adjacent_vertices(v, g_);
	ai_2 != ai_end_2; ++ai_2)
	adam << node_id[*ai_2] << " ";
adam << std::endl;

for (tie(ai_1, ai_end_1) = adjacent_vertices(u, g_);
	ai_1 != ai_end_1; ++ai_1)
//	std::cout << node_id[*ai_1] << " ";
for (tie(ai_2, ai_end_2) = adjacent_vertices(v, g_);
	ai_2 != ai_end_2; ++ai_2)
	if(node_id[*ai_1] == node_id[*ai_2]){
		adad += 1.0/log((double)out_degree(*ai_1,g_));
		adam << "node: " << node_id[*ai_1] << "'s degree: " << out_degree(*ai_1,g_) << std::endl;
	}

adam << "[" << t0 << "," << t1 << "]" << std::endl;
adam << "Adamic-Adar: " << adad << std::endl;
adam.close();
return adad;
}

//clear the current contact graph
int contactG::clearCG(){
//clear the graph
g_.clear();
nodes_.clear();
_edges.clear();
return 0;
}

//clear the current contact graph
int contactG::clearUnknownCG(){
//clear the graph
uk_g_.clear();
uk_nodes_.clear();
_uk_edges.clear();
return 0;
}

//this method generates a random set of external node , num denote the n of external nodes
int contactG::genExt(int num){
	//let's put both external node in our list
std::cout << "external nodes:" << std::endl;
//int arr_ext[73] = {2, 4, 7, 9, 11, 13, 14, 15, 16, 17, 19, 20, 21, 22, 23, 24, 26, 27, 28, 29, 30, 33, 34, 35, 36, 37, 38, 39, 41, 42, 43, 44, 45, 47, 48, 49, 50, 51, 52, 53, 54, 58, 59, 60, 61, 62, 63, 64, 65, 67, 70, 72, 74, 75, 76, 77, 78, 79, 80, 81, 82, 83, 84, 86, 87, 88, 89, 90, 91, 93, 95, 96, 97};

for (int i=0; i < num; i++){

	int id = ceil(config->node*get_rand()) + config->s_nid - 1;
	while(extM.find(id) != extM.end())	
		id = ceil(config->node*get_rand()) + config->s_nid - 1;

	//id = arr_ext[i];

	std::cout << id << " ";
	eNodes.push_back(id);
	extM.insert ( pair<int,int>(id,1) );
}
std::cout << std::endl;

eNodes.sort();

list<int>::iterator it;
char buf_[30];
bzero(_Rank_query, 2000);
bzero(buf_, 30);
int inx_ = 0;
for ( it = eNodes.begin() ; it != eNodes.end(); it++ ){
	sprintf (buf_, " and user1 != %d", *it);
	strncpy(&_Rank_query[inx_], buf_, strlen(buf_));
	inx_ += strlen(buf_);
	bzero(buf_, 30);
}
strncpy(&_Rank_query[inx_], ";", 1);
// printf("query: %s\n", _Rank_query);
}
//this method tries to infer missing contact by employing the latest contact time factor 
int contactG::lastContact(int ts, int TTL, int n1_, int n2_){

list<int>::iterator it;

//cTimes[i] correspond to the real times that node i has seen an external node
list<int> cTimes[100][100], nTimes[100][100];
list<int>::iterator lit;	//map list iterator 

//lastCont[i] --> a map which keeps the times node i has seen an external node
map<int,int> lastCont[100];	//lastCont[i] belongs to node i to keep track of others. 
map<int,int>::iterator mit;	//map iterator

int T1, T2;	//states
//T1=0 if ID1 is internal node otherwise 1
//T2=0 if ID2 is internal node otherwise 1

//let's store all interaction of networks in log.dat file
ofstream last( "/home/kazem/Desktop/traces/mobility/lastC.dat", ios::app );

// exit program if unable to create file

if ( !last ) // overloaded ! operator
{
	cerr << "File lastC.dat could not be opened" << endl;
	return -1;
}

// Connect to database 
	mysqlpp::Connection conn(false);

	// Connect to database 
	if (conn.connect(config->db, "localhost", "root", "mysql")) {
		// Retrieve a subset of the sample stock table set up by resetdb
		// and display it
		cout << "Connected to db!" << endl;
	//we take into account all contacts (sometimes a contact twice since we counted on both nodes), but now we restricted ourselves to long contact durations. what if we look at all contact duration range	
	char _query [500];
	bzero(_query, 500);
	//we should count all contacts: not only long ones!!!
	sprintf (_query, "select user1, user2, starttime, endtime, enumerator from %s where user1 <= %d and user1 >= %d and user2 <= %d and user2 >= %d and starttime > %d and starttime < %d order by starttime", config->table, config->e_nid, config->s_nid, config->e_nid, config->s_nid, ts, ts + TTL);
	mysqlpp::Query query = conn.query(_query);

	if (mysqlpp::StoreQueryResult res = query.store()){
		
		for (size_t i = 0; i < res.num_rows(); ++i) {
			//going through db and finding contact duration time for each contact and store them inside map table
			int stime = res[i][2];	//starting time
			int etime = res[i][3];	//end time
			int id1 = res[i][0];	//user1
			int id2 = res[i][1];	//user2
			//let's assume both ID1 and ID2 are internal nodes
			T1 = 0;
			T2 = 0;
			//let's check if ID1 and ID2 or internal or external nodes
			for ( it=eNodes.begin() ; it != eNodes.end(); it++ ){
				if(*it == id1)	T1 = 1;	//ID1 is an external node
				else if(*it == id2)	T2 = 1;	//ID2 is an external node
			}

			//ID2 cannot virtually see anybody
			//ID2 is external and ID1 is an internal node
			if(!T1 && T2){
				//first ID1 records the time since it has met an external node
				mit = lastCont[id1-config->s_nid].find(id2);
				if(mit == lastCont[id1-config->s_nid].end())
					lastCont[id1-config->s_nid].insert ( pair<int,int>(id2,stime) );
				else
					mit->second = stime;
				//let's ask ID1 when it has seen other external nodes to predict the contact times, for such we have to ask ID1 about all ther external nodes except ourself (ID2)
				for ( it=eNodes.begin() ; it != eNodes.end(); it++ )
    				if(*it != id2)
				{
				mit = lastCont[id1-config->s_nid].find(*it);
				if(mit != lastCont[id1-config->s_nid].end() && mit->second > 0){
					int dt = stime - mit->second; 
					//if (dt <= 60)
					last << "at " << stime << " " << id1 << " has seen " << (*it) << " id1's timer: " << mit->second << " DT: " << dt;
					if (dt <= 60) last << "<----" << endl;
					else	last << endl;
					if (dt <= 60){
						//let's record the nearby time for n1_
					if(*it <= id2)	
					nTimes[*it - config->s_nid][id2 - config->s_nid].push_back(stime);
					else 
					nTimes[id2 - config->s_nid][*it - config->s_nid].push_back(stime);
					}
				}
				}
			}

			//this is the real time of contact in real data
			//let's record all contact times between external nodes, then later we can evaluate our prediction times
			if(T1 && T2){
				last << id1 << " sees " << id2 << " [" << stime << "," << etime << "] for " << etime-stime << endl; 
				if(id1 <= id2)	cTimes[id1 - config->s_nid][id2 - config->s_nid].push_back(stime);	//let's record the real contact between (n1_,n2_)!
				else cTimes[id2 - config->s_nid][id1 - config->s_nid].push_back(stime);
			}
		}
		last.close();
	}
		else {
			cerr << "Failed to get item list: " << query.error() << endl;
			return 1;
		}
	}
	else {
		cerr << "DB connection failed: " << conn.error() << endl;
		return 1;
	}

	if(n1_ <= n2_)
		filter(&nTimes[n1_ - config->s_nid][n2_ - config->s_nid], &cTimes[n1_ - config->s_nid][n2_ - config->s_nid]);
	else
		filter(&nTimes[n2_ - config->s_nid][n1_ - config->s_nid], &cTimes[n2_ - config->s_nid][n1_ - config->s_nid]);
return 0;
}

int contactG::filter(list<int> *ntimes, list<int> *ctimes){
  list<int>::iterator it, itb;
  cout << "nearby times contains:" << endl;
  int btime, otime;
  for (it=ntimes->begin(); it!=ntimes->end(); it++){

	if(it == ntimes->begin()){
		btime = *it;
	}else{
		itb = it;
		itb--;
		otime = *itb;	//find the earlier state
		if(*it - otime > 132){//event happened
			cout << btime << " ";
			btime = *it;
		}
	}

//  cout << *it << " ";
  }
//let's check if the last one is a time to be printe
it--;
itb = it;
itb--;
if(*it - *itb > 132)	cout << *it << endl;
else cout << endl;

cout << "contact times contains:" << endl;
  for (it=ctimes->begin(); it!=ctimes->end(); it++){

	if(it == ctimes->begin()){
		btime = *it;
	}else{
		itb = it;
		itb--;
		otime = *itb;	//find the earlier state
		if(*it - otime > 132){//event happened
			cout << btime << " ";
			btime = *it;
		}
	}

//  cout << *it << " ";
  }
//let's check if the last one is a time to be printe
it--;
itb = it;
itb--;
if(*it - *itb > 132)	cout << *it << endl;
else cout << endl;

}

//this method tests if there is a contact between id1 and id2 in [t0,t1] interval
bool contactG::IsContact(int id1, int id2, int t0, int t1){
	char _query [500];
	mysqlpp::Connection conn(false);

	/* Connect to database */
	if (conn.connect(config->db, "localhost", "root", "mysql")) {

		bzero(_query, 500);

//		sprintf (_query, "select count(*) from contactspan where ((user1 = %d and user2 = %d) or (user1 = %d and user2 = %d)) and ((starttime <= %d and endtime >= %d) or (starttime <= %d and endtime >= %d))", id1, id2, id2, id1, t0, t0, t1, t1 );

		sprintf (_query, "select user1, user2, starttime, endtime from contactspan where ((user1 = %d and user2 = %d) or (user1 = %d and user2 = %d)) and ((starttime >= %d and starttime <= %d) or (endtime >= %d and endtime <= %d) or (starttime <= %d and endtime >= %d))", id1, id2, id2, id1, t0, t1, t0, t1, t0, t1);

		mysqlpp::Query query = conn.query(_query);

		if (mysqlpp::StoreQueryResult res = query.store()){
//			if(atoi(res[0][0]) > 0)	return true;
			if(res.num_rows() != 0){
			quadruple quad;
			quad.src = res[0][0];	//user1
			quad.dst = res[0][1];	//user2
			quad.stime = res[0][2];	//stime
			quad.etime = res[0][3];	//etime
			rContMap.insert ( pair<quadruple,int>(quad,1) );	//save the real contact which has been matched
			return true;
			}
			else return false;
		}else {
			cerr << "Failed to get item list: " << query.error() << endl;
			return false;
		}
	}
	else {
		cerr << "DB connection failed: " << conn.error() << endl;
		return false;
	}

  return false;
}

//this method is called when we want to enumerate paths
int contactG::generateG(int t0, int t1, bool print, ofstream& log){
	list< list< pair<int, int> > >::iterator it;

	//first clear the adj Table
	for(int i = 0; i < 100; i++){
		adjT[i].clear();	//clean adj table

		//clean X
		for(int i = 0; i < 100; i++)
		if(X[i].size() > 0){
			it = X[i].begin() ;
			(*it).clear();
			while((it = X[i].erase (it)) != X[i].end())	(*it).clear();	
		}

		//clean tempX
		for(int i = 0; i < 100; i++)
		if(tempX[i].size() > 0){
			it = tempX[i].begin() ;
			(*it).clear();
			while((it = tempX[i].erase (it)) != tempX[i].end())	(*it).clear();	
		}

	}

	mysqlpp::Connection conn(false);

	/* Connect to database */
	if (conn.connect("infocom06", "localhost", "root", "mysql")) {
		// Retrieve a subset of the sample stock table set up by resetdb
		// and display it
		//cout << "Connected to db!" << endl;
		
		char _query [200];
		bzero(_query, 200);

		//sampling interval method: all contacts whose ts in [t0,t1)
		sprintf (_query, "select user1, user2 from %s where user1 <= %d and user1 >= %d and user2 <= %d and user2 >= %d and starttime >= %d and starttime < %d order by starttime", config->table, config->e_nid, config->s_nid, config->e_nid, config->s_nid, t0, t1);

//		cout << _query << endl;
		log << "list: " << std::endl;
		mysqlpp::Query query = conn.query(_query);
		if (mysqlpp::StoreQueryResult res = query.store()) {
			for (size_t i = 0; i < res.num_rows(); ++i) {
				int id1 = res[i][0];	//user1
				int id2 = res[i][1];	//user2	
				if(print)	log << id1 << "<-->" << id2 << std::endl;
				//let's tore the link in the adj table
				adjT[id1 - config->s_nid].push_back(std::make_pair(t1, id2 - config->s_nid));
				adjT[id2 - config->s_nid].push_back(std::make_pair(t1, id1 - config->s_nid));				
			}
		}
		else {
			cerr << "Failed to get item list: " << query.error() << endl;
			return 1;
		}
	}
	else {
		cerr << "DB connection failed: " << conn.error() << endl;
		return 1;
	}

//   list< pair<int, int> >::iterator it_;
// 
//   log << "adjacency table contains:" << std::endl;
//   for (int i = 0; i < 100; i++){
//   if(adjT[i].size() > 0){ 
//   for ( it_ = adjT[i].begin() ; it_ != adjT[i].end(); it_++ )
//     log << "adj[" << i+21 << "].push_back(" << (*it_).second + 21 << ");" << std::endl;
//   }
//   }
}

//remmebr that current is node id - 21
//this method extends all paths Pij
//t: current timestep, dest: destination
void contactG::allPaths(vector<int> previous, int current, int dest, int t, ofstream& log)
{
    list< pair<int, int> > X_;
    list< pair<int, int> >::iterator it;
    vector<int>::iterator it_, it_2;

    previous.push_back(current);
    if(previous.size() > 1){
    X_.clear();
    //output all elements of previous, and return
//    log << "path:";
    it_2 = previous.begin();
    it_2++;
    for ( it_ = it_2; it_ < previous.end(); it_++ ){
//	log << " " << *it_ + config->s_nid;
	X_.push_back(std::make_pair(t, *it_));	//let's find the path
    }
//	log << endl;
	it_2 = previous.begin();
        tempX[*it_2].push_back(X_);	//store the new pat to the temporary paths for further analysis
    }
    //check if we have reached the dest: minimal progress
    if(current != dest)
    for ( it = adjT[current].begin() ; it != adjT[current].end(); it++ ){
        int flag = 1;
        for ( it_ = previous.begin() ; it_ < previous.end(); it_++ )
	    if(*it_ == (*it).second)	flag = 0;	//ignore this node since it's already has been visited
    	    if(flag) allPaths(previous, (*it).second, dest, t, log);
    }
}

int contactG::showPaths(ofstream& log){
	list< pair<int, int> > X_;
	list< pair<int, int> >::iterator it_, it_R, it_B, it_P;
	list< list< pair<int, int> > >::iterator it, itP;

/*
	for(int i = 0; i < 100; i++)
	if(tempX[i].size() > 0){
	log << "partial paths, node: " << i + config->s_nid << std::endl;
	for ( it = tempX[i].begin() ; it != tempX[i].end(); it++ ){
		for ( it_ = (*it).begin() ; it_ != (*it).end(); it_++ )
		log << (*it_).second + config->s_nid << " ";
	log << std::endl;
	}
	}
*/
	for(int i = 0; i < 100; i++)
//	for(int j = 0; j < nPaths; j++)
	for ( itP = Paths[i].begin() ; itP != Paths[i].end(); itP++ )
	if((*itP).size() > 0){

	for ( it = tempX[i].begin() ; it != tempX[i].end(); it++ ){
	//let's concat Paths[i][j] with tempX[i]'s paths (the extended path from i)
//	for ( it_ = Paths[i][j].begin() ; it_ != Paths[i][j].end(); it_++ )
	for ( it_ = (*itP).begin() ; it_ != (*itP).end(); it_++ )
	X_.push_back(std::make_pair((*it_).first, (*it_).second));	//the old path from Paths

	for ( it_ = (*it).begin() ; it_ != (*it).end(); it_++ )
	X_.push_back(std::make_pair((*it_).first, (*it_).second));//add the extended part from tempX

	X[i].push_back(X_);	//the whole new path extended from i
	X_.clear();
	}
	}

	//show the completed path at the current step
	for(int i = 0; i < 100; i++)
	if(X[i].size() > 0){
	log << "full paths, node: " << i + config->s_nid << std::endl;
	for ( it = X[i].begin() ; it != X[i].end(); it++ ){
		for ( it_ = (*it).begin() ; it_ != (*it).end(); it_++ )
		log << (*it_).second + config->s_nid << " ";
	log << std::endl;
	}
	}

return 0;
}
//the fist argument is the node that we are going to explore the paths generated from it and the second arg is the nbo of paths that we want to generate for this src to its destinations
int contactG::initPaths(int src, int no_Paths){
	nPaths = no_Paths;
/*
	for(int i = 0; i < 100; i++)
	for(int j = 0; i < nPaths; i++)
		Paths[i][j].clear();
*/
	list< pair<int, int> > X_;
	X_.push_back(std::make_pair(1, src));	//we make a list first
	//let's put the src into the matrix
	Paths[src].push_back(X_);	//we save the list into Paths
	X_.clear();
}

int contactG::processPaths(int t, int dst, ofstream& log){
	list< pair<int, int> >::iterator it_, it_R, it_B, it_F;
	list< list< pair<int, int> > >::iterator it, itP;
	list< pair<int, int> > X_;
	int done = 0;	//if we have generated nPaths for dest, we are done!

	//let's remove all paths with loop: loop avoidance part
	map<int,int> mymap;
	map<int,int>::iterator itM;

//	log << "start of loop avoidance..." << std::endl;
	for(int i = 0; i < 100; i++)
	if(X[i].size() > 0){

	for ( it = X[i].begin() ; it != X[i].end(); it++ ){
		int rep = 0;
		mymap.clear();
		for ( it_ = (*it).begin() ; it_ != (*it).end(); it_++ ){
		itM = mymap.find((*it_).second);	//let's keep track of no of times every node appears in the path
		if(itM == mymap.end())	mymap.insert ( pair<int,int>((*it_).second,1) );//first appearance
		else{
			itM->second++;	//second appearance
			rep = 1;//remove this path
			it_R = it_;	//save to remove this list
			break;
		}
		}
		if(rep)	(*it).clear();	//remove the path with loop
	}
	}

//	log << "end of loop avoidance!" << std::endl;

	//store the extended valid paths in Paths data structure
	for(int i = 0; i < 100; i++)
	if(X[i].size() > 0){
	for ( it = X[i].begin() ; it != X[i].end(); it++ ){
		it_B = (*it).end();
		it_B--;	//the last node in this path
//		int j = 0;//let's find the first place to save this path
		//we have to first make sure that there is not any similar path in Paths at the same timestep
		if(Paths[(*it_B).second].size() <= nPaths){
		//we only store upto nPaths for each destination
		int found = 0;
		itP = Paths[(*it_B).second].begin();
//		log << "node: " << (*it_B).second << ", no of stored paths: " << Paths[(*it_B).second].size() << std::endl;
//		while(Paths[(*it_B).second][j].size() > 0 && j < nPaths && !found){
		while((*itP).size() > 0  && !found){
			if((*itP).size() == (*it).size()){
			found = 1;
			//let's compare two paths
			it_F = (*itP).begin();
			for ( it_ = (*it).begin() ; it_ != (*it).end(); it_++ ){
			if(((*it_F).first != (*it_).first) || ((*it_F).second != (*it_).second)){
				found = 0;	//not matched
				break;
			}
			if(it_F != (*itP).end())	it_F++;
			}
			}
// 			j++;
			itP++;	//let's go to the next path

			if(itP == Paths[(*it_B).second].end()){
//			log << "end of enumeration" << std::endl;
			break;
			}
		}
		//we have to see if this path was already there
		if(!found)
//		if(j < nPaths)							
		for ( it_ = (*it).begin() ; it_ != (*it).end(); it_++ ){
		//let's save the extended path into the right place
//		Paths[(*it_B).second][j].push_back(std::make_pair((*it_).first, (*it_).second));
//		(*itP).push_back(std::make_pair((*it_).first, (*it_).second));
		X_.push_back(std::make_pair((*it_).first, (*it_).second));	//we make a list first
		if(((*it_B).second == dst) && (Paths[(*it_B).second].size() == nPaths ))	done = 1;	//we have 
		}
		Paths[(*it_B).second].push_back(X_);
		X_.clear();
		}
	}
	}
	int length[100];
	
	for(int i = 0; i < 100; i++)
//	for(int j = 0; j < nPaths; j++)
	if(i == dst && Paths[i].size() >= nPaths){
	log << "Paths data structure @ " << t << ", no of paths: " << Paths[i].size() << std::endl;
	for (int z = 0; z < 100; z++)	length[z] = 0;
	for ( itP = Paths[i].begin() ; itP != Paths[i].end(); itP++ )
	if((*itP).size() > 0){
//	log << "i: " << i << ", j: " << j << std::endl; 
	for ( it_ = (*itP).begin() ; it_ != (*itP).end(); it_++ )
//	log << (*it_).second + config->s_nid << " ";
//	log << std::endl;
//	log << (*itP).size() << " ";
	length[(*itP).size()]++;
	}
	}
	if(Paths[dst].size() >= nPaths)
	for (int z = 0; z < 100; z++){
	log << length[z] << " ";
	}
if(done) return 1;
else return 0;
}

//this method finds the neighborhood similarity between nodes id1 and id2 on the current contact graph
int contactG::degree_(int id){
	Vertex u;
	
	//let's convert the nodes to the string format
	char n_[ 10 ];
	bzero(n_, 10);
	sprintf(n_,"%d", id); 
	
	std::map < std::string, Vertex >::iterator it;
	it = nodes_.find(n_);
	if(it == nodes_.end())	return -1;	//we have not found this node in the graph
	else	u = it->second;
	
	int degu_ = out_degree(u,g_);	//find the degree of node u
	
	return degu_;
}

int contactG::ncn(int id1, int id2){
	Vertex u, v;

	//let's convert the nodes to the string format
	char n1_[ 10 ];
	char n2_[ 10 ];
	bzero(n1_, 10);
	bzero(n2_, 10);
	sprintf(n1_,"%d", id1);
	sprintf(n2_,"%d", id2);

	std::map < std::string, Vertex >::iterator it;
	it = nodes_.find(n1_);
	if(it == nodes_.end()){
		return -1;
	}else	u = it->second;

	it = nodes_.find(n2_);
	if(it == nodes_.end()){
		return -1;
	}else	v = it->second;

	int nInter = 0;	//intersection set of u and v's neighbor sets

	//now let's go through node u's neighbors to count the number of present edges between them
	graph_traits<Graph>::adjacency_iterator ai_1, ai_2;
	graph_traits<Graph>::adjacency_iterator ai_end_1, ai_end_2;

	for (tie(ai_1, ai_end_1) = adjacent_vertices(u, g_);
		ai_1 != ai_end_1; ++ai_1)
	//	std::cout << node_id[*ai_1] << " ";
	for (tie(ai_2, ai_end_2) = adjacent_vertices(v, g_);
		ai_2 != ai_end_2; ++ai_2)
		if(node_id[*ai_1] == node_id[*ai_2])
			nInter++;

	return nInter;	//intersection similarity
}

bool contactG::edge_exists(int id1, int id2){
	map<E, int>::iterator ii, jj;
	//the graph is undirected, so (id1, id2) = (id2, id1)
	ii = _edges.find(E(id1, id2));	//let's see if we have this an edge in the graph from id1 to id2
	jj = _edges.find(E(id2, id1));	//let's see if we have this an edge in the graph from id2 to id1

	if((*ii).first != E(id1, id2) && (*jj).first != E(id2, id1))
		return false;
	else return true;
}
