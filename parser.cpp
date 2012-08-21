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
#include "contactspan.h"
#include "interests.h"
#include "common.h"
#include <string>
#include <fstream> // file stream        
#include "rgmodel.h"
#include "distr.h"

using std::ifstream; // input file stream
using namespace std;

mysqlpp::Connection parser::connect_to_db(){
	mysqlpp::Connection conn(false);
	if (conn.connect("infocom06", "localhost", "root", "mysql")){
		cout << "0" << endl;
		return conn;
	}
	else
		return -1;
}
//function which connects to mysql
//this method parse the contact data from "contacts.Exp6.dat" file into contactspan table
// the only parameter is the contat file name with full path
int parser::parse_contact_data(const char *file){
	ifstream inClientFile(file, ios::in );
	// exit program if ifstream could not open file
        if ( !inClientFile )
        {
        	cerr << file << " could not be opened" << endl;
        	exit( 1 );
        } // end if

	mysqlpp::Connection conn(false);

	/* Connect to database */
	if (conn.connect(config->db, "localhost", "root", "mysql")) {
		// Retrieve a subset of the sample stock table set up by resetdb
		// and display it
		cout << "Connected to " << config->db << endl;

	//the columns of the contactspan table
	char ID1[ 9 ];
	char ID2[ 9 ];	
	double stime;	//I used this for simulated contact files
	double etime;
	int enumer;
	int intercon;

     	// display each record in file
	//read the records from the file and insert them into contactspan table
	int counter = 0;
     	while ( inClientFile >> ID1 >> ID2 >> stime >> etime >> enumer >> intercon){
        	//cout << left << setw( 10 ) << ID1 << endl;
		// constructor: creating a row of data.
		char _query [500];
		bzero(_query, 500);
		//we should count all contacts: not only long ones!!!
		sprintf (_query, "insert into %s (starttime, endtime, user1, user2, enumerator, intercontact) values (%f, %f, %s, %s, %d, %d)", config->table, stime, etime, ID1, ID2, enumer, intercon);	

		mysqlpp::Query query = conn.query(_query);

  	        // Show the query about to be executed.
// 	        cout << "Query: " << query << endl;

	        // Execute the query.  We use execute() because INSERT doesn't
	        // return a result set.
	        query.execute();
		counter++;
	}
	}
	else {
		cerr << "DB connection failed: " << conn.error() << endl;
		return 1;
        }

	 // ifstream destructor closes the file
	inClientFile.close();	
	return 0;	
}

int parser::parse_synth_contact_data(const char *file){
	ifstream inClientFile(file, ios::in );
	// exit program if ifstream could not open file
        if ( !inClientFile )
        {
        	cerr << file << " could not be opened" << endl;
        	exit( 1 );
        } // end if

	mysqlpp::Connection conn(false);

	/* Connect to database */
	if (conn.connect(config->db, "localhost", "root", "mysql")) {
		// Retrieve a subset of the sample stock table set up by resetdb
		// and display it
		cout << "Connected to " << config->db << endl;

	//the columns of the contactspan table
	char ID1[ 9 ];
	char ID2[ 9 ];	
	int stime;
	int etime;
	int enumer;
	int intercon;

     	// display each record in file
	//read the records from the file and insert them into contactspan table
	int counter = 0;
     	while ( inClientFile >> ID1 >> ID2 >> stime >> etime >> enumer >> intercon){
        	//cout << left << setw( 10 ) << ID1 << endl;
		// constructor: creating a row of data.
		char _query [500];
		bzero(_query, 500);
		//we should count all contacts: not only long ones!!!
		sprintf (_query, "insert into %s (starttime, endtime, user1, user2, enumerator, intercontact) values (%d, %d, %s, %s, %d, %d)", config->table, stime, etime, ID1, ID2, enumer, intercon);	

		mysqlpp::Query query = conn.query(_query);

  	        // Show the query about to be executed.
//	        cout << "Query: " << query << endl;

	        // Execute the query.  We use execute() because INSERT doesn't
	        // return a result set.
	        query.execute();
		counter++;
	}
	}
	else {
		cerr << "DB connection failed: " << conn.error() << endl;
		return 1;
        }

	 // ifstream destructor closes the file
	inClientFile.close();	
	return 0;	
}

