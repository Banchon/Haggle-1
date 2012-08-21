/*
 * ml.cpp
 *
 *  Created on: 2011-10-17
 *      Author: kazem
 */

#include "ml.h"

using namespace std;

//this method extreacts all the features from the cg for a given pair (u,v)
int ml::extract_features(contactG &cg, int k, char *data){
    this->extract_deg_features(cg);

    map<int,int>::iterator it;
    // show all deg-based features
    for ( it=this->degrees.begin() ; it != this->degrees.end(); it++ ){
      if(0) cout << (*it).first << " => " << (*it).second << endl;
    }
    //extract ncn for all pairs
    this->extract_ncn_features(cg);
    //store all features
    this->store_all_features(cg);
    this->print_feature_vectors();
    this->export_features(k, data);

    return 0;
}

//this method extracts all degree based features
int ml::extract_deg_features(contactG &cg){
  	  std::cout << " vertices no: " << num_vertices(cg.g_) << " edge no: " << num_edges(cg.g_) << std::endl;

	  graph_traits < Graph >::vertex_iterator i, end;
	  Vertex u;
	  //let's go through all vertives of G
	  for (boost::tie(i, end) = vertices(cg.g_); i != end; ++i) {
		u = *i;
		//std::cout << "node: " << cg.node_id[u] << "'s degree: " << out_degree(u,cg.g_) << std::endl;
		//int degree = out_degree(u, cg.g_);	//find the degree of node u

		stringstream ss(cg.node_id[u]); // Could of course also have done ss("1234") directly.
	    int id_;
	    if( !(ss >> id_).fail() )
	    {
	    	//success: store all node degrees in a map for later processings
	    	this->degrees.insert( pair<int,int>(id_, out_degree(u,cg.g_)) );
	    	this->Nodes.push_back(id_);
	    }
	  }
	  return 0;
}

int ml::extract_ncn_features(contactG &cg){
	list<int>::iterator it_1, it_2, it_3;
	//let's generate all possible pairs among all nodes in cg
	for ( it_1 = this->Nodes.begin() ; it_1 != this->Nodes.end(); it_1++ ){
		it_3 = it_1;
		it_3++;
		for ( it_2 = it_3 ; it_2 != this->Nodes.end(); it_2++ ){
			int ncn_ = cg.ncn(*it_1, *it_2);
			//std::cout << "(" << *it_1 << "," << *it_2 << ") =>" << ncn_ << std::endl;
			this->ncn.insert(pair<pair<int,int>,int >(pair<int,int>(*it_1, *it_2), ncn_));
		}
	}
	return 0;
}

int ml::store_all_features(contactG &cg){
	map<pair<int,int>,int>::iterator it_1;
	map<int,int>::iterator it_2;
	//let's go through ncn map
	for ( it_1 = this->ncn.begin() ; it_1 != this->ncn.end(); it_1++ ){
	    cout << "(" << (*it_1).first.first << "," << (*it_1).first.second << ")" << " => " << (*it_1).second << endl;
	    feature fet;
	    fet.nodes_pair = (*it_1).first;
	    fet.ncn = (*it_1).second;
	    fet.first_deg = this->degrees.find((*it_1).first.first)->second;		//first node degree
	    fet.sec_deg = this->degrees.find((*it_1).first.second)->second;		//second node degree
	    fet.deg_prod = fet.first_deg*fet.sec_deg;

	    if(cg.edge_exists((*it_1).first.first,(*it_1).first.second))
	    	fet.edge = 1;
	    else
	    	fet.edge = 0;
	    this->features.push_back(fet);
	}
	return 0;
}

int ml::print_feature_vectors(){
	list<feature>::iterator it;
	int sum_=0;
	cout << "feature vectors:";
	for ( it = features.begin() ; it != features.end(); it++ ){
		std::cout << "(" << (*it).nodes_pair.first << "," << (*it).nodes_pair.second << ") => " << "y = " << (*it).edge << " & x=(" << (*it).first_deg << "," << (*it).sec_deg << "," << (*it).deg_prod << "," << (*it).ncn << ")" << std::endl;
		if((*it).edge)	sum_++;
		//		std::cout << (*it).first_deg << std::endl;
	}
	std::cout << sum_ << std::endl;
	return 0;
}

