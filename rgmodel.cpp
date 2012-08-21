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
#include "common.h"

using std::ifstream; // input file stream
using namespace std;

//this method gets two users' names and caculates the closeness for a specific interest
double rgmodel::intersect(char *interest, char *id1, char *id2){
	mysqlpp::Connection conn(false);

	/* Connect to database */
	if (conn.connect("infocom06", "localhost", "root", "mysql")) {
		// Retrieve a subset of the sample stock table set up by resetdb
		// and display it
		//cout << "Connected to db!" << endl;
		
		char _query [200];
		bzero(_query, 200);
		sprintf (_query, "select %s from interests where name=%s or name=%s", interest, id1, id2);
		//printf ("the query: %s\n", _query);

		mysqlpp::Query query = conn.query(_query);
		if (mysqlpp::StoreQueryResult res = query.store()) {
			char v1[50], v2[50];
			bzero(v1, 50); 
			bzero(v2, 50); 
			strncpy(v1, res[0][0], 50);
			strncpy(v2, res[1][0], 50);
			//cout << '\t' << "v1=(" << v1 << ") & v2=(" << v2 << ")" << endl;
  			char temp[ 100 ];
			bzero(temp, 100);
			sprintf(temp,",%s,", v2); 
  
			//  cout << temp << endl;
			char key[] = ",";
			char * pch;
			int cnt1 = 1;
			pch = strpbrk (v1, key);
			while (pch != NULL)
			{
			pch = strpbrk (pch+1,key);
			cnt1++;
			}
			//cout << endl;
			//cout << "no of interests of v1: " << cnt1 << endl;
			
			int cnt2 = 1;
			pch = strpbrk (v2, key);
			while (pch != NULL)
			{
			pch = strpbrk (pch+1,key);
			cnt2++;
			}
			
			//cout << endl;
			//cout << "no of interests of v2: " << cnt2 << endl;
			
			int intersect = 0;
			pch = strtok (v1,",");
			while (pch != NULL)
			{
			//cout << pch << endl;
			
			if (pch != NULL){
			char * found;
			//match intermediate part
			char match[ 6 ];
			bzero(match, 6);
			sprintf(match, ",%s,", pch);
			
			//cout << match << endl;
			
			found = strstr (temp, match);
			
			if (found != NULL)	{
//				cout << "id1: " << id1 << ",id2: " << id2 << '\t' << "v1=(" << v1 << ") & v2=(" << v2 << ")" << endl;
//				cout << "match found: " << found << endl;
				intersect++;
			}
			
			}
			
			pch = strtok (NULL, ",");
			}
			
			int _union = cnt1 + cnt2 - intersect;
			
			//cout  << "union: " << _union << ", intersection: " << intersect << endl;
			
			double closeness = (double) intersect / (double) _union;
			//cout << "closeness: " << closeness << endl;
			return closeness;
			
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

//this method gets two users' names and caculates the closeness for a specific interest
int rgmodel::focusSize(char *interest, char *id1, char *id2){
	mysqlpp::Connection conn(false);

	/* Connect to database */
	if (conn.connect("infocom06", "localhost", "root", "mysql")) {
		// Retrieve a subset of the sample stock table set up by resetdb
		// and display it
		//cout << "Connected to db!" << endl;
		
		char _query [200];
		bzero(_query, 200);
		sprintf (_query, "select %s from interests where name=%s or name=%s", interest, id1, id2);
		//printf ("the query: %s\n", _query);

		mysqlpp::Query query = conn.query(_query);
		if (mysqlpp::StoreQueryResult res = query.store()) {
			char v1[50], v2[50];
			bzero(v1, 50); 
			bzero(v2, 50); 
			strncpy(v1, res[0][0], 50);
			strncpy(v2, res[1][0], 50);
			//cout << '\t' << "v1=(" << v1 << ") & v2=(" << v2 << ")" << endl;


  char temp[ 100 ];
  bzero(temp, 100);
  sprintf(temp,",%s,", v2); 
  
//  cout << temp << endl;


  char key[] = ",";
  char * pch;
int cnt1 = 1;
  pch = strpbrk (v1, key);
  while (pch != NULL)
  {
    pch = strpbrk (pch+1,key);
    cnt1++;
  }
//cout << endl;
//cout << "no of interests of v1: " << cnt1 << endl;

int cnt2 = 1;
  pch = strpbrk (v2, key);
  while (pch != NULL)
  {
    pch = strpbrk (pch+1,key);
    cnt2++;
  }

//cout << endl;
//cout << "no of interests of v2: " << cnt2 << endl;

int focusS = 79, focusS_temp = 79;	//this focus size, initial value is set as a very large no

  pch = strtok (v1,",");
  while (pch != NULL)
  {
    //cout << pch << endl;
    
if (pch != NULL){
    char * found;
    //match intermediate part
    char match[ 6 ];
    bzero(match, 6);
    sprintf(match, ",%s,", pch);
    
    found = strstr (temp, match);

    if (found != NULL)	{
//        cout << "users: (" << id1 << "," << id2 << ")" << " share " << match << "for " << interest  << endl;
	//match: the shared focus
	//let's find how many member this focus has
	focusS_temp = 2;	//we have id1 and id2 initialy inside this focus

	bzero(_query, 200);
	sprintf (_query, "select %s, name from interests where name!=%s and name!=%s", interest, id1, id2);
	//printf ("the query: %s\n", _query);

	mysqlpp::Query query = conn.query(_query);
	if (mysqlpp::StoreQueryResult res = query.store()) {
		for (size_t i = 0; i < res.num_rows(); ++i) {
			char v[50];
			char * found_;
			bzero(v, 50); 
			strncpy(v, res[i][0], 50);

  			char temp_[ 100 ];
  			bzero(temp_, 100);
  			sprintf(temp_,",%s,", v); 

			found_ = strstr (temp_, match);
			if (found_ != NULL){
				//this user id has the same focus as id1 and id2
//				cout << res[i][0] << endl;
//				cout << "user: " << res[i][1] << " shares focus " << match << " on the " << interest << " with users: " << id1 << " & " << id2 << endl;
				focusS_temp++;
			}
		}
	}
	else {
		cerr << "Failed to get item list: " << query.error() << endl;
		return 1;
	}
//	if (focusS_temp < 79) cout << "new focus size " << interest << " : " << focusS_temp << endl;
	focusS = min(focusS, focusS_temp);	//keep track of minimum focus set
    }

}

pch = strtok (NULL, ",");
  }

//if (focusS < 79) cout << "size of focus " << interest << " : " << focusS << endl;

return focusS;
			
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