//function which connects to mysql
//this method parse the interests data from forms into interests table
int parser::parse_interest_data(){
	mysqlpp::Connection conn(false);

	/* Connect to database */
	if (conn.connect("infocom06", "localhost", "root", "mysql")) {
		// Retrieve a subset of the sample stock table set up by resetdb
		// and display it
		cout << "Connected to db!" << endl;
	}
	else {
		cerr << "DB connection failed: " << conn.error() << endl;
		return 1;
}
	int counter = 21;
	while (counter <= 99){
		char file [50];
		bzero(file, 50);
		sprintf (file, "/home/kazem/Desktop/haggle/Exp6/forms/%d.dat", counter);
		printf ("file name: %s\n",file);
	
		// ifstream constructor opens the file for contacts
		ifstream inClientFile( file, ios::in );
		// exit program if ifstream could not open file
		if ( !inClientFile )
		{
			cerr << file << " could not be opened" << endl;
			exit( 1 );
		} // end if
		//the columns of the interests table
		char name[ 255 ];
		char email[ 255 ];
		char nationality[ 255 ];
		char studies[ 255 ];
		char languages[ 255 ];
		char affiliation[ 255 ];
		char position[ 255 ];
		char city[ 255 ];
		char country[ 255 ];
		char airports[ 255 ];
		char stay[ 255 ];
		char member[ 255 ];
		char topics[ 255 ];
		char dump[ 255 ];
	
		// display each record in file
		//read the records from the file and insert them into contactspan table
	
		
		bzero(dump, 255);
		inClientFile >> dump;
		strncpy(name, dump+5, 20);
	
		bzero(dump, 255);
		inClientFile >>  dump;
		strncpy(email, dump+6, 20);
	
		bzero(dump, 255);
		inClientFile >>  dump;
		strncpy(nationality, dump+12, 20);
	
		bzero(dump, 255);
		inClientFile >>  dump;
		strncpy(studies, dump+8, 20);
	
		bzero(dump, 255);
		inClientFile >>  dump;
		strncpy(languages, dump+10, 20);
	
		bzero(dump, 255);
		inClientFile >>  dump;
		strncpy(affiliation, dump+12, 20);
	
		bzero(dump, 255);
		inClientFile >>  dump;
		strncpy(position, dump+9, 20);
	
		bzero(dump, 255);
		inClientFile >>  dump;
		strncpy(city, dump+5, 20);
	
		bzero(dump, 255);
		inClientFile >>  dump;
		strncpy(country, dump+8, 20);
	
		inClientFile >> dump;
	
		bzero(dump, 255);
		inClientFile >>  dump;
		strncpy(airports, dump+9, 20);
	
		inClientFile >> dump;
		inClientFile >> dump;
	
		bzero(dump, 255);
		inClientFile >>  dump;
		strncpy(stay, dump+5, 20);
	
		inClientFile >> dump;
		inClientFile >> dump;
		inClientFile >> dump;
	
		bzero(dump, 255);
		inClientFile >>  dump;
		strncpy(member, dump+7, 20);
	
		bzero(dump, 255);
		inClientFile >>  dump;
		strncpy(topics, dump+7, 50);
	
		// constructor: creating a row of data.
		interests row(name, email, nationality, studies, languages, affiliation, position, city, country, airports, stay, member, topics);
	
		// Form the query to insert the row into the stock table.
		mysqlpp::Query query = conn.query();
		query.insert(row);
		// Show the query about to be executed.
		cout << "Query: " << query << endl;
	
		// Execute the query.  We use execute() because INSERT doesn't
		// return a result set.
		query.execute();
		counter++;
	}
	 // ifstream destructor closes the file
	return 0;	
}
//this function fills the output files with average contact duration, no of contacts, and distance between every pair of nodes in [t0,t1] time interval. social: 0tccalsn, where n:nationality, s: study, l:language, a: affiliation, c: city, c: country, and t: topic -- represents the social aspects that we are interested to look at. if 0: means ignore and 1: means take into account.
//input args: countM which is couting method -->
//0: the old way of couting contacts which DONOT consider the mutual sighting issue
//1: the new way of couting contacts which DO consider the mutual sighting issue
int parser::generate_traces(int t0, int t1, unsigned char social, int countM){
   distr dist(config);
   // ofstream constructor opens file                 
   //we save all of the (no of contacts, closeness) here	
   // we use plot.dat for ploting the desired data
   ofstream outFile_plot( "/home/kazem/Desktop/traces/mobility/plot.dat", ios::out );
   //we use mat.dat for matlab purpose
   ofstream outFile_mat( "/home/kazem/Desktop/traces/mobility/mat.dat", ios::out );
   //let's save the pairs as well
   ofstream outFile_pair( "/home/kazem/Desktop/traces/mobility/pairs.dat", ios::out );

	
  // exit program if unable to create file
	
  if ( !outFile_plot ) // overloaded ! operator
  {
    cerr << "File plot.dat could not be opened" << endl;
    return -1;
  } // end if

  if ( !outFile_mat ) // overloaded ! operator
  {
    cerr << "File mat.dat could not be opened" << endl;
    return -1;
  } // end if

  if ( !outFile_pair ) // overloaded ! operator
  {
    cerr << "File pairs.dat could not be opened" << endl;
    return -1;
  } // end if

  int* null_rows;	//list of null data
  int null_num = 0;	//number of null rows

  mysqlpp::Connection conn(false);
	
  // Connect to database 
  if (conn.connect("infocom06", "localhost", "root", "mysql")) {
		// Retrieve a subset of the sample stock table set up by resetdb
		// and display it
		cout << "Connected to db!" << endl;
		char _query [200];
		bzero(_query, 200);
		//let's find the lists of people who didnt fill the survey and exclude them
		sprintf (_query, "select name from interests where affiliation='' and nationality=''");
		//printf ("the query: %s\n", _query);

		mysqlpp::Query query = conn.query(_query);
		
		if (mysqlpp::StoreQueryResult res = query.store()) {
			null_rows = (int *) malloc(res.num_rows());
			null_num = res.num_rows();
			cout << "We have:" << endl;
			for (size_t i = 0; i < res.num_rows(); ++i) {
				null_rows[i] = atoi(res[i][0]);
				cout << '\t' << null_rows[i] << endl;
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


  int flag = 0;
  for (int i = 21; i <= 99; i++ ){
  
	flag = 0;
	for(int t = 0; t < null_num; t++){
		if (i == null_rows[t])	{
			flag = 1;
			//cout << "i: " << i << endl;
			break;
		}
	} 
	
	if(flag == 1)	continue;

  for (int j = i+1; j <= 99; j++ ){

  if (i != j){
  //exclude rows with null data in interests table
  flag = 0;
  for(int t = 0; t < null_num; t++)
	if (j == null_rows[t]){
		flag = 1;
		//cout << "j: " << j << endl;
		break;
	}
  
  if(flag == 1)	continue;

  char id1[ 10 ];
  char id2[ 10 ];
  char interest[ 20 ];
  bzero(id1, 10);
  bzero(id2, 10);
  sprintf(id1,"%d", i); 
  sprintf(id2,"%d", j);
  double dotproduct;
  double odistance, ndistance;	//old and new social distance

double rand = get_rand();
//let's assign social distances randomly!
//dotproduct = rand;
distr dist_(config);
dotproduct = dist_.closeness(id1, id2, social);	//social similarity between two nodes

odistance = 1.0/dotproduct;	//social distance: why this way?!!!
ndistance = dist_.socialdist ( id1,id2, social );	//this is the actual distance
double prob_f = 1.0/ndistance;	//rank based friendship based on Nowell and Kleinberg model, the main question is that 1/dist^q for which q results the best prediction for mobility
// double prob_f2, prob_f3, prob_f4, prob_f5;
// //Q: what is the best exponent to have the highest correlation between nodes distance and their mobility!!?
// prob_f2 = pow(prob_f,2.0);
// prob_f3= pow(prob_f,1.5);
// prob_f4= pow(prob_f,0.5);
// prob_f5= pow(prob_f,0.1);

int no_cont = 0;
int cont_dur = 0;

if(countM){
//let's use count the real no of contact and contact duartion
//the new method of counting contacts between two nodes consider the issue of mutual sighting
dist.real_no_of_contacts(id1, id2,t0, t1);
map<int, int>::iterator ii=dist.contact.begin();
no_cont = (*ii).first;
cont_dur = (*ii).second;
}
else{
//old method of counting contacts between two nodes DONOT consider the issue of mutual sighting
//moreover, it only counts the contact recorded by node id1
no_cont = dist.no_of_contacts(id1, id2, t0, t1, 1);	//--> aggregate contacts
cont_dur = dist.contact_duration(id1,id2, t0, t1);	//we have to compare the total contact duration with contact no. we cannot look at average becuse cont_dur/no_cont and no_cont have much smaller correlation -->misleading
}

//cout << "no contacts between: " << id1 << " " << id2 << " " << no_cont << " " << cont_dur << endl;

// int conproduct = 1;	//contact product
// conproduct *= dist.no_of_contacts(id1, t0, t1, 1);	//let's look at total contact rate, later we can look at the unique contact rate to see if we can find something interesting
// conproduct *= dist.no_of_contacts(id2, t0, t1, 1);

//let's find the no of contact of each node as well
// int no_cnt1 = dist.no_of_contacts(id1,t0, t1, 0);	
// int no_cnt2 = dist.no_of_contacts(id2, t0, t1, 0);
// 
// double p = (double)no_cont/(double)(no_cnt1 + no_cnt1 - no_cont);

if(dotproduct > 0.0 && prob_f > 0.0){
//  cout << "dotproduct(" << id1 << "," << id2 << "):" << dotproduct << endl;
  if(cont_dur > 0 && no_cont > 0)
//	if (cont_dur >= 3600) 
	outFile_mat << cont_dur << " " << no_cont << " " << ndistance << " " << dotproduct << " " << prob_f << ';';

  if(cont_dur > 0 && no_cont > 0)
//  	if (cont_dur >= 3600) 
	outFile_plot << cont_dur << " " << no_cont << " " << ndistance << " " << dotproduct << " " << prob_f << endl;//" " << prob_f2 << " " << prob_f3 << " " << prob_f4 << endl;

if(cont_dur > 0 && no_cont > 0)
//	if (no_cont >= 10) 
	outFile_pair <<"(" << id1 << "," << id2 << ")" << " " << cont_dur << " " << no_cont << " " << dotproduct << " " << prob_f << endl;
}
 }// end if
 }//end for
}//end for
//let's close the trace files

outFile_plot.close();
outFile_mat.close();
outFile_pair.close();
}