int ml::export_features(int k, char *data){
	struct tm *current;
	time_t now;
	time(&now);
	current = localtime(&now);

	//open a file to export the features
	char _file [200];
	bzero(_file, 200);
	sprintf (_file, "/home/kazem/Desktop/traces/mobility/ml/features_%d_%s.dat", k, data);
	ofstream features_( _file, ios::app );

	if ( !features_ ) // overloaded ! operator
	{
		cerr << "File features.dat could not be opened" << endl;
		return -1;
	} // end if

	//open a file to output the output variable
	bzero(_file, 200);
	sprintf (_file, "/home/kazem/Desktop/traces/mobility/ml/outputs_%d_%s.dat", k, data);
	ofstream output_( _file, ios::app );

	// exit program if unable to create file
	if ( !output_ ) // overloaded ! operator
	{
		cerr << "File outputs.dat could not be opened" << endl;
		return -1;
	} // end if

	list<feature>::iterator it;

	for ( it = features.begin() ; it != features.end(); it++ ){
		features_ << (*it).first_deg << "\t" << (*it).sec_deg << "\t" << (*it).deg_prod << "\t" << (*it).ncn << std::endl;
		output_ << (*it).edge << std::endl;
	}

    features_.close();
    output_.close();
	return 0;
}

int ml::generate_M(contactG &cg, int t, char *data){
	  graph_traits < Graph >::vertex_iterator i, end;
	  Vertex u;
	  //let's save the number of nodes in cg
	  nodes_num = num_vertices(cg.g_);

	  //let's go through all vertices of G and insert them into Node list
	  for (boost::tie(i, end) = vertices(cg.g_); i != end; ++i) {
		u = *i;
		stringstream ss(cg.node_id[u]); // Could of course also have done ss("1234") directly.
	    int id_;
	    if( !(ss >> id_).fail() )
	    {
	    	//store all nodes in Nodes list
	    	this->Nodes.push_back(id_);
	    }
	  }

	  list<int>::iterator it_1, it_2, it_3;
	  //sort id's
	  Nodes.sort();
	  int id_ = 0;
	  //generate the map from node id to matrix index
	  for ( it_1 = this->Nodes.begin() ; it_1 != this->Nodes.end(); it_1++ ){
		  //cout << (*it_1) << " ";
		  id_map[(*it_1)] = id_;
		  id_++;
	  }

	//let's generate all possible pairs among all nodes in cg
	for ( it_1 = this->Nodes.begin() ; it_1 != this->Nodes.end(); it_1++ ){
		it_3 = it_1;
		it_3++;
		for ( it_2 = it_3 ; it_2 != this->Nodes.end(); it_2++ ){
			//matrix index for nodes u and v
			int id_u, id_v;
			id_u = id_map.find((*it_1))->second;
			id_v = id_map.find((*it_2))->second;

			if(id_u > id_v){
				int temp = id_u;
				id_u = id_v;
				id_v = temp;
			}
			pair<int,int> cont_(id_u, id_v);
			//let's find out if there is an edge between the corresponding nodes or not
		    if(cg.edge_exists((*it_1), (*it_2)) || cg.edge_exists((*it_2), (*it_1))){
		    	//there is an edge
		    	M[cont_] = 1;
		    }else{
		    	//no edge
		    	M[cont_] = 0;
		    }
		}
	}

	return 0;
}

int ml::write_M(int k, char *data){
	//open a file to export the features
	char _file [200];
	bzero(_file, 200);
	sprintf (_file, "/home/kazem/Desktop/traces/mobility/ml/M_%d_%s.dat", k, data);
	ofstream M_( _file, ios::app );

	if ( !M_ ) // overloaded ! operator
	{
		cerr << "File M.dat could not be opened" << endl;
		return -1;
	} // end if

	map< pair<int,int>, int >:: iterator it;
	for (int i = 0; i < nodes_num; i++){
		for (int j = 0; j < nodes_num; j++){
			int id_u = i;
			int id_v = j;

			if(id_u > id_v){
				int temp = id_u;
				id_u = id_v;
				id_v = temp;
			}
			pair<int,int> cont_(id_u, id_v);
			it = M.find(cont_);

			M_ <<  i << "\t"	<< j << "\t";
			if((*it).second){
				M_ << 1 << "\t";
			}else{
				M_ << 0 << "\t";
			}
			M_ << endl;
		}
	}
	return 0;
}
