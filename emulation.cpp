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
#include <fstream> // file stream        

using std::ifstream; // input file stream
using namespace std;

//this method generates simulation table
int emulation::simt(){
	mysqlpp::Connection conn(false);

	/* Connect to database */
	if (conn.connect("infocom06", "localhost", "root", "mysql")) {
		// Retrieve a subset of the sample stock table set up by resetdb
		// and display it
		//cout << "Connected to db!" << endl;
		//list of members in community 9
		int members[12] = { 23, 30, 31, 35, 36, 40, 55, 59, 65, 83, 96, 99};
		char _query [6000];
		char str[100];
		bzero(_query, 6000);
		int t = 0;
		//we will go through all existing contacts between nodes in a community during an interval and insert all of them into another table called simulation table, later on we will use simulation table to emulate message dlivery in our contact graph 
		memcpy(_query + t, "insert into simulation (starttime, endtime, user1, user2) select contactspan.starttime, contactspan.endtime, contactspan.user1, contactspan.user2 from contactspan where contactspan.starttime > 57000 and contactspan.endtime < 64800 and (", 236);
		t += 236;
		for (int i = 0; i < 12; i++){
			for (int j = i + 1; j < 12; j++){
				bzero(str, 100);

				if(i == 10 && j==11)	
					sprintf (str, "(contactspan.user1=%d and contactspan.user2=%d)) ORDER BY contactspan.starttime", members[i], members[j]);
				else	sprintf (str, "(contactspan.user1=%d and contactspan.user2=%d) or ", members[i], members[j]);

				memcpy(_query + t, str, strlen(str));
				t += strlen(str);
				//cout << _squery << endl;
			}
		}
		cout << _query << endl;
		//printf ("the query: %s\n", _query);
		//let's generate simulation table. every row in the simulation table represents an event: a communication between user1 and user2 at starttime until endtime in which direction!
		mysqlpp::Query query = conn.query(_query);
		query.execute();
	}
	else {
		cerr << "DB connection failed: " << conn.error() << endl;
		return 1;
	}
	
}
//let's run the emualtion by using simulation table
int emulation::run(){
  //T: packet generation rate
  int T = 300, transT = 0;	//let's think node 21 sends its position every 5 mins
  int init = 57000;	//simulation's starting time: the first location has been broadcasted at this time
  map<int, int>::iterator ii, jj, kk;
  map<int, int> state;	//this table keeps the last position whose every node has about 23's position
  map<int, int> trans;	//this table keeps the mapping between message number and time of generation
  int members[12] = { 23, 30, 31, 35, 36, 40, 55, 59, 65, 83, 96, 99};	//affiliation 9
  char _uquery [200];
  int node = 23;	//node which broadcasts its position once a while
  //trace files
  ofstream outFile_ndelay( "/home/kazem/Desktop/traces/ndelay.dat", ios::out );	//nodes delay distribution
  //we use mat.dat for matlab purpose
  ofstream outFile_tdelay( "/home/kazem/Desktop/traces/tdelay.dat", ios::out );	//total delay distribution

  transT = init;	//the first transmission time
//  state.insert(std::make_pair(members[0], 1));
//  state.insert(std::make_pair(1, transT));

  // exit program if unable to create file
	
  if ( !outFile_ndelay ) // overloaded ! operator
  {
    cerr << "File ndelay.dat could not be opened" << endl;
    return -1;
  } // end if

  if ( !outFile_tdelay ) // overloaded ! operator
  {
    cerr << "File tdelay.dat could not be opened" << endl;
    return -1;
  } // end if


  for (int i = 0; i < 12; i++) state.insert(std::make_pair(members[i], -1));	//initialize the map table with null message
  //now let's go through simualtion table and run the simulation step by step in a time order
  mysqlpp::Connection conn(false);

  /* Connect to database */
  if (conn.connect("infocom06", "localhost", "root", "mysql")) {
	// Retrieve a subset of the sample stock table set up by resetdb
	// and display it
	mysqlpp::Query query = conn.query("select * from simulation");
	if (mysqlpp::StoreQueryResult res = query.store()) {
		for (size_t i = 0; i < res.num_rows(); ++i) {
			if((atoi(res[i][3]) == node || atoi(res[i][4]) == node) /*&& (atoi(res[i][1]) - transT >= T)*/){
				//node generate a new data msg at each contact which is its location
				ii = state.find(node);
				(*ii).second++;
				cout << "generate a new packet at: " << res[i][1] << endl;
				transT = atoi(res[i][1]); //the time this message has been generated, we send our new location!
  				trans.insert(std::make_pair((*ii).second, transT));	//update the message-time mapping
			}	
			
			//update query
			ii = state.find(atoi(res[i][3]));	//user1's msg
			jj = state.find(atoi(res[i][4]));	//user2's msg
			cout << "ii: " << (*ii).second << "jj: " << (*jj).second << endl;
			//if user1 --> user2, then dir=1 otherwise: dir=-1
			if((*ii).second > (*jj).second){
				//the msg goes from user1 --> user2
				bzero(_uquery, 200);
				int k = res[i][3];
				kk = trans.find((*ii).second);	//find the time of transmission
				sprintf (_uquery, "update simulation set msg=%d,dir=%d,transtime=%d where user1=%d and user2=%d and starttime=%d", (*ii).second, 1, (*kk).second, atoi(res[i][3]), atoi(res[i][4]), atoi(res[i][1]));

				mysqlpp::Query uquery = conn.query(_uquery);
				(*jj).second = (*ii).second;	//exchange the msg: user1-->user2: update  statet table
				//update simulation table to indicate the message exchange
				uquery.execute();
				cout << "at time: " << res[i][1] << ", " << res[i][3] << " ---> " << res[i][4] << ", msg: " << (*ii).second << " transmitted at: " << (*kk).second << endl;
				outFile_tdelay << atoi(res[i][1]) - (*kk).second << ',';	//delay
				outFile_ndelay << atoi(res[i][4]) << ":" << atoi(res[i][1]) - (*kk).second << ',';	//delay
				
			}else if ((*ii).second < (*jj).second){
				//the msg goes from user2 --> user1
				bzero(_uquery, 200);
				kk = trans.find((*jj).second);	//find the time of transmission
				sprintf (_uquery, "update simulation set msg=%d,dir=%d,transtime=%d where user1=%d and user2=%d and starttime=%d", (*jj).second, -1, (*kk).second, atoi(res[i][3]), atoi(res[i][4]), atoi(res[i][1]), transT);

				mysqlpp::Query uquery = conn.query(_uquery);
				(*ii).second = (*jj).second;	//exchange the msg: user2-->user1: update state table
				//update simulation table to indicate the message exchange
				uquery.execute();
				cout << "at time: " << res[i][1] << ", " << res[i][3] << " <--- " << res[i][4] << ", msg: " << (*jj).second << " transmitted at: " << (*kk).second << endl;

				outFile_tdelay << atoi(res[i][1]) - (*kk).second << ',';	//delay
				outFile_ndelay << atoi(res[i][3]) << ":" << atoi(res[i][1]) - (*kk).second << ',';	//delay

			}else if ((*ii).second == (*jj).second){
				//the msg goes from user2 --> user1
				bzero(_uquery, 200);
				sprintf (_uquery, "update simulation set msg=%d,dir=%d,transtime=%d where user1=%d and user2=%d and starttime=%d", (*jj).second, 0, 0, atoi(res[i][3]), atoi(res[i][4]), atoi(res[i][1]));

				mysqlpp::Query uquery = conn.query(_uquery);
				//update simulation table to indicate the message exchange
				uquery.execute();
				//cout << "at time: " << res[i][1] << "no msg exchange!" << endl;

			}			
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
}
