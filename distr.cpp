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
#include "parser.h"
#include "distr.h"
#include "rgmodel.h"
#include <math.h>
#include "common.h"

using namespace std;

struct ltstr
{
  bool operator()(const char* s1, const char* s2) const
  {
    //return strcmp(s1, s2) < 0;
    int a_ = atoi(s1);
    int b_ = atoi(s2);
    return (a_ - b_) < 0;
  }
};

distr::distr(configuration *cfg){
	//let's setup the configuration for contactG class
	config = cfg;
	cg.config = cfg;
}

int distr::contactdist(){
	mysqlpp::Connection conn(false);

	/* Connect to database */
	if (conn.connect("infocom06", "localhost", "root", "mysql")) {
		// Retrieve a subset of the sample stock table set up by resetdb
		// and display it
//		mysqlpp::Query query = conn.query("select starttime, endtime from contactspan where user1=31");
		mysqlpp::Query query = conn.query("select starttime, endtime from contactspan");
		if (mysqlpp::StoreQueryResult res = query.store()) {
			cout << "We have:" << endl;
			int max = 0;
			map<int, int>::iterator ii;
			map<int, int> contd;
			for (size_t i = 0; i < res.num_rows(); ++i) {
				//going through db and finding contact duration time for each contact and store them inside map table
				int stime = res[i][0];	//starting time
				int etime = res[i][1];	//end time
				int cont = etime - stime;	//contact time
				
				
				//map<const char*, int, ltstr> months;
					//a map for storing contact distribution
				cout << '\t' << cont << endl;

				ii = contd.find(cont);

				if((*ii).first != cont){
					cout << '\t' << "inside" << cont << endl;
					contd.insert(std::make_pair(cont, 1));
				}
				else (*ii).second++;

				//cout << (*ii).first << ":" << (*ii).second  << endl;

				if (max < cont) max = cont;
				
			}
	int contact_no = 0;	//no of contacts			
	for( map<int, int>::iterator ii=contd.begin(); ii!=contd.end(); ++ii)
	{
		cout << (*ii).first << " " << (*ii).second << ";";
		contact_no += (*ii).second;
		
	}
	cout << '\t' << "size of map table: " << contd.size() << endl;
	cout << '\t' << "max contact duration: " << max << endl;
	cout << '\t' << "no of contacts: " << contact_no << endl;


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

/*
The following method finds the bin which represents the inter-contact distribution of all nodes.
Note that the bin#i corresponds to all inter-contac times between 2min*i < t < 2min*(i+1)
*/
int distr::intercontactdist(){
	mysqlpp::Connection conn(false);

	/* Connect to database */
	if (conn.connect("infocom06", "localhost", "root", "mysql")) {
		// Retrieve a subset of the sample stock table set up by resetdb
		// and display it

		mysqlpp::Query query = conn.query("select intercontact from contactspan where (endtime-starttime) > 100 and user1 <= 99 and user1 >= 21 and user2 <= 99 and user2 >= 21 and starttime>57600 and starttime < 57600+10.5*3600");
		if (mysqlpp::StoreQueryResult res = query.store()) {
			cout << "We have:" << endl;
			int max = 0;
			map<double, int>::iterator ii;
			map<double, int> inter_contd;
			for (size_t i = 0; i < res.num_rows(); ++i) {
				//going through db and finding contact duration time for each contact and store them inside map table
				double inter_cont = res[i][0];	//inter contact time
				
				//map<const char*, int, ltstr> months;
					//a map for storing inter-contact distribution
				double value = (inter_cont - fmod(inter_cont, 120.0))/120.0;
				cout << inter_cont << endl;
				cout << value << endl;

				ii = inter_contd.find(value);

				if((*ii).first != value){
//					cout << '\t' << "inside" << inter_cont << endl;
					inter_contd.insert(std::make_pair(value, 1));
				}
				else (*ii).second++;

				//cout << (*ii).first << ":" << (*ii).second  << endl;

				if (max < inter_cont) max = inter_cont;
				
			}
			
			for( map<double, int>::iterator ii=inter_contd.begin(); ii!=inter_contd.end(); ++ii)
			{
				cout << (*ii).first << " " << (*ii).second << ";";
				
			}
			cout << '\t' << "size of map table: " << inter_contd.size() << endl;
			cout << '\t' << "max inter_contact duration: " << max << endl;

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

/*
The following method finds the inter-contact distribution of the given pair nodes.
Note that the bin#i corresponds to all inter-contac times between 2min*i < t < 2min*(i+1)
*/
int distr::intercontactdist(char *id1, char *id2){
	mysqlpp::Connection conn(false);

	/* Connect to database */
	if (conn.connect("infocom06", "localhost", "root", "mysql")) {
		// Retrieve a subset of the sample stock table set up by resetdb
		// and display it
		char _query [400];
		bzero(_query, 400);
		sprintf (_query, "select intercontact from contactspan where (endtime-starttime) > 100 and user1=%s and user2=%s and starttime > 57600 and starttime < 57600+10.5*3600", id1, id2);

		mysqlpp::Query query = conn.query(_query);
		if (mysqlpp::StoreQueryResult res = query.store()) {
			cout << "We have:" << endl;
			int max = 0;
			map<double, int>::iterator ii;
			map<double, int> inter_contd;
			for (size_t i = 0; i < res.num_rows(); ++i) {
				//going through db and finding contact duration time for each contact and store them inside map table
				double inter_cont = res[i][0];	//inter contact time
				
				//map<const char*, int, ltstr> months;
					//a map for storing inter-contact distribution
				double value = (inter_cont - fmod(inter_cont, 120.0))/120.0;
				cout << inter_cont << endl;
				cout << value << endl;

				ii = inter_contd.find(value);

				if((*ii).first != value){
//					cout << '\t' << "inside" << inter_cont << endl;
					inter_contd.insert(std::make_pair(value, 1));
				}
				else (*ii).second++;

				//cout << (*ii).first << ":" << (*ii).second  << endl;

				if (max < inter_cont) max = inter_cont;
				
			}
			
			for( map<double, int>::iterator ii=inter_contd.begin(); ii!=inter_contd.end(); ++ii)
			{
				cout << (*ii).first << " " << (*ii).second << ";";
				
			}
			cout << '\t' << "size of map table: " << inter_contd.size() << endl;
			cout << '\t' << "max inter_contact duration: " << max << endl;

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

/*
The following method finds the inter-contact distribution of the given node with the rest of system nodes.
Note that the bin#i corresponds to all inter-contac times between 2min*i < t < 2min*(i+1)
*/
int distr::intercontactdist(char *id1){
	mysqlpp::Connection conn(false);

	/* Connect to database */
	if (conn.connect("infocom06", "localhost", "root", "mysql")) {
		// Retrieve a subset of the sample stock table set up by resetdb
		// and display it
		char _query [400];
		bzero(_query, 400);
		sprintf (_query, "select intercontact from contactspan where (endtime-starttime) > 100 and user1=%s and user2 <= 99 and user2 >= 21 and starttime>57600 and starttime < 57600+10.5*3600", id1);

		mysqlpp::Query query = conn.query(_query);
		if (mysqlpp::StoreQueryResult res = query.store()) {
			cout << "We have:" << endl;
			int max = 0;
			map<double, int>::iterator ii;
			map<double, int> inter_contd;
			for (size_t i = 0; i < res.num_rows(); ++i) {
				//going through db and finding contact duration time for each contact and store them inside map table
				double inter_cont = res[i][0];	//inter contact time
				
				//map<const char*, int, ltstr> months;
					//a map for storing inter-contact distribution
				double value = (inter_cont - fmod(inter_cont, 120.0))/120.0;
				cout << inter_cont << endl;
				cout << value << endl;

				ii = inter_contd.find(value);

				if((*ii).first != value){
//					cout << '\t' << "inside" << inter_cont << endl;
					inter_contd.insert(std::make_pair(value, 1));
				}
				else (*ii).second++;

				//cout << (*ii).first << ":" << (*ii).second  << endl;

				if (max < inter_cont) max = inter_cont;
				
			}
			
			for( map<double, int>::iterator ii=inter_contd.begin(); ii!=inter_contd.end(); ++ii)
			{
				cout << (*ii).first << " " << (*ii).second << ";";
				
			}
			cout << '\t' << "size of map table: " << inter_contd.size() << endl;
			cout << '\t' << "max inter_contact duration: " << max << endl;

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
//method return value: no of contacts between id1 and id2
int distr::no_of_contacts(char *id1, char *id2){
	mysqlpp::Connection conn(false);

	/* Connect to database */
	if (conn.connect(config->db, "localhost", "root", "mysql")) {
		// Retrieve a subset of the sample stock table set up by resetdb
		// and display it
		//cout << "Connected to db!" << endl;
		
		char _query [200];
		bzero(_query, 200);
		//here there is a missing point: what about the contact which have been logged by id2
		sprintf (_query, "select count(*) from contactspan where user1=%s and user2=%s", id1, id2);
		//printf ("the query: %s\n", _query);

		mysqlpp::Query query = conn.query(_query);
		if (mysqlpp::StoreQueryResult res = query.store()) {
			
			//cout << '\t' << "no of occurence: " << res[0][0] << endl;
			return res[0][0];
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

//method return value: no of contacts between id1 and id2 in [t0,t1]
//method arg: 0 --> menas we want to only collect the neighborhood set at time t0=t1
//=1 means that we are couting all contacts happened between id1 and id2 for a long period interval --> useful for aggregation type
//=2 --> interval sampling for sampling the contact graphs
int distr::no_of_contacts(char *id1, char *id2, int t0, int t1, int method){
	mysqlpp::Connection conn(false);

	/* Connect to database */
	if (conn.connect(config->db, "localhost", "root", "mysql")) {
		// Retrieve a subset of the sample stock table set up by resetdb
		// and display it
		//cout << "Connected to db!" << endl;
		
		char _query [200];
		bzero(_query, 200);

		if(method == 0)
		//sampling time method
		sprintf (_query, "select count(*) from %s where user1=%s and user2=%s and starttime <= %d and %d <= endtime", config->table, id1, id2, t0, t1);
		else if(method == 1)
		//here there is a missing point: what about the contact which have been logged by id2
		sprintf (_query, "select count(*) from %s where user1=%s and user2=%s and starttime >= %d and endtime <= %d", config->table, id1, id2, t0, t1);
		//printf ("the query: %s\n", _query);
		else
		//sampling interval method
		sprintf (_query, "select count(*) from %s where user1=%s and user2=%s and ((starttime >= %d and starttime <= %d) or (endtime >= %d and endtime <= %d))", config->table, id1, id2, t0, t1, t0, t1);

//cout << _query << endl;
		mysqlpp::Query query = conn.query(_query);
		if (mysqlpp::StoreQueryResult res = query.store()) {
			
			//cout << '\t' << "no of occurence: " << res[0][0] << endl;
			return res[0][0];
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

//method return value: no of contacts between id1 and id2 in [tstart,tend]
//this method differs from no_of_contacts because it tries to solve the mutual sighting issue
//if tstar=tend=-1: means that we are looking at all contacts
int distr::real_no_of_contacts(char *id1, char *id2, int tstart, int tend){
	int ts, te, ts_, te_, t1, t2, t1_, t2_;
	int flag1_ = 0, flag2_ = 0, no_cont = 0, cont_dur = 0, cntf = 1;//cntf is a flag to cheskc if we have counted the last contact or not!
	//we set flag1_ and flag2_ when we reach to the end of list1 and list2 respectively

/*   ofstream outFile( "/home/kazem/Desktop/traces/log.dat", ios::out );
  if ( !outFile ) // overloaded ! operator
  {
    cerr << "File log.dat could not be opened" << endl;
    return -1;
  } // end if
*/
	mysqlpp::Connection conn(false);

	// Connect to database 
	if (conn.connect("infocom06", "localhost", "root", "mysql")) {
		// Retrieve a subset of the sample stock table set up by resetdb
		// and display it
		//cout << "Connected to db!" << endl;
		char _query1 [200];
		char _query2 [200];

		if(tstart != -1 && tend != -1){		
			bzero(_query1, 200);
			sprintf (_query1, "select starttime, endtime from contactspan where user1=%s and user2=%s and starttime > %d and endtime < %d order by starttime", id1, id2, tstart, tend);
	//		printf ("the query_1: %s\n", _query1);
	
			sprintf (_query2, "select starttime, endtime from contactspan where user1=%s and user2=%s and starttime > %d and endtime < %d order by starttime", id2, id1, tstart, tend);
	//		printf ("the query_2: %s\n", _query2);
		}else{
			//all contacts
			bzero(_query1, 200);
			sprintf (_query1, "select starttime, endtime from contactspan where user1=%s and user2=%s order by starttime", id1, id2);
	//		printf ("the query_1: %s\n", _query1);
	
			bzero(_query2, 200);
			sprintf (_query2, "select starttime, endtime from contactspan where user1=%s and user2=%s order by starttime", id2, id1);
		}


	mysqlpp::Query query_1 = conn.query(_query1);
	mysqlpp::Query query_2 = conn.query(_query2);

	if (mysqlpp::StoreQueryResult res_1 = query_1.store()){
	if(mysqlpp::StoreQueryResult res_2 = query_2.store()) {
		
		
		size_t i = 0;
		size_t j = 0;
//		cout << "inside the loop" << ", list 1 size: " << res_1.num_rows() << endl;
//		cout << "inside the loop" << ", list 2 size: " << res_2.num_rows() << endl;

		if(i < res_1.num_rows()){
		//tstat and tend from the first list
		t1 = atoi(res_1[i][0]);
		t2 = atoi(res_1[i][1]);
		} else flag1_ = 1;

		if(j < res_2.num_rows()){
		//tstat and tend from the second list
		t1_ = atoi(res_2[j][0]);
		t2_ = atoi(res_2[j][1]);
		} else	flag2_ = 1;	//no item in l2

		//which one is the under study slot?
		if(!flag1_ && !flag2_){
			//both l1 and l2 have items
			if (t1 < t1_ || !flag2_){
				//read from l1
				cntf = 0;
				ts = t1;
				te = t2;
				i++;
			}else if(t1_ < t1 || !flag1_){
				cntf = 0;
				ts = t1_;
				te = t2_;
				j++;
			}
		}else if(!flag1_)
		{
			//read from l1
			cntf = 0;
			ts = t1;
			te = t2;
			i++;
		}else if(!flag2_)
		{
			//read from l2
			cntf = 0;
			ts = t1_;
			te = t2_;
			j++;
		}
	
//		cout << "1: ts: " << ts << ", te: " << te << ", (i,j): " << i << " " << j << endl;

		while(1){
		//consider exceptions
		if(res_1.num_rows() == 0 && res_2.num_rows() == 0){
//			cout << "exception #0! " << cntf << endl;
			break;
		}
		//tstat and tend from the first list
		if(res_1.num_rows() == 1 && res_2.num_rows() == 0){
//			cout << "exception #1!" << endl;
			break;
		}

		if(res_1.num_rows() == 0 && res_2.num_rows() == 1){
//			cout << "exception #2!" << endl;
			break;
		}


		if(i < res_1.num_rows() && !flag1_){
			t1 = atoi(res_1[i][0]);
			t2 = atoi(res_1[i][1]);
		}else flag1_ = 1;

		//tstat and tend from the second list
		if(j < res_2.num_rows() && !flag2_){
			t1_ = atoi(res_2[j][0]);
			t2_ = atoi(res_2[j][1]);
		}else	flag2_ = 1;
		
		if(cntf){
			//let's find the ts and te
			//which one is the under study slot?
			if(!flag1_ && !flag2_){
				//when both list still have data to read
				if (t1 < t1_){
					//which one is the under study slot?
					if (t1 < ts_){
						//update ts and te: read from l1
						ts = t1;
						te = t2;
					}else{
						//update ts and te from l2
						ts = ts_;
						te = te_;
	
						ts_ = t1;
						te_ = t2;
					}	
	
					cntf = 0;
					i++;	//read list 1
				}else{
					//which one is the under study slot?
					if (t1_ < ts_){
						//update ts and te
						ts = t1_;
						te = t2_;
					}else{
						ts = ts_;
						te = te_;
	
						ts_ = t1_;
						te_ = t2_;
					}	
	
					cntf = 0;
					j++;	//read list 2
				}
			}
			else if(!flag1_){		
				//we should read from l1 only
				ts = ts_;
				te = te_;
				ts_ = t1;
				te_ = t2;
				i++;
				cntf = 0;
			}
			else if(!flag2_){
				//we should read from l2 only
				ts = ts_;
				te = te_;
				ts_ = t1_;
				te_ = t2_;
				j++;
				cntf = 0;
			}

		}else{
			if(!flag1_ && !flag2_){
				//bot list have data to read
				//which one is the under study slot?
				//let's find ts_ and te_
				if (t1 < t1_){
					//read list 1
					ts_ = t1;
					te_ = t2;
					i++;
				}else{
					//read list 2
					ts_ = t1_;
					te_ = t2_;
					j++;
				}
			}
			else if(!flag1_){		
				//we should read from l1 only
				ts_ = t1;
				te_ = t2;
				i++;
			}
			else if(!flag2_){
				//we should read from l2 only
				ts_ = t1_;
				te_ = t2_;
				j++;
			}
		}
//				cout << "5: ts: " << ts << ", te: " << te << ", (i,j): " << i << " " << j << ", ts_: " << ts_ << ",te_: " << te_ << endl;


		//let's start merging
		if(te < ts_){
			//no overlap
			if((te -ts) > 0){
				//let's only cont the non-zero contacts
				no_cont++;
				cont_dur += te - ts;
			}
//			outFile << "6 merge: ts: " << ts << ", te: " << te << " no overlap" << ", (i,j): " << i << " " << j << ", cont no: " << no_cont << ", duration: " << cont_dur << endl;

//			cout << "6 merge: ts: " << ts << ", te: " << te << " no overlap" << ", (i,j): " << i << " " << j << ", cont no: " << no_cont << ", duration: " << cont_dur << endl;
			ts = ts_;
			te = te_;
			cntf = 1;	//we have counted this contact: (ts,te)
// 			cout << "6 merge: ts: " << ts << ", te: " << te << " no overlap" << ", (i,j): " << i << " " << j << ", cont no: " << no_cont << ", duration: " << cont_dur << endl;

		}else if (te < te_){
			te = te_;
//			cout << "7 merge ts: " << ts << ", te: " << te << " te < te_, read from l1 " << ", (i,j): " << i << " " << j << endl;

		}
/*
else{		
	cout << "8 merge ts: " << ts << ", te: " << te << " the last else " << ", (i,j): " << i << " " << j << ", read from l2" << endl;	
		}*/
		
		if ((i == res_1.num_rows()) && (j == res_2.num_rows())){
			//count the very last slot
			no_cont++;
			cont_dur += te - ts;
			cntf = 1;
			break;
		}
		}		

	}
	else {
		cerr << "Failed to get item list: " << query_2.error() << endl;
		return 1;
	}
	}
	else {
		cerr << "Failed to get item list: " << query_1.error() << endl;
		return 1;
	}

	}
	else {
		cerr << "DB connection failed: " << conn.error() << endl;
		return 1;
	}
//	cout << no_cont << " " << cont_dur << endl;
	if (cntf == 0){
		if((te - ts) > 0){
			//let's count non-zero contacts
			no_cont++;	//we have not counted the last contact
			cont_dur += (te-ts);	//we have not counted the last contact
		}
	}
/*	cout << "no cont: " << no_cont << endl;
	cout << "cont duration: " << cont_dur << endl;*/
	if(!contact.empty()) contact.clear();
	contact.insert(std::make_pair(no_cont, cont_dur));
}

//method return value: total no of contacts which belong to id1 in [t0,t1]
//uniqF: if 1: means that we are only interested in the total unique contacts of node id1 otherwise the total contact rates inclusing the duplicates!
int distr::no_of_contacts(char *id1, int t0, int t1, int uniqF){
	mysqlpp::Connection conn(false);

	/* Connect to database */
	if (conn.connect(config->db, "localhost", "root", "mysql")) {
		// Retrieve a subset of the sample stock table set up by resetdb
		// and display it
		//cout << "Connected to db!" << endl;
		char _query [400];
		if(uniqF == 0){
			//count the total contact rate
			bzero(_query, 400);
			//we have to resolve the issue of mutual sighting
			//here we count all contacts including internal and external contacts
			sprintf (_query, "select count(*) from contactspan where user1=%s and user2 >=%d and user2 <=%d and starttime > %d and endtime < %d", id1, config->s_nid, config->e_nid, t0, t1);

			mysqlpp::Query query = conn.query(_query);
			if (mysqlpp::StoreQueryResult res = query.store()) {	
				return res[0][0];
			}
			else {
				cerr << "Failed to get item list: " << query.error() << endl;
				return 1;
			}

		}else{
			//count only unique contacts
			bzero(_query, 400);
			sprintf (_query, "select distinct user2 from contactspan where user2 >= 21 and starttime > %d and starttime < %d and user1 = %s;", t0, t1, id1);

			mysqlpp::Query query = conn.query(_query);
	
			if (mysqlpp::StoreQueryResult res = query.store()) {
				if(res.num_rows() != 0){
					return res.num_rows();
				}
				else	return 0;
	
			}
			else {
				cerr << "Failed to get item list: " << query.error() << endl;
				return -1;
			}
		}
		
	}
	else {
		cerr << "DB connection failed: " << conn.error() << endl;
		return -1;
	}

}

//this method return value: the total contact duration time beween user1 and user2
int distr::contact_duration(char *id1, char *id2){
	mysqlpp::Connection conn(false);

	/* Connect to database */
	if (conn.connect(config->db, "localhost", "root", "mysql")) {
		// Retrieve a subset of the sample stock table set up by resetdb
		// and display it
		//cout << "Connected to db!" << endl;
		
		char _query [200];
		bzero(_query, 200);
		//let's make sure that there is any contact between id1 and id2
		//bug: we only look at id1's observation which is not symmetric!
		sprintf (_query, "select isnull(sum(endtime-starttime)) from contactspan where user1 =%s and user2 = %s;", id1, id2);
		
		//printf ("the query: %s\n", _query);

		mysqlpp::Query query = conn.query(_query);
		if (mysqlpp::StoreQueryResult res = query.store()) {
			if (atoi(res[0][0]) == 0){
				bzero(_query, 200);
				//now let's calculate the total contact duration time between user1 and user2
				sprintf (_query, "select sum(endtime-starttime) from contactspan where user1 =%s and user2 = %s;", id1, id2);
		
				//printf ("the query: %s\n", _query);

				mysqlpp::Query query = conn.query(_query);
				if (mysqlpp::StoreQueryResult res = query.store()) {

					//cout << '\t' << "contact duration: " << res[0][0] << endl;
			 		return res[0][0];
				}else
					return 0;
			}
			else return 0;
		}
		else {
			cerr << "Failed to get item list: " << query.error() << endl;
			return -1;
		}
	}
	else {
		cerr << "DB connection failed: " << conn.error() << endl;
		return -1;
	}

}

//this method return value: the total contact duration time beween user1 and user2 during [t0,t1]
int distr::contact_duration(char *id1, char *id2, int t0, int t1){
	mysqlpp::Connection conn(false);

	/* Connect to database */
	if (conn.connect(config->db, "localhost", "root", "mysql")) {
		// Retrieve a subset of the sample stock table set up by resetdb
		// and display it
		//cout << "Connected to db!" << endl;
		
		char _query [200];
		bzero(_query, 200);
		//let's make sure that there is any contact between id1 and id2
		//reported bug: we only look at node id1's observation not node id2's logged contacts
		//the other issue is that we shoul look for those contacts which fall in the [t0,t1] interval completely (we should not only consider the starttime)
		sprintf (_query, "select isnull(sum(endtime-starttime)) from %s where user1 =%s and user2 = %s and starttime > %d and endtime < %d;", config->table, id1, id2, t0, t1);
		
		//printf ("the query: %s\n", _query);

		mysqlpp::Query query = conn.query(_query);
		if (mysqlpp::StoreQueryResult res = query.store()) {
			if (atoi(res[0][0]) == 0){
				bzero(_query, 200);
				//now let's calculate the total contact duration time between user1 and user2
				sprintf (_query, "select sum(endtime-starttime) from %s where user1 =%s and user2 = %s and starttime > %d and starttime < %d;", config->table, id1, id2, t0, t1);
		
				//printf ("the query: %s\n", _query);

				mysqlpp::Query query = conn.query(_query);
				if (mysqlpp::StoreQueryResult res = query.store()) {

					//cout << '\t' << "contact duration: " << res[0][0] << endl;
			 		return res[0][0];
				}else
					return 0;
			}
			else return 0;
		}
		else {
			cerr << "Failed to get item list: " << query.error() << endl;
			return -1;
		}
	}
	else {
		cerr << "DB connection failed: " << conn.error() << endl;
		return -1;
	}

}


//method return value: the social closeness between id1 and id2
//social: shows what dimensions should be taken into account
double distr::closeness(char *id1, char *id2, unsigned char social){

 int row_num = 0;	//number of rows

  mysqlpp::Connection conn(false);
	
  // Connect to database 
  if (conn.connect("infocom06", "localhost", "root", "mysql")) {
		// Retrieve a subset of the sample stock table set up by resetdb
		// and display it
		//cout << "Connected to db!" << endl;
		char _query [200];
		bzero(_query, 200);
		//let's find the lists of people who didnt fill the survey and exclude them
		sprintf (_query, "select count(*) from interests where affiliation!='' and nationality!='' and (name=%s or name=%s)", id1, id2);
		//printf ("the query: %s\n", _query);

		mysqlpp::Query query = conn.query(_query);
		
		if (mysqlpp::StoreQueryResult res = query.store()) {
			//cout << "We have:" << endl;
			row_num = atoi(res[0][0]);	//no of rows
			//cout << '\t' << row_num << endl;

		}
		else {
			cerr << "Failed to get item list: " << query.error() << endl;
			return -1;
		}

  }
  else {
		cerr << "DB connection failed: " << conn.error() << endl;
		return -1;
  }
double dotproduct = 0;
if(row_num ==2 && strcmp(id1,id2) != 0){
  rgmodel rg;
  double close = 0;
  char interest[ 20 ];

  int cnt = 0;
//  double weights[7] = {0.1, 0.1, 0.1, 0.3, 0.1, 0.0, 0.3};
  double weights[7] = {1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0};
  if(social & 01){
	bzero(interest, 20);
	sprintf(interest, "%s", "nationality");
  	close += weights[0]*rg.intersect(interest, id1, id2);
	cnt++;
  }
  if(social & 2){
	bzero(interest, 20);
	sprintf(interest, "%s", "studies");
	close += weights[1]*rg.intersect(interest, id1, id2);
	cnt++;
  }

  if(social & 4){
	bzero(interest, 20);
	sprintf(interest, "%s", "languages");
	close += weights[2]*rg.intersect(interest, id1, id2);
	cnt++;
  }

  if(social & 8){
	bzero(interest, 20);
	sprintf(interest, "%s", "affiliation");
	close += weights[3]*rg.intersect(interest, id1, id2);
	cnt++;
  }

  bzero(interest, 20);
  sprintf(interest, "%s", "position");
 // close += rg.intersect(interest, id1, id2);
  if(social & 16){
	bzero(interest, 20);
	sprintf(interest, "%s", "city");
	close += weights[4]*rg.intersect(interest, id1, id2);
	cnt++;
  }
  
  if(social & 32){
	bzero(interest, 20);
	sprintf(interest, "%s", "country");
	close += weights[5]*rg.intersect(interest, id1, id2);
	cnt++;
  }

  bzero(interest, 20);
  sprintf(interest, "%s", "airports");
 // close += rg.intersect(interest, id1, id2);

  bzero(interest, 20);
  sprintf(interest, "%s", "stay");
 // close += rg.intersect(interest, id1, id2);

  bzero(interest, 20);
  sprintf(interest, "%s", "member");
  //close += rg.intersect(interest, id1, id2);

  if(social & 64){
	bzero(interest, 20);
	sprintf(interest, "%s", "topics");
	close += weights[6]*rg.intersect(interest, id1, id2);
	cnt++;
  }

  dotproduct = close / (double)cnt;

  if(dotproduct == 0)	dotproduct = 0.01;	//nodes with no social connection
  }
  else if (strcmp(id1,id2) == 0)	dotproduct = 1.0;//the same node 
  else dotproduct = -1;	//nodes with no social profiles: we exclude these nodes
  
  return dotproduct;
}

//method return value: the social closeness between id1 and id2
//social: shows what dimensions should be taken into account
double distr::socialdist(char *id1, char *id2, unsigned char social){

 int row_num = 0;	//number of rows

  mysqlpp::Connection conn(false);
	
  // Connect to database 
  if (conn.connect("infocom06", "localhost", "root", "mysql")) {
		// Retrieve a subset of the sample stock table set up by resetdb
		// and display it
		//cout << "Connected to db!" << endl;
		char _query [200];
		bzero(_query, 200);
		//let's find the lists of people who didnt fill the survey and exclude them
		sprintf (_query, "select count(*) from interests where affiliation!='' and nationality!='' and (name=%s or name=%s)", id1, id2);
		//printf ("the query: %s\n", _query);

		mysqlpp::Query query = conn.query(_query);
		
		if (mysqlpp::StoreQueryResult res = query.store()) {
			//cout << "We have:" << endl;
			row_num = atoi(res[0][0]);	//no of rows
			//cout << '\t' << row_num << endl;

		}
		else {
			cerr << "Failed to get item list: " << query.error() << endl;
			return -1;
		}

  }
  else {
		cerr << "DB connection failed: " << conn.error() << endl;
		return -1;
  }

  double dist = 79.0;	//a very large no and it has to be > total no of nodes
  int focus = 0;	//let's find out which focus is more effective where nationality, studies,languages, affiliation, city, country, and topic have id from 1 to 7
if(row_num ==2 && strcmp(id1,id2) != 0){
  rgmodel rg;
  char interest[ 20 ];
  int fSize;
 
 if(social & 01){
  bzero(interest, 20);
  sprintf(interest, "%s", "nationality");
  fSize = rg.focusSize(interest, id1, id2);
if (fSize < dist) focus = 1; 
  dist = min((double)fSize, dist);
 }
//  cout << "nation: " << fSize << endl;

if(social & 2){
  bzero(interest, 20);
  sprintf(interest, "%s", "studies");
fSize = rg.focusSize(interest, id1, id2);
if (fSize < dist) focus = 2; 
  dist = min((double)fSize, dist);
}
//  cout << "studie: " << fSize << endl;

  if(social & 4){
  bzero(interest, 20);
  sprintf(interest, "%s", "languages");
fSize = rg.focusSize(interest, id1, id2);
if (fSize < dist) focus = 3; 
  dist = min((double)fSize, dist);
//  cout << "lang: " << fSize << endl;
}

  if(social & 8){
  bzero(interest, 20);
  sprintf(interest, "%s", "affiliation");
fSize = rg.focusSize(interest, id1, id2);
if (fSize < dist) focus = 4; 
  dist = min((double)fSize, dist);
//  cout << "affil: " << fSize << endl;
}

  bzero(interest, 20);
  sprintf(interest, "%s", "position");
//fSize = rg.focusSize(interest, id1, id2);
//  dist = min((double)fSize, dist);


//  cout << "position: " << rg.intersect(interest, id1, id2) << endl;
  if(social & 16){
  bzero(interest, 20);
  sprintf(interest, "%s", "city");
fSize = rg.focusSize(interest, id1, id2);
if (fSize < dist) focus = 5; 
  dist = min((double)fSize, dist);
//  cout << "city: " << fSize << endl;
}

  if(social & 32){
  bzero(interest, 20);
  sprintf(interest, "%s", "country");
fSize = rg.focusSize(interest, id1, id2);
if (fSize < dist) focus = 6; 
  dist = min((double)fSize, dist);
//  cout << "country: " << fSize << endl;
}

  bzero(interest, 20);
  sprintf(interest, "%s", "airports");
//  dist = min(rg.focusSize(interest, id1, id2), dist);

 // cout << "airp: " << rg.intersect(interest, id1, id2) << endl;

  bzero(interest, 20);
  sprintf(interest, "%s", "stay");
//    dist = min(rg.focusSize(interest, id1, id2), dist);


  bzero(interest, 20);
  sprintf(interest, "%s", "member");
 //   dist = min(rg.focusSize(interest, id1, id2), dist);

  if(social & 64){
  bzero(interest, 20);
  sprintf(interest, "%s", "topics");
fSize = rg.focusSize(interest, id1, id2);
if (fSize < dist) focus = 7; 
  dist = min((double)fSize, dist);
  
//  cout << "topics: " << fSize << endl;
}
//cout << focus << " " ;
  }
  else if (strcmp(id1,id2) == 0)	return 1.0;//the same node 
  else return -1.0;	//there is not social profiles, so the social distance is not calculatable!
 

  return dist;
}

//method return value: the social closeness between id1 and id2
//social: shows what dimensions should be taken into account
//we run the social distance based on Watts paper: two people are close if they are similar in any of social dimensions
double distr::sim_watts(char *id1, char *id2, unsigned char social){

 int row_num = 0;	//number of rows

  mysqlpp::Connection conn(false);
	
  // Connect to database 
  if (conn.connect("infocom06", "localhost", "root", "mysql")) {
		// Retrieve a subset of the sample stock table set up by resetdb
		// and display it
		//cout << "Connected to db!" << endl;
		char _query [200];
		bzero(_query, 200);
		//let's find the lists of people who didnt fill the survey and exclude them
		sprintf (_query, "select count(*) from interests where affiliation!='' and nationality!='' and (name=%s or name=%s)", id1, id2);
		//printf ("the query: %s\n", _query);

		mysqlpp::Query query = conn.query(_query);
		
		if (mysqlpp::StoreQueryResult res = query.store()) {
			//cout << "We have:" << endl;
			row_num = atoi(res[0][0]);	//no of rows
			//cout << '\t' << row_num << endl;

		}
		else {
			cerr << "Failed to get item list: " << query.error() << endl;
			return -1;
		}

  }
  else {
		cerr << "DB connection failed: " << conn.error() << endl;
		return -1;
  }
double sim = 0;
if(row_num ==2 && strcmp(id1,id2) != 0){
  rgmodel rg;
  double close = 0;
  char interest[ 20 ];

  int cnt = 0;

  if(social & 01){
	bzero(interest, 20);
	sprintf(interest, "%s", "nationality");
  	close = max(rg.intersect(interest, id1, id2), close);
	cnt++;
  }
  if(social & 2){
	bzero(interest, 20);
	sprintf(interest, "%s", "studies");
	close = max(rg.intersect(interest, id1, id2), close);
	cnt++;
  }

  if(social & 4){
	bzero(interest, 20);
	sprintf(interest, "%s", "languages");
	close = max(rg.intersect(interest, id1, id2), close);
	cnt++;
  }

  if(social & 8){
	bzero(interest, 20);
	sprintf(interest, "%s", "affiliation");
  	close = max(rg.intersect(interest, id1, id2), close);
	cnt++;
  }

  bzero(interest, 20);
  sprintf(interest, "%s", "position");
 // close += rg.intersect(interest, id1, id2);
  if(social & 16){
	bzero(interest, 20);
	sprintf(interest, "%s", "city");
  	close = max(rg.intersect(interest, id1, id2), close);
	cnt++;
  }
  
  if(social & 32){
	bzero(interest, 20);
	sprintf(interest, "%s", "country");
  	close = max(rg.intersect(interest, id1, id2), close);
	cnt++;
  }

  bzero(interest, 20);
  sprintf(interest, "%s", "airports");
 // close += rg.intersect(interest, id1, id2);

  bzero(interest, 20);
  sprintf(interest, "%s", "stay");
 // close += rg.intersect(interest, id1, id2);

  bzero(interest, 20);
  sprintf(interest, "%s", "member");
  //close += rg.intersect(interest, id1, id2);

  if(social & 64){
	bzero(interest, 20);
	sprintf(interest, "%s", "topics");
  	close = max(rg.intersect(interest, id1, id2), close);
	cnt++;
  }

  sim = close;

  if(sim == 0)	sim = 1.0/100.0;	//nodes with no social connection
  }
  else if (strcmp(id1,id2) == 0)	sim = 1.0;//the same node 
  else sim = -1;	//nodes with no social profiles: we exclude these nodes
  
  return sim;
}

//this function calculates the Ranks of all nodes in [tStart, tStop] interval
//arguments: wLen= window length for counting unique encouners of each node, and uTime= update duration for calling uRank function
// tStart: start time for calculating ranks, tStop: stopping time for calculating ranks

int distr::Ranks(int tStart, int tStop, int wLen, int uTime){
	int r_id = 0;	//the rank of node id
	int id = 40;
	int current_time;	//emulation clock is updated
	//let's store delay in profile.dat file

	ofstream ranksF, timesF;
  	ranksF.open ("/home/kazem/Desktop/traces/logs/ranks.dat", ios::app);

//   	timesF.open ("/home/kazem/Desktop/traces/logs/times.dat", ios::app);

	// exit program if unable to create file
		
// 	if ( !timesF ) // overloaded ! operator
// 	{
// 		cerr << "File times.dat could not be opened" << endl;
// 		return -1;
// 	} // end if

	// exit program if unable to create file
		
	if ( !ranksF ) // overloaded ! operator
	{
		cerr << "File ranks.dat could not be opened" << endl;
		return -1;
	} // end if

	for(current_time = tStart; current_time <= tStop; current_time += uTime){
		//we have to update the rank list for all users
		cout << (double)(current_time - tStart)/3600.0 << " ";
		cg.updateRanks(tStart, current_time, wLen, 0);
		for (id = 21; id <= 99; id++){
			r_id = cg.rank(id);
			ranks_t[id - 21].push_back(r_id);	//push rank number into link list table
//			time_r[id-21].push_back((double)(current_time - tStart)/3600.0);	//save the time sequence
		}
	}

 	list<int>::iterator k;	// an iterator for the link list
//  	list<double>::iterator l;	// an iterator for the link list

	ranksF << "rank lists:" << endl;
 	for (int j = 0; j < 79; j++){	
//		ranks << "nid: " << j + 21 << endl;
//		ranks << "ranks: ";
		for(k = ranks_t[j].begin(); k != ranks_t[j].end(); ++k) 
			ranksF << (*k) << " ";
		ranksF << ";";

/*		for(l = time_r[j].begin(); l != time_r[j].end(); ++l) 
			timesF << (*l) << " ";
		timesF << ";";*/
	}

	ranksF.close();		

	return 0;
}

//what are the locations of nodes: what static nodes they have met --> we are interested in recording the location sequence that we have visited from sTime to eTime
int distr::locations(int sTime, int eTime){

	ofstream locations, times_;	//files for locations and times sequence
  	locations.open ("/home/kazem/Desktop/traces/logs/locations.dat", ios::app);

	// exit program if unable to create file
		
	if ( !locations ) // overloaded ! operator
	{
		cerr << "File locations.dat could not be opened" << endl;
		return -1;
	} // end if

  	times_.open ("/home/kazem/Desktop/traces/logs/times.dat", ios::app);

	// exit program if unable to create file
		
	if ( !times_ ) // overloaded ! operator
	{
		cerr << "File times.dat could not be opened" << endl;
		return -1;
	} // end if

	mysqlpp::Connection conn(false);

	// Connect to database 
	if (conn.connect("infocom06", "localhost", "root", "mysql")) {
		// Retrieve a subset of the sample stock table set up by resetdb
		// and display it
		for (int i = 21; i <= 99; i++)
		{
			char id[10];	//user id
			bzero (id, 10);
			sprintf(id, "%d", i);
			char _query [400];
			bzero(_query, 400);
			//let's find the contacts that we had with static nodes
//			sprintf (_query, "select user2, starttime from contactspan where user2 <= 20 and starttime > %d and starttime < %d and user1 = %s order by starttime;", sTime, eTime, id);

			sprintf (_query, "select user1, starttime from contactspan where user1 <= 20 and starttime > %d and starttime < %d and user2 = %s order by starttime;", sTime, eTime, id);

			//if(i == 59) printf ("the query: %s\n", _query);
			
			mysqlpp::Query query = conn.query(_query);
	
			if (mysqlpp::StoreQueryResult res = query.store()) {
				if(res.num_rows() != 0){
			for (size_t j = 0; j < res.num_rows(); ++j) {
				//going through db and finding contact duration time for each contact and store them inside map table

					int static_n = atoi(res[j][0]);	//static node that we have visited: node sequence
//					double time = atoi(res[j][0]);
					int time = ceil((atoi(res[j][1]) - sTime)/600.0);
					locations_t[i - 21].push_back(static_n);	//push rank number into link list table
					times[i-21].push_back(time);	//save the time sequence
			}
				}
				else{
					locations_t[i-21].push_back(0);	//push rank number into link list table
				}
			}
			else {
				cerr << "Failed to get item list: " << query.error() << endl;
				return -1;
			}
		}
	}
	else {
		cerr << "DB connection failed: " << conn.error() << endl;
		return -1;
	}

 	list<int>::iterator k;	// an iterator for the link list
	
	locations << "location sequences from: " << sTime << ", to: " << eTime << endl;
	times_ << "time sequences from: " << sTime << ", to: " << eTime << endl;
 	for (int j = 0; j < 79; j++){	
		locations << "node: " << j + 21 << endl;
		times_ << "node: " << j + 21 << endl;
		for(k = locations_t[j].begin(); k != locations_t[j].end(); ++k) 
			locations << (*k) << " ";
		locations << endl;

		for(k = times[j].begin(); k != times[j].end(); ++k) 
			times_ << (*k) << " ";
		times_ << endl;
	}

	locations.close();		
	times_.close();

	return 0;
}
//read the locations which were visited by our nodes over a given time period
int distr::visited_locs(int sTime, int eTime){

	mysqlpp::Connection conn(false);
	
	// Connect to database 
	if (conn.connect("infocom06", "localhost", "root", "mysql")) {
		// Retrieve a subset of the sample stock table set up by resetdb
		// and display it
		for (int i = 21; i <= 99; i++){
		char id[10];	//user id
		bzero (id, 10);
		sprintf(id, "%d", i);
		char _query [400];
		bzero(_query, 400);
		//let's find the contacts that we had with static nodes
		//sprintf (_query, "select user2, starttime from contactspan where user2 <= 20 and starttime > %d and starttime < %d and user1 = %s order by starttime;", sTime, eTime, id);

		sprintf (_query, "select user1, starttime from contactspan where user1 <= 20 and starttime > %d and starttime < %d and user2 = %s order by starttime;", sTime, eTime, id);

		
		mysqlpp::Query query = conn.query(_query);

		if (mysqlpp::StoreQueryResult res = query.store()) {
			if(res.num_rows() != 0){
		for (size_t j = 0; j < res.num_rows(); ++j) {
			//going through db and finding contact duration time for each contact and store them inside map table

				int static_n = atoi(res[j][0]);	//static node that we have visited: node sequence
				//double time = atoi(res[j][0]);
				int time = ceil((atoi(res[j][1]) - sTime)/600.0);
				locations_t[i-21].push_back(static_n);	//push rank number into link list table
				//times.push_back(time);	//save the time sequence
		}
			}
		}
		else {
			cerr << "Failed to get item list: " << query.error() << endl;
			return -1;
		}
	}
	}
	else {
		cerr << "DB connection failed: " << conn.error() << endl;
		return -1;
	}
	return 0;
}
//finding the sequence of nodes quality which a specific node meets during the conference
int distr::node_sequence(int sTime, int eTime, int id, int wLen, int uTime){
	int lUpdate = 0;	//this is for ranking updates
	list<int> quality_t;	//qualiy list
	list<double> times;	//time sequence
	list<int> node_id;	//node id sequenec
	list<int> own_t;	//node's own quality

	ofstream quality;	//files for quality and time sequence
  	quality.open ("/home/kazem/Desktop/traces/logs/quality.dat", ios::app);

	// exit program if unable to create file
		
	if ( !quality ) // overloaded ! operator
	{
		cerr << "File quality.dat could not be opened" << endl;
		return -1;
	} // end if
	mysqlpp::Connection conn(false);

	// Connect to database 
//	if (conn.connect("infocom06", "localhost", "root", "mysql")) {
	if (conn.connect(config->db, "localhost", "root", "mysql")) {
/*			char id_[10];	//user id
			bzero (id_, 10);
			sprintf(id_, "%d", id);*/
			char _query [500];
			bzero(_query, 500);

			//let's log the user1's which have been seen by user id
//			sprintf (_query, "select user2,starttime from contactspan where user2 > 20 and user2 <= 99 and starttime > %d and starttime < %d and user1 = %d order by starttime;", sTime, eTime, id);	//infocom06 query

			sprintf (_query, "select user2,starttime from %s where user2 >= %d and user2 <= %d and starttime > %d and starttime < %d and user1 = %d order by starttime;", config->table, config->s_nid, config->e_nid, sTime, eTime, id);	//infocom05 query

//			cout << "query: " << _query << endl;
			mysqlpp::Query query = conn.query(_query);
	
			if (mysqlpp::StoreQueryResult res = query.store()) {
				if(res.num_rows() != 0){
					for (size_t j = 0; j < res.num_rows(); ++j) {
		
						int id_ = res[j][0];	//user2: the user which we met
						int current_time = res[j][1];	//current time: emulation clock is updated
						if((lUpdate == 0 || (current_time - lUpdate) > uTime) && (current_time  <= eTime)){
							//we have to update the rank list for all users
							cg.updateRanks(sTime, current_time, wLen, 0);
							lUpdate = current_time;		//update the last update pointer for rank lists
						}
//						cout << cg.rank(id) << endl;
						quality_t.push_back(cg.rank(id_));	//push rank number into link list table
						times.push_back(double((current_time - sTime)/3600.0));	//save the time sequence: the hour is logged
						node_id.push_back(id_);
						own_t.push_back(cg.rank(id));	//node's own quality
					}
				}
				else{
					own_t.push_back(-1);
					node_id.push_back(-1);
					quality_t.push_back(-1);	//push rank number into link list table
					times.push_back(-1);
				}
			}
			else {
				cerr << "Failed to get item list: " << query.error() << endl;
				return -1;
			}
	}
	else {
		cerr << "DB connection failed: " << conn.error() << endl;
		return -1;
	}

 	list<int>::iterator k;	// an iterator for the link list

	quality << "quality sequences of other nodes met by " << id << " from "  << sTime << " to " << eTime << endl;

	for(k = quality_t.begin(); k != quality_t.end(); ++k) 
		quality << (*k) << " ";
	quality << endl << endl;

	quality << "node " << id << " 's own quality sequences from " << sTime << " to " << eTime << endl;
	
	for(k = own_t.begin(); k != own_t.end(); ++k) 
		quality << (*k) << " ";
	quality << endl;

	quality << "time sequences from " << sTime << " to " << eTime << endl;
	
 	list<double>::iterator l;	

	for(l = times.begin(); l != times.end(); ++l) 
		quality << (*l) << " ";
	quality << endl;

	quality << "id sequences:" << endl;

	for(k = node_id.begin(); k != node_id.end(); ++k) 
		quality << (*k) << " ";
	quality << endl;

	quality.close();		

	return 0;
}


//this method return value: the total no of contacts for each node during [t0,t1]
int distr::total_degrees(int t0, int t1){
	mysqlpp::Connection conn(false);

	/* Connect to database */
	if (conn.connect("infocom06", "localhost", "root", "mysql")) {
		// Retrieve a subset of the sample stock table set up by resetdb
		// and display it
		//cout << "Connected to db!" << endl;
		for (int i = 21; i <= 99; i++){
			char id[10];	//user id
			bzero (id, 10);
			sprintf(id, "%d", i);		
			char _query [200];
			bzero(_query, 200);
			//here there is a missing point: what about the contact which have been logged by id2
			sprintf (_query, "select user2 from contactspan where user1=%s and starttime >= %d and endtime <= %d and user2 >= 21 and user2 <= 99", id, t0, t1);
			//printf ("the query: %s\n", _query);
	
			mysqlpp::Query query = conn.query(_query);
			if (mysqlpp::StoreQueryResult res = query.store()) {
				cout << res.num_rows() << " ";				
//				cout << res[0][0] << " ";
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

}

//let's count the no of times an external node appears in the contact graph
int distr::extcount(int t0, int t1)
{
	map<int,int>::iterator it;

	if(!extcont.empty()) extcont.clear();

	ofstream extcontF( "/home/kazem/Desktop/traces/logs/extcont.dat", ios::app );
		
	// exit program if unable to create file
		
	if ( !extcontF ) // overloaded ! operator
	{
		cerr << "File extcont.dat could not be opened" << endl;
		return -1;
	} // end if
	
	mysqlpp::Connection conn(false);

	/* Connect to database */
	if (conn.connect("infocom06", "localhost", "root", "mysql")) {
		// Retrieve a subset of the sample stock table set up by resetdb
		// and display it
		//cout << "Connected to db!" << endl;
		char _query [200];
		bzero(_query, 200);
		//here there is a missing point: what about the contact which have been logged by id2
		sprintf (_query, "select user2 from contactspan where user2 > 99 and starttime >= %d and endtime <= %d", t0, t1);
		//printf ("the query: %s\n", _query);

		mysqlpp::Query query = conn.query(_query);
		if (mysqlpp::StoreQueryResult res = query.store()) {
			if(res.num_rows() != 0){
				for (size_t j = 0; j < res.num_rows(); ++j){ 		
				//	cout << res[j][0] << " ";
					extcont.insert(std::make_pair(res[j][0], 0));
				}
			}
		}
		else {
			cerr << "Failed to get item list: " << query.error() << endl;
			return 1;
		}

    		//cout << (*it).first << " => " << (*it).second << endl;
 		for ( it=extcont.begin() ; it != extcont.end(); it++ ){
		bzero(_query, 200);
		//here there is a missing point: what about the contact which have been logged by id2
		sprintf (_query, "select count(*) from contactspan where user2 = %d and starttime >= %d and endtime <= %d", (*it).first, t0, t1);
		//printf ("the query: %s\n", _query);

		mysqlpp::Query query = conn.query(_query);
		if (mysqlpp::StoreQueryResult res = query.store()) {
			(*it).second = res[0][0];
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
	int extcounter = 0;
	for ( it=extcont.begin() ; it != extcont.end(); it++ )
		if((*it).second > 5){
			extcontF << (*it).first << " => " << (*it).second << endl;
			extcounter += (*it).second;
	}
	cout << extcounter << endl;
	extcontF.close();
}
