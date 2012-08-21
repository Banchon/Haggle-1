// /***************************************************************************
//  *   Copyright (C) 2009 by Kazem   *
//  *   root@HERMES   *
//  *                                                                         *
//  *   This program is free software; you can redistribute it and/or modify  *
//  *   it under the terms of the GNU General Public License as published by  *
//  *   the Free Software Foundation; either version 2 of the License, or     *
//  *   (at your option) any later version.                                   *
//  *                                                                         *
//  *   This program is distributed in the hope that it will be useful,       *
//  *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
//  *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
//  *   GNU General Public License for more details.                          *
//  *                                                                         *
//  *   You should have received a copy of the GNU General Public License     *
//  *   along with this program; if not, write to the                         *
//  *   Free Software Foundation, Inc.,                                       *
//  *   59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.             *
//  ***************************************************************************/
// 
#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <iostream>
#include <cstdlib>
#include "parser.h"
#include "distr.h"
#include "rgmodel.h"
#include <fstream> // file stream        
#include "statistics.h"
#include "emulation.h"
#include "contactG.h"
#include <list>
#include "message.h"
#include "traffic.h"
#include "routing.h"
#include <math.h>
#include "quadruple.h"
#include "weakcond.h"
#include "Sim_type.h"
#include "ml.h"

using std::ifstream; // input file stream

using namespace std;
//the eclipse version
//a Quad structure for storing infomation about predicted moves
typedef struct {
	int id1;
	int id2;
	int time;
	double score;	//score
}Quad;

//a Penta structure for storing infomation about predicted moves
typedef struct {
	int id1;
	int id2;
	int time;
	double neigh_sim;	//neighborhood similarity
	double score;		//social distance or the popularity
}Penta;

typedef struct {
	int match;
	double score;	//score
	double ncn;
}Tri;

typedef struct {
	int id1;
	int id2;
	double ts;	//starting time of the contact
	double te;	//ending time of the contact
}Contact;

//comparison func for sorting Quad list
bool compare_pairs (Quad q1_, Quad q2_)
{
	if (q1_.score > q2_.score)	return true;
	else return false;
}

//reverse comparison func for sorting Quad list
bool compare_pairs_r (Quad q1_, Quad q2_)
{
	if (q1_.score < q2_.score)	return true;
	else return false;
}

//comparison func for sorting Penta list
bool compare_pairs_penta (Penta p1_, Penta p2_)
{
	if (p1_.score > p2_.score)	return true;
	else return false;
}

//reverse comparison func for sorting Penta list
bool compare_pairs_penta_r (Penta p1_, Penta p2_)
{
	if (p1_.score < p2_.score)	return true;
	else return false;
}

int main(int argc, char *argv[])
{
	srand ( (unsigned)time ( NULL ) );
	//this is the configuration of this simulation
	configuration config;	//let's setup of this test setting
	char dataset[] = "infocom06"; 	//we have to set this based on our input dataset
	//let's setup the data set we are going to use for this simulation
	
	if(strncmp(dataset,"infocom06",9) == 0){
		//infocom06
		cout << "using inf06 as the input dataset" << std::endl;
		sprintf(config.db, "%s", "infocom06");	//db name
		sprintf(config.table, "%s", "contactspan");	//table name
		config.s_nid = 21;	//the range of internal nodes id
		config.e_nid = 99;	//the range of internal nodes id
		config.node = 79;	//the no of internal nodes
	}else if(strncmp(dataset,"infocom05",9) == 0){
		//infocom05
		cout << "using inf05 as the input dataset" << std::endl;
		sprintf(config.db, "%s", "infocom05");	//db name
		sprintf(config.table, "%s", "contactspan");	//table name
		config.s_nid = 1;	//the range of internal nodes id
		config.e_nid = 41;	//the range of internal nodes id
		config.node = 41;	//the no of internal nodes
	}else if(strncmp(dataset,"intel",5) == 0){
		cout << "using intel as the input dataset" << std::endl;
		sprintf(config.db, "%s", "intel");	//db name
		sprintf(config.table, "%s", "contactspan");	//table name
		config.s_nid = 2;	//the range of internal nodes id
		config.e_nid = 9;	//the range of internal nodes id
		config.node = 8;	//the no of internal nodes
	}else if(strncmp(dataset,"cambsyslab",10) == 0){
		cout << "using cambridge system lab as the input dataset" << std::endl;
		sprintf(config.db, "%s", "cambsyslab");	//db name
		sprintf(config.table, "%s", "contactspan");	//table name
		config.s_nid = 1;	//the range of internal nodes id
		config.e_nid = 12;	//the range of internal nodes id
		config.node = 12;	//the no of internal nodes
	}else if(strncmp(dataset,"syntmob",7) == 0){
		//synthetic mobility data
		cout << "using synthetic mobility as the input dataset" << std::endl;
		sprintf(config.db, "%s", "syntmob");	//db name
		sprintf(config.table, "%s", "swn_100_5_02_04");	//table name
		config.s_nid = 0;	//the range of nodes id
		config.e_nid = 99;	//the range of nodes id
		config.node = 100;	//the no of nodes
	}else if(strncmp(dataset,"cambridge",9) == 0){
		cout << "using cambridge city as the input dataset" << std::endl;
		sprintf(config.db, "%s", "cambridge");	//db name
		sprintf(config.table, "%s", "contactspan");	//table name
		config.s_nid = 1;	//the range of internal nodes id
		config.e_nid = 36;	//the range of internal nodes id
		config.node = 36;	//the no of internal nodes
	}else if(strncmp(dataset,"rollernet",9) == 0){
		cout << "using rollernet city as the input dataset" << std::endl;
		sprintf(config.db, "%s", "rollernet");	//db name
		sprintf(config.table, "%s", "contactspan");	//table name
		config.s_nid = 1;	//the range of internal nodes id
		config.e_nid = 62;	//the range of internal nodes id
		config.node = 62;	//the no of internal nodes
	}else if(strncmp(dataset,"simcontacts",11) == 0){
		cout << "using simulated contact as the input dataset" << std::endl;
		sprintf(config.db, "%s", "simcontacts");	//db name
		sprintf(config.table, "%s", "contactspan");	//table name
		config.s_nid = 0;	//the range of internal nodes id
/*		config.e_nid = 89;	//the range of internal nodes id
		config.node = 90;	//the no of internal nodes		*/
		config.e_nid = 99;	//the range of internal nodes id
		config.node = 100;	//the no of internal nodes		
	}else if(strncmp(dataset,"reality",7) == 0){
		//infocom05
		cout << "using reality as the input dataset" << std::endl;
		sprintf(config.db, "%s", "reality");	//db name
		sprintf(config.table, "%s", "contactspan");	//table name
		config.s_nid = 1;	//the range of internal nodes id
		config.e_nid = 97;	//the range of internal nodes id
		config.node = 97;	//the no of internal nodes
	}
//ML
if(1){
    contactG cg;
	cg.config = &config;
	
	int t0 = 144000;	//starting time for constructing CG --> inf06
 	int tend = 0;		//135;		//for conference setting on one day
	int T = 240;		//interval length

	for (int k = 0; k <= tend; k++){
	  ml learning;
	  cg.generateG(t0+k*T, t0+(k+1)*T);
	  //let's extract the features of cg
	  //learning.extract_features(cg, k, config.db);
	  learning.generate_M(cg, k, config.db);
	  learning.write_M(k, config.db);
	  learning.ml::~ml();
	  cg.clearCG();	//clear the previous CG for the next step		  
	}	
	
}
if(0){
	int ext_no = 0;
	int simTimes = 100;
	
	if(argc != 3){
		std::cout << "usage: haggle ext_no time" << std::endl;
		return -1;
	}else{
		ext_no = atoi(argv[1]);	//no of external nodes
		simTimes = atoi(argv[2]);
		std::cout << "running the simulation for " << simTimes << " times with ext_no: " << ext_no << std::endl;
	}
	
  	contactG cg;
	cg.config = &config;	
	cg.genExt(ext_no);

	int t0 = 144000;	//starting time for constructing CG --> inf06
//	int t0 = 70000;		//starting time for inf05
 	int tend = 20;//135;		//for conference setting on one day
	int T = 240;		//interval length
	int dephase = 132;	

	for (int k = 0; k <= tend; k++){
	  cg.generateUknownG(t0+k*T, t0+(k+1)*T);
	  std::cout << "no of nodes in unknown cg: " << cg.uk_nodes_.size() << ", no of edges in unknown cg: " << cg._uk_edges.size() << std::endl ;
	  cg.clearUnknownCG();
	}
}
//compute RMSE
if(0){
	//let's construct the contact graph with different threshold (no of contacts between two vertices) and then study the properties of the resulting graph
	int ext_no = 0;
	int simTimes = 100;
	double sim_thresh_ = 1.0;	//the threshold for RMSE computation
	int RMSE = 0;
	int method = 1;	//ncn
	
	if(argc != 5){
		std::cout << "usage: haggle ext_no time method(1: ncn, 2:jac, 3:min) sim_threshold" << std::endl;
		return -1;
	}else{
		ext_no = atoi(argv[1]);	//no of external nodes
		simTimes = atoi(argv[2]);
		method = atoi(argv[3]);
		sim_thresh_ = atof(argv[4]);
		std::cout << "running the simulation for " << simTimes << " times with ext_no: " << ext_no << " threshold: " << sim_thresh_ << std::endl;
	}

	char _file [200];
	bzero(_file, 200);
	sprintf (_file, "/home/kazem/Desktop/traces/mobility/pred/predl_rms_%dext_%dt_%s.dat", ext_no, simTimes, config.db);
	ofstream predl( _file, ios::app );
		
	if ( !predl ) // overloaded ! operator
	{
		cerr << "File predl.dat could not be opened" << endl;
		return -1;
	} // end if

	bzero(_file, 200);
	sprintf (_file, "/home/kazem/Desktop/traces/mobility/pred/sim_rms_%dext_%dt_%s.dat", ext_no, simTimes, config.db);
	ofstream sim( _file, ios::app );
		
	// exit program if unable to create file
		
	if ( !sim ) // overloaded ! operator
	{
		cerr << "File sim.dat could not be opened" << endl;
		return -1;
	} // end if
	
	//dataset settings
	int t0 = 144000;	//starting time for constructing CG --> inf06
 	int tend = 135;		//for conference setting on one day
	int T = 240;		//interval length
	int dephase = 132;
	
	for (int z = 1; z <= simTimes; z++ ){
	contactG cg;
	list<int>::iterator it_1, it_2, it_3;
	list< Quad >::iterator it;

	cg.config = &config;
	cg.genExt(ext_no);
	list<int>::iterator itE;
	predl << "eNodes:";
	for ( itE = cg.eNodes.begin() ; itE != cg.eNodes.end(); itE++ )
	predl << *itE << ", " ;
	predl << std::endl;

	map<E, int>::iterator itM;
	list< Quad > merged;	//merged version	

	for (int k = 0; k <= tend; k++){
	//since we are intersted in the neighborhood set at a timestamp, method = 0
	cg.generateG(t0+k*T, t0+(k+1)*T);	//let's generate the graph

	std::cout << " no of nodes in known cg: " << cg.nodes_.size() << ", no of edges in known cg: " << cg._edges.size() << std::endl ;
	
	if(cg._edges.size() > 0)
	for ( it_1 = cg.eNodes.begin() ; it_1 != cg.eNodes.end(); it_1++ ){
	//let's generate the predictions for each interval Ti
	it_3 = it_1;
	it_3++;
	for ( it_2 = it_3 ; it_2 != cg.eNodes.end(); it_2++ ){
		Sim_type Nsim = cg.neighSim(*it_1,*it_2,t0+k*T, t0+(k+1)*T, sim, 0);	//neighbor similarity
		//ncn
		if(method == 1){
		if(Nsim.ncn_score == -1.0)	Nsim.ncn_score = 0.0;	//no common neighbor
		if(Nsim.ncn_score >= (int)sim_thresh_){
			//std::cout << "Sim(" << *it_1 << "," << *it_2 << ")=" << Nsim.ncn_score << std::endl;
			Quad q;
			q.id1 = *it_1;
			q.id2 = *it_2;
			q.time = k;
			q.score = Nsim.ncn_score;
			merged.push_back( q );
		}
		}else if(method == 2){
		//jacard
		if(Nsim.jacard_score == -1.0)	Nsim.jacard_score = 0.0;	//no common neighbor
		if(Nsim.jacard_score >= sim_thresh_){
			//std::cout << "Sim(" << *it_1 << "," << *it_2 << ")=" << Nsim.jacard_score << std::endl;
			Quad q;
			q.id1 = *it_1;
			q.id2 = *it_2;
			q.time = k;
			q.score = Nsim.jacard_score;
			merged.push_back( q );
		}
		}else{
		//min score
		if(Nsim.min_score == -1.0)	Nsim.min_score = 0.0;	//no common neighbor
		if(Nsim.min_score >= sim_thresh_){
			//std::cout << "Sim(" << *it_1 << "," << *it_2 << ")=" << Nsim.min_score << std::endl;
			Quad q;
			q.id1 = *it_1;
			q.id2 = *it_2;
			q.time = k;
			q.score = Nsim.min_score;
 			merged.push_back( q );
		}
		}
	}
	}
	//generate the unknown part of CG
	cg.generateUknownG(t0+k*T, t0+(k+1)*T);
	std::cout << "no of nodes in unknown cg: " << cg.uk_nodes_.size() << ", no of edges in unknown cg: " << cg._uk_edges.size() << std::endl ;		
	int obs_unmacthed_count = 0;
	int obs_macthed_count = 0;
	int pred_unmacthed_count = 0;
	int pred_macthed_count = 0;
	
	std::cout << "no of predictions: " << merged.size() << std::endl;
	//std::cout << "edges in uknown cg to be predicted" << std::endl;
	for ( itM = cg._uk_edges.begin() ; itM != cg._uk_edges.end(); itM++ ){
	  //std::cout << "(" << (*itM).first.first << ", " << (*itM).first.second << ")" << std::endl;
	  //we have to search to see if this edge has been predicted successfuly or not
	  bool matched_ = false;
	  for ( it = merged.begin() ; it != merged.end(); it++ )
	    if(((*it).id1 == (*itM).first.first && (*it).id2 == (*itM).first.second) || ((*it).id2 == (*itM).first.first && (*it).id1 == (*itM).first.second)){
	      //there is a successful prediction for this edge in unknown cg
	      matched_ = true;
	      break;
	    }
	  
	  if(!matched_){
	    RMSE += 1;
	    obs_unmacthed_count++;
	  }else{
	    obs_macthed_count++;
	  }
	}
	
	//let's check the prediction list to find oit how many of them are matched and how many are not matched
	pred_unmacthed_count = 0;
	pred_macthed_count = 0;
	for ( it = merged.begin() ; it != merged.end(); it++ ){
	  //we have to search to see if this edge has been predicted successfuly or not
	  bool matched_ = false;
	  
	  for ( itM = cg._uk_edges.begin() ; itM != cg._uk_edges.end(); itM++ )	  
	    /*
	    if(((*it).id1 == (*itM).first.first && (*it).id2 == (*itM).first.second) || ((*it).id2 == (*itM).first.first && (*it).id1 == (*itM).first.second)){
	      //there is a successful prediction for this edge in unknown cg
	      matched_ = true;
	      break;
	    }
	    */
	    
	    if(cg.IsContact((*it).id1, (*it).id2, t0 + ((*it).time)*T - dephase, t0 + ((*it).time+1)*T + dephase)){
	      //there is a successful prediction for this edge in unknown cg
	      matched_ = true;	      
	    }	    
	  
	  if(!matched_){
	    RMSE += 1;
	    pred_unmacthed_count++;
	  }else{
	    pred_macthed_count++;
	  }
	}
	std::cout << "no of obs matched: " << obs_macthed_count << ", no of observed unmatched: " << obs_unmacthed_count << std::endl;
	std::cout << "no of pred matched: " << pred_macthed_count << ", no of pred unmatched: " << pred_unmacthed_count << std::endl;
	merged.clear();
	
	cg.clearCG();	//clear the previous CG for the next step
	//clear the unkwon part of the CG
	cg.clearUnknownCG();	
	}
      }
      
      if(method == 1)
	std::cout << "NCN method RMSE" << std::endl;
      else if(method == 2)
	std::cout << "Jac method" << std::endl;
      else
	std::cout << "Min method" << std::endl;
      
      std::cout << "Root Mean Square Error: " << RMSE << std::endl;
      std::cout << "Root Mean Square Error: " << (double)2*RMSE/(double)(tend*ext_no*(ext_no - 1)*simTimes) << std::endl;
      
      predl.close();
      sim.close();
}
//ncn, jac, and min methods analysis
//prediction
//this is our code to test the power of different similarity metrics for contact prediction
if(0){
	//let's construct the contact graph with different threshold (no of contacts between two vertices) and then study the properties of the resulting graph
// 	double prob[30][10];	//max thresh:29 and max state :10
// 	int matched[30][10];
// 	int total[30][10];
// 
// 	for(int i=0; i< 29; i++)
// 	for(int j=0; j< 10; j++){
// 	prob[i][j] = 0.0;
// 	matched[i][j] = 0;
// 	total[i][j] = 0;
// 	}

	int order = 0;  //0:descending, 1:ascending
	int ext_no = 0;
	int simTimes = 100;
	int state_no = 6;	//no of future states that we are going to look at
	int bins[3][20];	//we want to find the first 2^t predictions
	int binf = 0;	//a flag for finding bins:0 disables finding bins
	//bins[0]-->NCN, bins[1]-->jac, and bins[2]-->min
	for(int i = 0; i < 3; i++)
	for(int j = 0; j < 20; j++)
	bins[i][j] = 0;
	
	if(argc != 6){
		std::cout << "usage: haggle ext_no state_no time binf(0: wo binning, 1: w binning) order (0:descend, 1:ascend)" << std::endl;
		return -1;
	}else{
		ext_no = atoi(argv[1]);	//no of external nodes
		state_no = atoi(argv[2]);
		simTimes = atoi(argv[3]);
		binf = atoi(argv[4]);	//bin flag
 		order = atoi(argv[5]);  //ordering
		std::cout << "running the simulation for " << simTimes << " times with ext_no: " << ext_no << ", extracting the " << state_no << " next states!" << ", bins: " << binf << ", order: " << order << std::endl;
	}

	char _file [200];
	bzero(_file, 200);
	sprintf (_file, "/home/kazem/Desktop/traces/mobility/pred/predl_%dext_%dstate_%dt_%s.dat", ext_no, state_no, simTimes, config.db);
	ofstream predl( _file, ios::app );
		
	// exit program if unable to create file
		
	if ( !predl ) // overloaded ! operator
	{
		cerr << "File predl.dat could not be opened" << endl;
		return -1;
	} // end if

	bzero(_file, 200);
	sprintf (_file, "/home/kazem/Desktop/traces/mobility/pred/sim_%dext_%dstate_%dt_%s.dat", ext_no, state_no, simTimes, config.db);
	ofstream sim( _file, ios::app );
		
	// exit program if unable to create file
		
	if ( !sim ) // overloaded ! operator
	{
		cerr << "File sim.dat could not be opened" << endl;
		return -1;
	} // end if

	for (int z = 1; z <= simTimes; z++ ){
	contactG cg;
	list<int>::iterator it_1, it_2, it_3;
//	list< pair<int,double> >::iterator it;
	list< Quad >::iterator it;

	cg.config = &config;
//	int ext_no = 10;
	cg.genExt(ext_no);
	list<int>::iterator itE;
	predl << "eNodes:";
	for ( itE = cg.eNodes.begin() ; itE != cg.eNodes.end(); itE++ )
	predl << *itE << ", " ;
	predl << std::endl;

	list< Quad > M[3][ext_no][ext_no];	// a matrix to store similarities and time step
	list< Quad > merged[3], merged_start[3];	//merged version
	vector< Quad > mergedv[3];

	map<int,int> mymap;	//map for matrix index and node id
	int  val = 0;
	for ( it_1 = cg.eNodes.begin() ; it_1 != cg.eNodes.end(); it_1++ ){
		mymap.insert ( pair<int,int>(*it_1, val) );
		val++;
	}

	int t0 = 144000;	//starting time for constructing CG --> inf06
//	int t0 = 70000;		//starting time for inf05
 	int tend = 135;		//for conference setting on one day
	int T = 240;		//interval length
	int dephase = 132;	

	
//	MIT
// 	int t0 = 1093996800;	//start: 01-Sep-2004 and endtime = 1115263440 which is May-05 2005
// 	int tend = 10000;//70889;	//246 days
// 	int T = 300;		//interval length
// 	int dephase = 300;

		
// 	int t0 = 121;		//starting time for cambridge system lab
// 	int tend = 2176;	//no of steps that we are going to run our simulations for cambridge syslab

// 	int t0 = 121;		//starting time for intel lab
// 	int tend = 1497;	//no of steps that we are going to run our simulations for intel lab

// 	int t0 = 1130493332;		//starting time for cambridge city
// 	int tend = 1602;	//no of steps that we are going to run our simulations for cambridge city
// 	int T = 600;
// 	int dephase = 132;	

// 	int t0 = 1156083900;		//starting time for rollerbading tour
//  	int tend = 360;	//no of steps that we are going to run our simulations for rollerblading tour
//  	int T = 30;
// 	int dephase = 20;	//rollernet	

// 	int T = 240;		//interval length

	for (int k = 0; k <= tend; k++){
	//since we are intersted in the neighborhood set at a timestamp, method = 0
	cg.generateG(t0+k*T, t0+(k+1)*T);	//let's generate the graph

	std::cout << " no of nodes in known cg: " << cg.nodes_.size() << ", no of edges in known cg: " << cg._edges.size() << std::endl ;
	
	if(cg._edges.size() > 0)
	for ( it_1 = cg.eNodes.begin() ; it_1 != cg.eNodes.end(); it_1++ ){
	it_3 = it_1;
	it_3++;
	for ( it_2 = it_3 ; it_2 != cg.eNodes.end(); it_2++ ){
 		int i = mymap.find(*it_1)->second;
 		int j = mymap.find(*it_2)->second;

		Sim_type Nsim = cg.neighSim(*it_1,*it_2,t0+k*T, t0+(k+1)*T, sim, 0);	//neighbor similarity
// 		if(Nsim > 0.0) std::cout << "CNN(" << *it_1 << "," << *it_2 << ")=" << Nsim << std::endl;	
		//ncn
		if(Nsim.ncn_score == -1.0)	Nsim.ncn_score = 0.0;	//no common neighbor
		if(Nsim.ncn_score > 0.0){
			std::cout << "Sim(" << *it_1 << "," << *it_2 << ")=" << Nsim.ncn_score << std::endl;
			Quad q;
			q.id1 = *it_1;
			q.id2 = *it_2;
			q.time = k;
			q.score = Nsim.ncn_score;
// 			M[0][i][j].push_back( q );
// 			M[0][j][i].push_back( q );
			merged_start[0].push_back( q );
		}
		//jacard
		if(Nsim.jacard_score == -1.0)	Nsim.jacard_score = 0.0;	//no common neighbor
		if(Nsim.jacard_score > 0.0){
			std::cout << "Sim(" << *it_1 << "," << *it_2 << ")=" << Nsim.jacard_score << std::endl;
			Quad q;
			q.id1 = *it_1;
			q.id2 = *it_2;
			q.time = k;
			q.score = Nsim.jacard_score;
// 			M[1][i][j].push_back( q );
// 			M[1][j][i].push_back( q );
			merged_start[1].push_back( q );
		}
		//min score
		if(Nsim.min_score == -1.0)	Nsim.min_score = 0.0;	//no common neighbor
		if(Nsim.min_score > 0.0){
			std::cout << "Sim(" << *it_1 << "," << *it_2 << ")=" << Nsim.min_score << std::endl;
			Quad q;
			q.id1 = *it_1;
			q.id2 = *it_2;
			q.time = k;
			q.score = Nsim.min_score;
// 			M[2][i][j].push_back( q );
// 			M[2][j][i].push_back( q );
 			merged_start[2].push_back( q );
		}
	}
	}
	cg.clearCG();	//clear the previous CG for the next step
	}
/*std::cout << "non-sorted version:" << std::endl;
for(int i = 0; i < ext_no; i++)
for(int j = i+1; j < ext_no; j++)
for ( it = M[i][j].begin() ; it != M[i][j].end(); it++ )
	std::cout << "(" << (*it).id1 << "," << (*it).id2 << ")=(" << (*it).time << "," << (*it).score << ")" << std::endl;*/
//sort lists seperately
// for(int h = 0; h < 3; h++)
// for(int i = 0; i < ext_no; i++)
// for(int j = i+1; j < ext_no; j++){
// if(order == 0) M[h][i][j].sort(compare_pairs);
// else M[h][i][j].sort(compare_pairs_r);
// }
// // std::cout << "sorted version:" << std::endl;
// // for(int i = 0; i < ext_no; i++)
// // for(int j = i+1; j < ext_no; j++)
// // for ( it = M[i][j].begin() ; it != M[i][j].end(); it++ )
// // 	std::cout << "(" << (*it).id1 << "," << (*it).id2 << ")=(" << (*it).time << "," << (*it).score << ")" << std::endl;
// 
// std::cout << "merging all lists ..." << std::endl;
// for(int h = 0; h < 3; h++)
// for(int i = 0; i < ext_no; i++)
// for(int j = i+1; j < ext_no; j++)
// for ( it = M[h][i][j].begin() ; it != M[h][i][j].end(); it++ )
// merged_start[h].push_back(*it);

std::cout << "copy lists to vectors!" << std::endl;
for(int h = 0; h < 3; h++)
for ( it = merged_start[h].begin() ; it != merged_start[h].end(); it++ )
mergedv[h].push_back(*it);

std::cout << "randomly shuffle the vectors!" << std::endl;
for(int h = 0; h < 3; h++)
random_shuffle ( mergedv[h].begin(), mergedv[h].end() );

vector<Quad>::iterator it_v;

cout << "copy back vectors to final lists" << std::endl;
for(int h = 0; h < 3; h++)
for ( it_v = mergedv[h].begin() ; it_v != mergedv[h].end(); it_v++ )
    merged[h].push_back(*it_v);

std::cout << "sorting merged list ..." << std::endl;
predl << "sorted merged list:" << std::endl;

if(order == 0) for(int i = 0; i < 3; i++)  merged[i].sort(compare_pairs);
else for(int i = 0; i < 3; i++) merged[i].sort(compare_pairs_r);

//merged is the sorted version of all lists
// for ( it = merged.begin() ; it != merged.end(); it++ )
// 	predl << "(" << (*it).id1 << "," << (*it).id2 << ")=(" << (*it).time << "," << (*it).score << ")" << std::endl;

//our prediction method
int matchedP = 0;

//int dephase = 20;	//rollernet
predl << "Evaluating the merged list" << std::endl;

// cg.rContMap.clear();
// for (int state = 0; state < state_no; state++)
// for (int threshold = 20; threshold >= 0; threshold --){
// i = 0;
// matchedP = 0;
// //let's enumerate all states for all thresholds for this run
// for ( it = merged.begin() ; it != merged.end(); it++ ){
// if((*it).score == threshold){
// i++;
// if(cg.IsContact((*it).id1, (*it).id2, t0 + ((*it).time+state)*T - dephase, t0 + ((*it).time+1+state)*T + dephase)){
// 	predl << "(" << (*it).id1 << "," << (*it).id2 << ")=(" << (*it).time << "," << (*it).score << ") is matched!" << std::endl;
// 	matchedP++;
// 	}
// 	if((*it).score < threshold) break;
// }
// }
// if( i != 0){
// predl << "run # " << z << " prediction --> no of matched contacts for threshold= " << threshold << " is " << matchedP << ", out of " << i << ",state: " << state << std::endl;
// predl << "the no of real contacts which have been matched: " << cg.rContMap.size() << std::endl;
// prob[threshold][state] += (double)matchedP/(double)i;
// matched[threshold][state] += matchedP;	//no of matched for a specified threshold
// total[threshold][state] += i;	//total number of nodes with threshold no of common neighbis 
// }
// 
// cg.rContMap.clear();
// }

if(binf){
//finding the number of matched prediction out of the first 2^t
unsigned int limit = 1;
int j = 0;
matchedP = 0;
predl << "Evaluating the merged list, with different bins" << std::endl;
for(int i = 0; i < 3; i++){

if(i == 0) predl << "NCN results: " << std::endl;
else if(i == 1) predl << "Jac results: " << std::endl;
else predl << "MIN results: " << std::endl;

for (int t = 3; t <= 12; t++){
j = 0;
matchedP = 0;
limit = 1;
limit <<= t;
//let's evaluate the first 2^t scores
predl << "threshold 2^" << t << std::endl;

for ( it = merged[i].begin() ; it != merged[i].end(); it++ ){
if(j < limit){
j++;
if(cg.IsContact((*it).id1, (*it).id2, t0 + ((*it).time)*T - dephase, t0 + ((*it).time+1)*T + dephase)){
	predl << "(" << (*it).id1 << "," << (*it).id2 << ")=(" << (*it).time << "," << (*it).score << ") is matched!" << std::endl;
	matchedP++;
	}
}else break;
}
bins[i][t] += matchedP;	//let's count the number of matched
}
}
}

for(int i = 0; i < 3; i++)
std::cout << "merged list size: " << merged[i].size() << std::endl;
}

// for (int state = 0; state < state_no; state++){
// std::cout << "state: " << state << std::endl;
// for (int threshold = 20; threshold >= 0; threshold --)
// //if(prob[threshold][state] > 0.0) std::cout << "threshold :" << threshold << ", prob: " << prob[threshold][state] << std::endl;
// if(total[threshold][state] > 0) std::cout << "threshold :" << threshold << ", prob: " << (double)matched[threshold][state]/(double)total[threshold][state] << std::endl;
// }

if(binf)
for(int i = 0; i < 3; i++){

if(i == 0) std::cout << "NCN results: " << std::endl;
else if(i==1) std::cout << "Jac results: " << std::endl;
else std::cout << "MIN results: " << std::endl;

for (int t = 3; t <= 12; t++){
unsigned int limit = 1;
limit <<= t;
std::cout << "the number of matches out of the first " << limit << " score: " << ceil((double)bins[i][t]/(double)simTimes) << std::endl;
}
}

sim.close();
predl.close();

}//ncn, jac, and min methods analysis

//this part uses the product of nodes ranks to compute their contact probaility
if(0){
	//let's construct the contact graph with different threshold (no of contacts between two vertices) and then study the properties of the resulting graph
	int order = 0;  //0:descending, 1:ascending
	int ext_no = 0;
	int simTimes = 100;
	int bins[20], bins_rand[20];	//we want to find the first 2^t predictions
	int binf = 0;	//a flag for finding bins:0 disables finding bins

	for(int i = 0; i < 20; i++){
	bins[i] = 0;
	bins_rand[i] = 0;
	}

	if(argc != 5){
		std::cout << "usage: haggle ext_no time binf(0: wo binning, 1: w binning) order (0:descend, 1:ascend)" << std::endl;
		return -1;
	}else{
		
		ext_no = atoi(argv[1]);	//no of external nodes
		simTimes = atoi(argv[2]);
		binf = atoi(argv[3]);	//bin flag
 		order = atoi(argv[4]);  //ordering
		std::cout << "running the simulation for " << simTimes << " times with ext_no: " << ext_no << ", bins: " << binf << ", order: " << order << std::endl;
	}

	char _file [200];
	bzero(_file, 200);
	sprintf (_file, "/home/kazem/Desktop/traces/mobility/pred/predl_pop_%dext_%dt_%s.dat", ext_no, simTimes, config.db);
	ofstream predl( _file, ios::app );
		
	// exit program if unable to create file
		
	if ( !predl ) // overloaded ! operator
	{
		cerr << "File predl.dat could not be opened" << endl;
		return -1;
	} // end if

	bzero(_file, 200);
	sprintf (_file, "/home/kazem/Desktop/traces/mobility/pred/sim_pop_%dext_%dt_%s.dat", ext_no, simTimes, config.db);
	ofstream sim( _file, ios::app );
		
	// exit program if unable to create file
		
	if ( !sim ) // overloaded ! operator
	{
		cerr << "File sim.dat could not be opened" << endl;
		return -1;
	} // end if
	
	//infocom
	int t0 = 144000;	//starting time for constructing CG --> inf06
// 	int t0 = 70000;		//starting time for inf05
 	int tend = 135;		//for conference setting on one day
	int dephase = 132;
 	int T = 240;		//interval length
	int wLen = 480;

	//MIT
// 	int t0 = 1093996800;	//start: 01-Sep-2004 and endtime = 1115263440 which is May-05 2005
// 	int tend = 10000;//70889;	//246 days
// 	int T = 300;		//interval length
// 	int dephase = 300;
// 	int wLen = 600;

// 	int t0 = 121;		//starting time for cambridge system lab
// 	int tend = 2176;	//no of steps that we are going to run our simulations for cambridge syslab
// 	int T = 240;		//interval length

// 	int t0 = 121;		//starting time for intel lab
// 	int tend = 1497;	//no of steps that we are going to run our simulations for intel lab
// 	int T = 240;		//interval length

//  	int t0 = 1130493332;		//starting time for cambridge city
//  	int tend = 1602;	//no of steps that we are going to run our simulations for cambridge city
//  	int T = 600;
// 	int dephase = 600;
// 	int wLen = 600;		//we have chosen this number intutively

/*	int t0 = 1156083900;		//starting time for rollerbading tour
 	int tend = 360;	//no of steps that we are going to run our simulations for rollerblading tour
 	int T = 30;
	int dephase = 20;
	int wLen = 240;		//we have chosen this number intutively*/

	int ts = t0 - wLen;	//starting time for measuring the contact rate. note that the 1800 bias is to make sure we dont have the warming effect --> simulation starting time

	for (int z = 1; z <= simTimes; z++ ){
	int lUpdate = 0;	//this is for ranking updates
	contactG cg;
	list<int>::iterator it_1, it_2, it_3;
	list< Quad >::iterator it;

	cg.config = &config;
	cg.genExt(ext_no);
	list< Quad > merged, merged_start;	//a list to store different edges scores over time
	vector< Quad > mergedv;		//a vector to shuffle the scores

	list < Quad > rlist;	//random list

	for (int k = 0; k <= tend; k++){
	std::cout << "starting round # " << k << std::endl; 
	int current_time = t0 + k*T;

	cg.generateG(current_time - wLen, current_time);	//let's generate the graph

	std::cout << " no of nodes: " << cg.nodes_.size() << ", no of edges: " << cg._edges.size() << std::endl ;
	
	if(cg._edges.size() > 0){
	cg.updateRanks(ts, t0 + k*T, wLen, 2);	//moving window contacts counting

	//cg.updateRanks(t0, t0+k*T, wLen, 3);	//all contacts so far

	for ( it_1 = cg.eNodes.begin() ; it_1 != cg.eNodes.end(); it_1++ ){
	it_3 = it_1;
	it_3++;
	for ( it_2 = it_3 ; it_2 != cg.eNodes.end(); it_2++ ){
		//we only are interested in score of internal nodes wo sensors to predict their next moves
		double score = cg.rank(*it_1)*cg.rank(*it_2);	//the product score
//		std::cout << "(" << *it_1 << "," << *it_2 << ") --> (" <<   cg.rank(*it_1) << "," <<  cg.rank(*it_2) << ")" << std::endl; 
		if(score > 0){
			std::cout << "(" << *it_1 << "," << *it_2 << ")=" << score << std::endl;
			Quad q;
			q.id1 = *it_1;
			q.id2 = *it_2;
			q.time = k;
			q.score = score;
			merged_start.push_back( q );
		}
	}
	}
	}
	cg.clearCG();	//clear the previous CG for the next step
	}//for

std::cout << "copy list to vector!" << std::endl;
for ( it = merged_start.begin() ; it != merged_start.end(); it++ )
mergedv.push_back(*it);

std::cout << "randomly shuffle the vector!" << std::endl;
random_shuffle ( mergedv.begin(), mergedv.end() );

vector<Quad>::iterator it_v;

cout << "copy back vector to final list" << std::endl;
for ( it_v = mergedv.begin() ; it_v != mergedv.end(); it_v++ )
    merged.push_back(*it_v);

std::cout << "sorting merged list ..." << std::endl;
predl << "sorted merged list:" << std::endl;

//let's sort the list based on social similarity
if(order == 0) merged.sort(compare_pairs);
else merged.sort(compare_pairs_r);

//merged is the sorted version of all lists
// for ( it = merged.begin() ; it != merged.end(); it++ )
// 	predl << "(" << (*it).id1 << "," << (*it).id2 << ")=(" << (*it).time << "," << (*it).score << ")" << std::endl;


//our prediction method
int i = 0;
int matchedP = 0;


if(binf){
//finding the number of matched prediction out of the first 2^t
unsigned int limit = 1;
int j = 0;
matchedP = 0;
predl << "Evaluating the merged list, with different bins" << std::endl;

for (int t = 3; t <= 12; t++){
j = 0;
matchedP = 0;
limit = 1;
limit <<= t;
//let's evaluate the first 2^t scores
predl << "threshold 2^" << t << std::endl; 
for ( it = merged.begin() ; it != merged.end(); it++ ){
if(j < limit){
j++;
if(cg.IsContact((*it).id1, (*it).id2, t0 + ((*it).time)*T - dephase, t0 + ((*it).time+1)*T + dephase)){
	predl << "(" << (*it).id1 << "," << (*it).id2 << ")=(" << (*it).time << "," << (*it).score << ") is matched!" << std::endl;
	matchedP++;
	}
}else break;
}
bins[t] += matchedP;	//let's count the number of matched
}
}

std::cout << "merged list size: " << merged.size() << std::endl;
}

if(binf){
std::cout << "processing pred list" << std::endl;
for (int t = 3; t <= 12; t++){
unsigned int limit = 1;
limit <<= t;
std::cout << "the number of matches out of the first " << limit << " score: " << ceil((double)bins[t]/(double)simTimes) << std::endl;
}
}
predl.close();
sim.close();
}//popularity method analysis

//let's test the power of social profiles more closely
//we take the list of all nodes which have been sharing at least one node in common indicating that they are in close distance. then we want to see what will happen to the performance of prediction if we sort them according to their social distance or their product popularity compared to the random sorting!
if(0){
	//let's construct the contact graph with different threshold (no of contacts between two vertices) and then study the properties of the resulting graph
	int order = 0;  //0:descending, 1:ascending
	int ext_no = 0;
	int simTimes = 100;
	int bins[20];	//we want to find the first 2^t predictions
	int binf = 0;	//a flag for finding bins:0 disables finding bins
	double soc_dist[100][100];	//social distances
	double ncn = 0;	//ncn

	int wLen = 480;
	int ts = 144000 - wLen;	//starting time for measuring the contact rate. note that the 1800 bias is to make sure we dont have the warming effect --> simulation starting time

	for(int i = 0; i < 20; i++)
	bins[i] = 0;
	
	if(argc != 6){
		std::cout << "usage: haggle ext_no time binf(0: wo binning, 1: w binning) order (0:descend, 1:ascend, 2: random) ncn" << std::endl;
		return -1;
	}else{
		
		ext_no = atoi(argv[1]);	//no of external nodes
		simTimes = atoi(argv[2]);
		binf = atoi(argv[3]);	//bin flag
 		order = atoi(argv[4]);  //ordering
		ncn = atof(argv[5]);  //ordering
		std::cout << "running the simulation for " << simTimes << " times with ext_no: " << ext_no << ", bins: " << binf << ", order: " << order  << ", ncn: " << ncn << std::endl;
	}

	char _file [200];
	bzero(_file, 200);
	sprintf (_file, "/home/kazem/Desktop/traces/mobility/pred/predl_%dext_%dt_%s_ncn%d_foci.dat", ext_no, simTimes, config.db, (int) ncn);
	ofstream predl( _file, ios::app );
		
	// exit program if unable to create file
		
	if ( !predl ) // overloaded ! operator
	{
		cerr << "File predl.dat could not be opened" << endl;
		return -1;
	} // end if

	bzero(_file, 200);
	sprintf (_file, "/home/kazem/Desktop/traces/mobility/pred/sim_%dext_%dt_%s_foci.dat", ext_no, simTimes, config.db);
	ofstream sim( _file, ios::app );
		
	// exit program if unable to create file
		
	if ( !sim ) // overloaded ! operator
	{
		cerr << "File sim.dat could not be opened" << endl;
		return -1;
	} // end if

	for (int i = 0; i <= 99; i++)
	for (int j = 0; j <= 99; j++)
		soc_dist[i][j] = 0.0;
	//let's find the social distance among external nodes!
	if(order != 2)
	std::cout << "start computing social distances" << std::endl;
	for(int id1 = config.s_nid; id1 <= config.e_nid; id1++)
	for(int id2 = id1 + 1; id2 <= config.e_nid; id2++){
		char u1[ 10 ];	//user1
		char u2[ 10 ];	//user2
		bzero ( u1, 10 );
		bzero ( u2, 10 );
		sprintf ( u1,"%d", id1 );
		sprintf ( u2,"%d", id2 );
		distr dist_(&config);

		double ndistance = dist_.socialdist ( u1,u2, 127 );	//this is the actual distance
		double Sdist = 1.0/ndistance;	//rank based friendship based on Nowell and Kleinberg model

//		double Sdist = dist_.closeness ( u1,u2, 127 );	//Jacard index
 
		if(Sdist == -1.0)	Sdist = 0.0;	//no common neighbor
		soc_dist[id1 - config.s_nid][id2 - config.s_nid] = Sdist;
		soc_dist[id2 - config.s_nid][id1 - config.s_nid] = Sdist;
		//std::cout << "(" << id1 << "," << id2 << ") = " << Sdist << std::endl;
	}

	list<Tri> probs;
 
	for (int z = 1; z <= simTimes; z++ ){
	std::cout << "run # " << z << std::endl;

	list<int>::iterator it_1, it_2, it_3;
	contactG cg;
	cg.config = &config;
	list< Penta >::iterator it;
	cg.genExt(ext_no);
	list<int>::iterator itE;
	predl << "eNodes:";
	for ( itE = cg.eNodes.begin() ; itE != cg.eNodes.end(); itE++ )
	predl << *itE << ", " ;
	predl << std::endl;

	list< Penta > M[ext_no][ext_no];	// a matrix to store similarities and time step
//	list< Penta > merged, merged_start;	//a list to store different edges scores over time
	list< Penta > merged, merged_start[20];
	vector< Penta > mergedv;		//a vector to shuffle the scores

	map<int,int> mymap;	//map for matrix index and node id
	int  val = 0;
	for ( it_1 = cg.eNodes.begin() ; it_1 != cg.eNodes.end(); it_1++ ){
		mymap.insert ( pair<int,int>(*it_1, val) );
		val++;
	}

 	int t0 = 144000;	//starting time for constructing CG --> inf06
 	int tend = 135;		//for conference setting on one day
	
 	int T = 240;		//interval length

	for (int k = 0; k <= tend; k++){
	//since we are intersted in the neighborhood set at a timestamp, method = 0
	cg.generateG(t0+k*T, t0+(k+1)*T);	//let's generate the graph

	for ( it_1 = cg.eNodes.begin() ; it_1 != cg.eNodes.end(); it_1++ ){
	it_3 = it_1;
	it_3++;
	for ( it_2 = it_3 ; it_2 != cg.eNodes.end(); it_2++ ){
		int i = mymap.find(*it_1)->second;
		int j = mymap.find(*it_2)->second;

		Sim_type Nsim = cg.neighSim(*it_1,*it_2,t0+k*T, t0+(k+1)*T, sim, 1);	//neighbor similarity

		std::cout << "(" << *it_1 << "," << *it_2 << ") -->(" << i << "," << j << ")=" << Nsim.ncn_score << std::endl;
		if(Nsim.ncn_score == -1.0)	Nsim.ncn_score = 0.0;	//no common neighbor
// 		if(Nsim.ncn_score == ncn){
		if(Nsim.ncn_score > 0){
			//just filter those pairs which have at least one node in common
			Penta p;
			p.id1 = *it_1;
			p.id2 = *it_2;
			p.time = k;
			p.neigh_sim = Nsim.ncn_score;
			p.score = soc_dist[*it_1 - config.s_nid][*it_2 - config.s_nid];	//score is social similarity
//			merged_start.push_back( p );
			merged_start[(int) Nsim.ncn_score].push_back( p );	//store in different lists
		}
	}
	}
	cg.clearCG();	//clear the previous CG for the next step
	}

// //sort lists seperately
// for(int i = 0; i < ext_no; i++)
// for(int j = i+1; j < ext_no; j++){
// if(order == 0) M[i][j].sort(compare_pairs_penta);
// else M[i][j].sort(compare_pairs_penta_r);
// }

// std::cout << "merging all lists ..." << std::endl;
// for(int i = 0; i < ext_no; i++)
// for(int j = i+1; j < ext_no; j++)
// for ( it = M[i][j].begin() ; it != M[i][j].end(); it++ )
// merged.push_back(*it);

// std::cout << "copy list to vector!" << std::endl;
// for ( it = merged_start.begin() ; it != merged_start.end(); it++ )
// mergedv.push_back(*it);
// 
// std::cout << "randomly shuffle the vector!" << std::endl;
// random_shuffle ( mergedv.begin(), mergedv.end() );
// 
// vector<Penta>::iterator it_v;
// 
// cout << "copy back vector to final list" << std::endl;
// for ( it_v = mergedv.begin() ; it_v != mergedv.end(); it_v++ )
//     merged.push_back(*it_v);
// 
// std::cout << "sorting merged list ..." << std::endl;
// predl << "sorted merged list:" << std::endl;

//let's sort the list based on social similarity
// if(order == 0) merged.sort(compare_pairs_penta);
// else if(order == 1) merged.sort(compare_pairs_penta_r);
//sort every list based on social data
for(int i=0; i < 20; i++)
if(order == 0) merged_start[i].sort(compare_pairs_penta);
else if(order == 1) merged_start[i].sort(compare_pairs_penta_r);

cout << "merge all lists" << std::endl;
for(int i = 19; i >= 0; i--)
for ( it = merged_start[i].begin() ; it != merged_start[i].end(); it++ )
     merged.push_back(*it);

//merged is the sorted version of all lists
for ( it = merged.begin() ; it != merged.end(); it++ )
	predl << "(" << (*it).id1 << "," << (*it).id2 << ")=(" << (*it).time << "," << (*it).neigh_sim << "," << (*it).score<< ")" << std::endl;


//our prediction method
int i = 0;
int matchedP = 0;
int dephase = 132;

if(binf){
//finding the number of matched prediction out of the first 2^t
unsigned int limit = 1;
int j = 0;
matchedP = 0;
predl << "Evaluating the merged list, with different bins" << std::endl;

for (int t = 3; t <= 12; t++){
j = 0;
matchedP = 0;
limit = 1;
limit <<= t;
//let's evaluate the first 2^t scores
predl << "threshold 2^" << t << std::endl; 
for ( it = merged.begin() ; it != merged.end(); it++ ){
if(j < limit){
j++;
if(cg.IsContact((*it).id1, (*it).id2, t0 + ((*it).time)*T - dephase, t0 + ((*it).time+1)*T + dephase)){
	predl << "(" << (*it).id1 << "," << (*it).id2 << ")=(" << (*it).time << "," << (*it).score << "," << (*it).neigh_sim << ") is matched!" << std::endl;
	matchedP++;
	}else
	predl << "(" << (*it).id1 << "," << (*it).id2 << ")=(" << (*it).time << "," << (*it).score << "," << (*it).neigh_sim << ") is not matched!" << std::endl;
}else break;
}
bins[t] += matchedP;	//let's count the number of matched
}
}
/*
//let's record all matched pairs with the same ncn and different social similarity
for ( it = merged.begin() ; it != merged.end(); it++ ){
if(cg.IsContact((*it).id1, (*it).id2, t0 + ((*it).time)*T - dephase, t0 + ((*it).time+1)*T + dephase)){
	Tri tri;
	tri.match = 1;
	tri.score = (*it).score;
	tri.ncn = (*it).neigh_sim;
	probs.push_back( tri );
}else{
	Tri tri;
	tri.match = 0;
	tri.score = (*it).score;
	tri.ncn = (*it).neigh_sim;
	probs.push_back( tri );
}
}
*/

std::cout << "merged list size: " << merged.size() << std::endl;
}

if(binf)
for (int t = 3; t <= 12; t++){
unsigned int limit = 1;
limit <<= t;
std::cout << "the number of matches out of the first " << limit << " score: " << ceil((double)bins[t]/(double)simTimes) << std::endl;
}

sim.close();
predl.close();

int sim_[5][5];
int total[5][5];

for(int i = 0; i < 5; i++)
for(int j = 0; j < 5; j++){
sim_[i][j] = 0;
total[i][j] = 0;
}
/*
std::cout << "prob list size: " << probs.size() << std::endl;

list<Tri>::iterator it_4;
for ( it_4 = probs.begin() ; it_4 != probs.end(); it_4++ ){
	if((*it_4).ncn == 1.0){
			if((*it_4).score >= 0.0 && (*it_4).score <= 0.1){
				total[0][0]++;
				if((*it_4).match == 1)	sim_[0][0]++; 
			}else if((*it_4).score > 0.1 && (*it_4).score <= 0.2){
				total[0][1]++;
				if((*it_4).match == 1)	sim_[0][1]++; 
			}
			else if((*it_4).score > 0.2 && (*it_4).score <= 0.3){
				total[0][2]++;
				if((*it_4).match == 1)	sim_[0][2]++; 
			}
			else if((*it_4).score > 0.3 && (*it_4).score <= 0.4){
				total[0][3]++;
				if((*it_4).match == 1)	sim_[0][3]++; 
			}
			else if((*it_4).score > 0.4 && (*it_4).score <= 0.5){
				total[0][4]++;
				if((*it_4).match == 1)	sim_[0][4]++; 
			}
	}else if((*it_4).ncn == 2.0){
			if((*it_4).score >= 0.0 && (*it_4).score <= 0.1){
				total[1][0]++;
				if((*it_4).match == 1)	sim_[1][0]++; 
			}else if((*it_4).score > 0.1 && (*it_4).score <= 0.2){
				total[1][1]++;
				if((*it_4).match == 1)	sim_[1][1]++; 
			}
			else if((*it_4).score > 0.2 && (*it_4).score <= 0.3){
				total[1][2]++;
				if((*it_4).match == 1)	sim_[1][2]++; 
			}
			else if((*it_4).score > 0.3 && (*it_4).score <= 0.4){
				total[1][3]++;
				if((*it_4).match == 1)	sim_[1][3]++; 
			}
			else if((*it_4).score > 0.4 && (*it_4).score <= 0.5){
				total[1][4]++;
				if((*it_4).match == 1)	sim_[1][4]++; 
			}
	}else if((*it_4).ncn == 3.0){
			if((*it_4).score >= 0.0 && (*it_4).score <= 0.1){
				total[2][0]++;
				if((*it_4).match == 1)	sim_[2][0]++; 
			}else if((*it_4).score > 0.1 && (*it_4).score <= 0.2){
				total[2][1]++;
				if((*it_4).match == 1)	sim_[2][1]++; 
			}
			else if((*it_4).score > 0.2 && (*it_4).score <= 0.3){
				total[2][2]++;
				if((*it_4).match == 1)	sim_[2][2]++; 
			}
			else if((*it_4).score > 0.3 && (*it_4).score <= 0.4){
				total[2][3]++;
				if((*it_4).match == 1)	sim_[2][3]++; 
			}
			else if((*it_4).score > 0.4 && (*it_4).score <= 0.5){
				total[2][4]++;
				if((*it_4).match == 1)	sim_[2][4]++; 
			}
	}else if((*it_4).ncn == 4.0){
			if((*it_4).score >= 0.0 && (*it_4).score <= 0.1){
				total[3][0]++;
				if((*it_4).match == 1)	sim_[3][0]++; 
			}else if((*it_4).score > 0.1 && (*it_4).score <= 0.2){
				total[3][1]++;
				if((*it_4).match == 1)	sim_[3][1]++; 
			}
			else if((*it_4).score > 0.2 && (*it_4).score <= 0.3){
				total[3][2]++;
				if((*it_4).match == 1)	sim_[3][2]++; 
			}
			else if((*it_4).score > 0.3 && (*it_4).score <= 0.4){
				total[3][3]++;
				if((*it_4).match == 1)	sim_[3][3]++; 
			}
			else if((*it_4).score > 0.4 && (*it_4).score <= 0.5){
				total[3][4]++;
				if((*it_4).match == 1)	sim_[3][4]++; 
			}
	}else if((*it_4).ncn == 5.0){
			std::cout << "ncn: " << (*it_4).ncn << ", score: " << (*it_4).score << ", match: " << (*it_4).match << std::endl;
			if((*it_4).score >= 0.0 && (*it_4).score <= 0.1){
				total[4][0]++;
				if((*it_4).match == 1)	sim_[4][0]++;
			}else if((*it_4).score > 0.1 && (*it_4).score <= 0.2){
				total[4][1]++;
				if((*it_4).match == 1)	sim_[4][1]++; 
			}
			else if((*it_4).score > 0.2 && (*it_4).score <= 0.3){
				total[4][2]++;
				if((*it_4).match == 1)	sim_[4][2]++; 
			}
			else if((*it_4).score > 0.3 && (*it_4).score <= 0.4){
				total[4][3]++;
				if((*it_4).match == 1)	sim_[4][3]++; 
			}
			else if((*it_4).score > 0.4 && (*it_4).score <= 0.5){
				total[4][4]++;
				if((*it_4).match == 1)	sim_[4][4]++; 
			}
	}//switch
}//for
*/

for(int i = 0; i < 5; i++){
std::cout << "for ncn=" << i + 1 << ", probs are: " << std:: endl;
for(int j = 0; j < 5; j++){
// std::cout << "sim: " << (double)sim_[i][j] << ", total: " << (double)total[i][j] << " "; 
std::cout << (double)sim_[i][j]/(double)total[i][j] << " "; 
}
std::cout << std::endl;
}
}

//this part mix the product of nodes ranks and NCN to compute their contact probaility
if(0){
	//let's construct the contact graph with different threshold (no of contacts between two vertices) and then study the properties of the resulting graph
	int order = 0;  //0:descending, 1:ascending
	int ext_no = 0;
	int simTimes = 100;
	int bins[20], bins_rand[20];	//we want to find the first 2^t predictions
	int binf = 0;	//a flag for finding bins:0 disables finding bins
	int rankf = 0;

	for(int i = 0; i < 20; i++){
	bins[i] = 0;
	bins_rand[i] = 0;
	}

	if(argc != 6){
		std::cout << "usage: haggle ext_no time binf(0: wo binning, 1: w binning) order (0:descend, 1:ascend) rankf (0: no rank, 1: rank)" << std::endl;
		return -1;
	}else{
		
		ext_no = atoi(argv[1]);	//no of external nodes
		simTimes = atoi(argv[2]);
		binf = atoi(argv[3]);	//bin flag
 		order = atoi(argv[4]);  //ordering
		rankf = atoi(argv[5]);	//rank flag
		std::cout << "running the simulation for " << simTimes << " times with ext_no: " << ext_no << ", bins: " << binf << ", order: " << order << ", rank flag: " << rankf << std::endl;
	}

	char _file [200];
	bzero(_file, 200);
	sprintf (_file, "/home/kazem/Desktop/traces/mobility/pred/predl_%dext_%dt_%s_pop_ncn.dat", ext_no, simTimes, config.db);
	ofstream predl( _file, ios::app );
		
	// exit program if unable to create file
		
	if ( !predl ) // overloaded ! operator
	{
		cerr << "File predl.dat could not be opened" << endl;
		return -1;
	} // end if

	bzero(_file, 200);
	sprintf (_file, "/home/kazem/Desktop/traces/mobility/pred/sim_%dext_%dt_%s_pop_ncn.dat", ext_no, simTimes, config.db);
	ofstream sim( _file, ios::app );
		
	// exit program if unable to create file
		
	if ( !sim ) // overloaded ! operator
	{
		cerr << "File sim.dat could not be opened" << endl;
		return -1;
	} // end if
	
	//infocom
	int t0 = 144000;	//starting time for constructing CG --> inf06
// 	int t0 = 70000;		//starting time for inf05
 	int tend = 135;		//for conference setting on one day
	int dephase = 132;
 	int T = 240;		//interval length
	int wLen = 480;

	//MIT
// 	int t0 = 1093996800;	//start: 01-Sep-2004 and endtime = 1115263440 which is May-05 2005
// 	int tend = 10000;//70889;	//246 days
// 	int T = 300;		//interval length
// 	int dephase = 300;
// 	int wLen = 600;

// 	int t0 = 121;		//starting time for cambridge system lab
// 	int tend = 2176;	//no of steps that we are going to run our simulations for cambridge syslab
// 	int T = 240;		//interval length

// 	int t0 = 121;		//starting time for intel lab
// 	int tend = 1497;	//no of steps that we are going to run our simulations for intel lab
// 	int T = 240;		//interval length

//  	int t0 = 1130493332;		//starting time for cambridge city
//  	int tend = 1602;	//no of steps that we are going to run our simulations for cambridge city
//  	int T = 600;
// 	int dephase = 600;
// 	int wLen = 600;		//we have chosen this number intutively

/*	int t0 = 1156083900;		//starting time for rollerbading tour
 	int tend = 360;	//no of steps that we are going to run our simulations for rollerblading tour
 	int T = 30;
	int dephase = 20;
	int wLen = 240;		//we have chosen this number intutively*/

	int ts = t0 - wLen;	//starting time for measuring the contact rate. note that the 1800 bias is to make sure we dont have the warming effect --> simulation starting time

	for (int z = 1; z <= simTimes; z++ ){
	int lUpdate = 0;	//this is for ranking updates
	contactG cg;
	list<int>::iterator it_1, it_2, it_3;
	list< Penta >::iterator it;

	cg.config = &config;
	cg.genExt(ext_no);
	list< Penta > merged, merged_start[20];	//a list to store different edges scores over time
	vector< Penta > mergedv;		//a vector to shuffle the scores

	for (int k = 0; k <= tend; k++){
	std::cout << "starting round # " << k << std::endl; 
	int current_time = t0 + k*T;

	cg.generateG(current_time - wLen, current_time);	//let's generate the graph

	std::cout << " no of nodes: " << cg.nodes_.size() << ", no of edges: " << cg._edges.size() << std::endl ;
	
	if(cg._edges.size() > 0){
	cg.updateRanks(ts, t0 + k*T, wLen, 2);	//moving window contacts counting

	//cg.updateRanks(t0, t0+k*T, wLen, 3);	//all contacts so far

	for ( it_1 = cg.eNodes.begin() ; it_1 != cg.eNodes.end(); it_1++ ){
	it_3 = it_1;
	it_3++;
	for ( it_2 = it_3 ; it_2 != cg.eNodes.end(); it_2++ ){
		//we only are interested in score of internal nodes wo sensors to predict their next moves
		double score;
		if(rankf) score = cg.rank(*it_1)*cg.rank(*it_2);	//the product score
//		std::cout << "(" << *it_1 << "," << *it_2 << ") --> (" <<   cg.rank(*it_1) << "," <<  cg.rank(*it_2) << ")" << std::endl; 
		Sim_type Nsim = cg.neighSim(*it_1,*it_2,t0+k*T, t0+(k+1)*T, sim, 0);	//neighbor similarity
// 		if(Nsim > 0.0) std::cout << "CNN(" << *it_1 << "," << *it_2 << ")=" << Nsim << std::endl;
		if(Nsim.ncn_score == -1.0)	Nsim.ncn_score = 0.0;	//no common neighbor

		if(Nsim.ncn_score == 1.0){
			std::cout << "(" << *it_1 << "," << *it_2 << ")=(" << score << "," << Nsim.ncn_score << ")" << std::endl;
			Penta p;
			p.id1 = *it_1;
			p.id2 = *it_2;
			p.time = k;
			if(rankf) p.score = score;
			else p.score = Nsim.ncn_score;
			p.neigh_sim = Nsim.ncn_score;
			merged_start[(int) Nsim.ncn_score].push_back( p );	//store in different lists
		}
	}
	}
	}
	cg.clearCG();	//clear the previous CG for the next step
	}//for

std::cout << "sorting merged list ..." << std::endl;
predl << "sorted merged list:" << std::endl;

//sort every list based on social data
for(int i=0; i < 20; i++)
if(order == 0) merged_start[i].sort(compare_pairs_penta);
else if(order == 1) merged_start[i].sort(compare_pairs_penta_r);

cout << "merge all lists" << std::endl;
for(int i = 19; i >= 0; i--)
for ( it = merged_start[i].begin() ; it != merged_start[i].end(); it++ )
     merged.push_back(*it);

//merged is the sorted version of all lists
for ( it = merged.begin() ; it != merged.end(); it++ )
	predl << "(" << (*it).id1 << "," << (*it).id2 << ")=(" << (*it).time << "," << (*it).neigh_sim << "," << (*it).score<< ")" << std::endl;


//merged is the sorted version of all lists
// for ( it = merged.begin() ; it != merged.end(); it++ )
// 	predl << "(" << (*it).id1 << "," << (*it).id2 << ")=(" << (*it).time << "," << (*it).score << ")" << std::endl;


//our prediction method
int i = 0;
int matchedP = 0;


if(binf){
//finding the number of matched prediction out of the first 2^t
unsigned int limit = 1;
int j = 0;
matchedP = 0;
predl << "Evaluating the merged list, with different bins" << std::endl;

for (int t = 3; t <= 12; t++){
j = 0;
matchedP = 0;
limit = 1;
limit <<= t;
//let's evaluate the first 2^t scores
predl << "threshold 2^" << t << std::endl; 
for ( it = merged.begin() ; it != merged.end(); it++ ){
if(j < limit){
j++;
if(cg.IsContact((*it).id1, (*it).id2, t0 + ((*it).time)*T - dephase, t0 + ((*it).time+1)*T + dephase)){
	predl << "(" << (*it).id1 << "," << (*it).id2 << ")=(" << (*it).time << "," << (*it).score << ") is matched!" << std::endl;
	matchedP++;
	}
}else break;
}
bins[t] += matchedP;	//let's count the number of matched
}
}

std::cout << "merged list size: " << merged.size() << std::endl;
}

if(binf){
std::cout << "processing pred list" << std::endl;
for (int t = 3; t <= 12; t++){
unsigned int limit = 1;
limit <<= t;
std::cout << "the number of matches out of the first " << limit << " score: " << ceil((double)bins[t]/(double)simTimes) << std::endl;
}
}
predl.close();
sim.close();
}

 return EXIT_SUCCESS;
}
