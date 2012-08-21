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
#include "weakcond.h"

// typedef std::tr1::ranlux64_base_01 Myeng; 
//the strategy is 0: for utilizing all contacts and 1 for not utilizaing all contacts
//wc: true if we want to utilize contacts for content spreading and false otherwise
//prob: probability to not forego a contact
int weakcond::spreadNews(int strategy, bool wc, double NFG_prob){
	list<Node>::iterator it1, it2;
	list<message>::iterator it_1, it_2;
	//open the contact file
	ifstream contactFile("/home/kazem/Desktop/traces/mobility/test.dat", ios::in);
	
	if (!contactFile) {
		cerr << "Unable to open file contacts.dat";
		return -1;
	}

	int id1, id2;
	double stime, etime;
	srand ( time(NULL) );	//seed for random generator
	double nextCTime = 0.0;	//the next contact happens at this time
	double nextUTime = 0.0;	//the next content update is going to be scheduled at this timestep
	size_t inx = 0;
	double t_id1, t_id2;
	bool cond = false;	//this condition tells us if we should forego the current contact or not
	message msg;
	setupDB();	//let's setup the database
	//let's read the first contact
	if(inx < queryRes.num_rows()){
		stime = queryRes[inx][2];	//starting time
		etime = queryRes[inx][3];	//end time
		id1 = queryRes[inx][0];	//user1
		id2 = queryRes[inx][1];	//user2
		inx++;
	}
 	nextCTime = stime;	//update the next contact time

	while(true)
	{
		if(SS_flag == 0 && clock >= t_wup){
			
			SS_flag = 1;	//from now on we start computing the maximu expected age (MEA)
			init_MEA();	//let's initialize all required data structures
		}
		if(clock > tEnd)	break;	//end of simulation
		
		if(wc && nextCTime < nextUTime){
			//the next event which has to be scheduled is a new contact
			log_file << id1 << " contacts " << id2 << " @ " << stime << std::endl;
			clock = nextCTime;	//advance the clock
			totalCont++;		//a new contact
			//let's find id1's msg timestamp
			it1 = nodes.begin();
			for ( int i = 0 ; i < id1; i++ )	it1++;
			it_1 = (*it1).buffer_list.begin();

			//let's find id2's msg timestamp
			it2 = nodes.begin();
			for ( int i = 0 ; i < id2; i++ )	it2++;
			it_2 = (*it2).buffer_list.begin();

			//if we are in steady state, then let's start updating our list for neighbors, NFg and Fg lists
			if(SS_flag == 1 && strategy == 1)	cond = update_Lists(it1, it2);

			double rnd_ = (double) random()/ (double) RAND_MAX;	//the probability that we not forego a contact
			if(SS_flag == 1 && (cond || (strategy == 0  && rnd_ <= NFG_prob))){
				utilCont++;	//no of conatcts whih have been used: it reflects the Bw usage
				if (strategy == 1) log_file << "Not Forego the contact " << id1 << " & " << id2 << " @ " << stime << std::endl;
				//we should utilize this contact
				if(it_1 != (*it1).buffer_list.end())
					t_id1 = (*it_1).timestamp;
				else	t_id1 = -1.0;
					
				if(it_2 != (*it2).buffer_list.end())
					t_id2 = (*it_2).timestamp;
				else
					t_id2 = -1.0;
				
				if(t_id1 < t_id2){
					log_file << "content-update in " << id1 << " by " << id2 << " @ " << clock << " from " << t_id1 << " to " << t_id2 << std::endl;
					if(t_id1 == -1.0){
						msg.timestamp = t_id2;	//set the timestamp as the id2's ts
						(*it1).buffer_list.push_back(msg);	//insert the message
					}else	(*it_1).timestamp = t_id2;
					if(SS_flag == 1)	compute_EA(id1, t_id2);	//pass  node id and its new age
				}else if(t_id1 > t_id2){
					log_file << "content-update in " << id2 << " by " << id1  << " @ " << clock << " from " << t_id2 << " to " << t_id1 << std::endl;
					if(t_id2 == -1.0){
						msg.timestamp = t_id1;	//set the timestamp as the id1's ts
						(*it2).buffer_list.push_back(msg);	//insert the message
					}else	(*it_2).timestamp = t_id1;
					if(SS_flag == 1)	compute_EA(id2, t_id1);	//pass  node id and its new age
				}else if(t_id1 == t_id2 && t_id1 != -1.0)
					log_file << id1 << " has the same ts as " << id2 << std::endl;
				else	log_file << "no nodes has any content yet!" << std::endl;
			}else if(SS_flag == 1 && (cond == false && strategy == 1)) log_file << "Do Forego the contact " << id1 << " & " << id2 << " @ " << stime << std::endl;
			//read the next contact
			if(inx < queryRes.num_rows()){
				stime = queryRes[inx][2];	//starting time
				etime = queryRes[inx][3];	//end time
				id1 = queryRes[inx][0];	//user1
				id2 = queryRes[inx][1];	//user2
				inx++;
			}else break;
			nextCTime = stime;	//update the next contact time
		}else{
			//the next event is a content update by Service Provider
			clock = nextUTime;	//advance the clock
			//pick a random node id in the range of [0,nodeNum]
			double rand_ = (double) random()/ (double) RAND_MAX;
			int shift = ceil(config->node*rand_) + config->s_nid - 1;
// 			log_file << "random node: " << shift << std::endl;
			//the next event to be scheduled is a new content update
			log_file << "content-update in " << shift << " by SP @ " << clock << std::endl;
			//the random node which we have to update its content
			it1 = nodes.begin();
			for ( int i = 0 ; i < shift; i++ )	it1++;
			it_1 = (*it1).buffer_list.begin();
			if(it_1 == (*it1).buffer_list.end()){
				msg.timestamp = clock;	//set the timestamp as the current time
				(*it1).buffer_list.push_back(msg);	//insert the message
			}else	(*it_1).timestamp = clock;
			//let's generate the next content
			double ia_time = -(1.0/rate) * log((double) random()/ (double) RAND_MAX);	//inter arrival time
			nextUTime += ia_time;
// 			log_file << "exp: " << ia_time << std::endl;
			if(SS_flag == 1)	compute_EA(shift, clock);	//pass  node id and its new age
		}
	}

	contactFile.close();
	//now let's calculate the Maximum Expected Age (MEA)
	compute_MEA();
	log_file << "total contacts: " << totalCont << ", utilized contacts: " << utilCont << std::endl;
	//let's test several nodes forego and contact lists
	if (strategy == 1)
	for(int i = 0; i <= 10; i++){
		double rand_ = (double) random()/ (double) RAND_MAX;
		int shift = ceil(config->node*rand_) + config->s_nid - 1;	
		print_Lists(shift);
	}
	return 0;
}


int weakcond::setupDB(){
	mysqlpp::Connection conn(false);

	/* Connect to database */
	if (conn.connect(config->db, "localhost", "root", "mysql")) {
		// Retrieve a subset of the sample stock table set up by resetdb
		// and display it
		//log_file << "Connected to db!" << endl;
		
		char _query [250];
		bzero(_query, 250);
		//we should count all contacts: not only long ones!!!
		sprintf (_query, "select user1, user2, starttime, endtime from %s where starttime >= %f and starttime <= %f and user1 <= %d and user1 >= %d and user2 <= %d and user2 >= %d order by starttime", config->table, tStart, tEnd, config->e_nid, config->s_nid, config->e_nid, config->s_nid);

		mysqlpp::Query query = conn.query(_query);
		if (queryRes = query.store()) {
			
			log_file << "DB is setup successfully" << std::endl;
			return 0;
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
//initialize all required data structure for comouting expected age of each node during the system operation
int weakcond::init_MEA(){
	list<Node>::iterator it;
	list<message>::iterator it_;

	//set the EA array to all zeros
	for(int i = config->s_nid; i < config->node; i++)	EA[i] = 0.0;

	//we have to compute Yprev and Tprev for all nodes
	int inx = 0;
	for ( it = nodes.begin() ; it != nodes.end(); it++ ){
		it_ = (*it).buffer_list.begin();	//find this nodes buffer
		if(it_ == (*it).buffer_list.end()){
			log_file << "there is no update for node " << inx << " until warm up" << std::endl;
		}else{
			Yprev[inx] = clock - (*it_).timestamp;	//the age of this node's message
			Tprev[inx] = clock;			//we start calculating TA from now on
		}
		inx++;
	}
	return 0;
}
//this method is called whenever there is an update by SP or another node for the specific node. Then, this methods will update the expected age of the corresponding node through computing the area under Y[i] curve
int weakcond::compute_EA(int id, double age_){
	//update the expected age for id
	EA[id] += 0.5*(2.0*Yprev[id] + clock - Tprev[id])*(clock - Tprev[id]);
	Tprev[id] = clock;
	Yprev[id] = clock - age_;
	return 0;
}

int weakcond::compute_MEA(){
	//update the expected age for id
	MEA = 0.0;
	double EEA = 0;
	int id = 0;
	for(int i = config->s_nid; i < config->node; i++){
		EEA += EA[i]/(clock - t_wup);
		if(EA[i] > MEA){
			MEA = EA[i];
			id = i;
		}
		log_file << "EA of node " << i << " is " << EA[i]/(clock - t_wup) << std::endl;
	}
	MEA = MEA/(clock - t_wup);
	log_file << "Maximum Expected Age of the system is " << MEA << " and belong to " << id << std::endl;
	log_file << "Expected of Expected Age of the system is " << EEA/(double)config->node << std::endl;

	return 0;
}

bool weakcond::update_Lists(std::_List_iterator<Node> it_1, std::_List_iterator<Node> it_2){
	int id_1 = (*it_1).id;		// --> u's id
	int id_2 = (*it_2).id;		//--> v's id
	map<int,int>::iterator it_n;
	bool cnd_1 = false, cnd_2 = false;
	
	if((*it_1).Ngh.find(id_2) == (*it_1).Ngh.end()){
		//v is not in u's neighbor set
		(*it_1).Ngh.insert ( pair<int,int>(id_2,1) );
		//if v is not in u's forego list, then add it to u's not forego list
		if((*it_1).Forego.find(id_2) == (*it_1).Forego.end()){
			(*it_1).nForego.insert ( pair<int,int>(id_2,1) );
// 			cnd_1 = true;
		}
	}
	//let's add any node like w in Nv - Nu to Nu's Forego list: those nodes that u has learnt about them through v
	for ( it_n = (*it_2).Ngh.begin() ; it_n != (*it_2).Ngh.end(); it_n++ )
		if((*it_1).Ngh.find((*it_n).first) == (*it_1).Ngh.end())
			(*it_1).Forego.insert ( pair<int,int>((*it_n).first,1) );

	if((*it_2).Ngh.find(id_1) == (*it_2).Ngh.end()){
		//u is not in v's neighbor set
		(*it_2).Ngh.insert ( pair<int,int>(id_1,1) );
		//if u is not in v's forego list, then add it to v's not forego list
		if((*it_2).Forego.find(id_1) == (*it_2).Forego.end()){
			(*it_2).nForego.insert ( pair<int,int>(id_1,1) );
// 			cnd_2 = true;
		}
	}
	//let's add any node like w in Nu - Nv to Nv's Forego list: those nodes which v has learnt about them through u
	for ( it_n = (*it_1).Ngh.begin() ; it_n != (*it_1).Ngh.end(); it_n++ )
		if((*it_2).Ngh.find((*it_n).first) == (*it_2).Ngh.end())
			(*it_2).Forego.insert ( pair<int,int>((*it_n).first,1) );
	
	//if one of nodes are not in the Forego list of the other one, use this contact
	if((*it_1).Forego.find(id_2) == (*it_1).Forego.end() || (*it_2).Forego.find(id_1) == (*it_2).Forego.end())
		return true;
	else return false;
}
//this methods prints the forego, not forego, and neighborhood set of a given node
int weakcond::print_Lists(int id_){
	map<int,int>::iterator it_m;
	list<Node>::iterator it;

	it = nodes.begin();
	for ( int i = 0 ; i < id_; i++ )	it++;

/*	std:log_file << "Forego list of node: " << id_ << std::endl;
	for ( it_m = (*it).Forego.begin() ; it_m != (*it).Forego.end(); it_m++ )
		log_file << (*it_m).first << " " ;
	log_file << std::endl;*/
	
	log_file << "Neighborhood set of node: " << id_ << std::endl;
	for ( it_m = (*it).Ngh.begin() ; it_m != (*it).Ngh.end(); it_m++ )
		log_file << (*it_m).first << " " ;
	log_file << std::endl;
	
	log_file << "Not Forego list of node: " << id_ << std::endl;
	for ( it_m = (*it).nForego.begin() ; it_m != (*it).nForego.end(); it_m++ )
		log_file << (*it_m).first << " " ;
	log_file << std::endl;
}
//generate all messages for all nodes
int weakcond::setupMsgs(double time){
	list<Node>::iterator it;
	//let's go through traffic profile to create the messages and push them into buffers of nodes
	message msg;
	int id = config->s_nid;
	for (it = nodes.begin(); it != nodes.end(); it++){
		msg.id = id;	//set message id equal to node id
		msg.sid = (*it).id;	//the sender id
		msg.gen_t =  time;	//copy the transmission time
		(*it).buffer_list.push_back(msg);	// Insert a new message at the end of the coressponding node
		log_file << "node " << (*it).id << " has msg id " << id << std::endl;
		id++;
	}
	return 0;
}

int weakcond::spreadMsgs(int strategy){
	setupMsgs(0.0);	
	list<Node>::iterator it1, it2;
	list<message>::iterator it_1, it_2;

	int id1, id2;
	double stime, etime;
	srand ( time(NULL) );	//seed for random generator
	size_t inx = 0;
	double t_id1, t_id2;
	bool cond = false;	//this condition tells us if we should forego the current contact or not
	message msg;
	map<int,int> done_nodes;	//those nodes which are done and they have all other nodes' messages
	map<int,int> comm;
	int print_once[] = {1, 1, 1, 1};
	//for real data
// 	int comms[] = {0, 32, 3, 4, 5, 1, 8, 9, 12, 13, 14, 16, 17, 18, 19, 22, 29};
// 	int comms_len = 17;
//   	int comms[] = {0, 32, 3, 4, 5, 1, 8, 9, 12, 14, 17, 18, 19, 22, 29};
//  	int comms_len = 15;

// 	int comms[] = {33, 2, 35, 6, 10, 11, 15, 20, 21, 24, 26, 27, 28, 31};
//   	int comms[] = {33, 2, 35, 6, 10, 11, 15, 20, 21, 24, 27, 28, 31};
//   	int comms_len = 13;

// 	int comms[] = {0, 32, 3, 4, 5, 1, 8, 9, 12, 14, 17, 18, 19, 22, 29, 33, 2, 35, 6, 10, 11, 15, 20, 21, 24, 26, 27, 28, 31};
// 	int comms_len = 28;
// 	for (int i = 0; i < comms_len; i++)	comm.insert( pair<int,int>(comms[i]+1,1) );

	for (int i = 0; i < config->node; i++)	comm.insert( pair<int,int>(i,1) );

	//let's find the time we start to spread info
	int start_flag = 0;
	int start_spread = 0;

	setupDB();	//let's setup the database
	//let's read the first contact
	if(inx < queryRes.num_rows()){
		stime = (double)queryRes[inx][2];	//starting time
		etime = (double)queryRes[inx][3];	//end time
		id1 = queryRes[inx][0];	//user1
		id2 = queryRes[inx][1];	//user2
		inx++;
	}

	while(true)
	{
		clock = stime;	//advance the clock

		if(clock >= t_wup && comm.find(id1) != comm.end() && comm.find(id2) != comm.end()){
		//the next event which has to be scheduled is a new contact
		log_file << id1 << " contacts " << id2 << " @ " << stime << std::endl;
		
		totalCont++;		//a new contact

		//let's find id1's iterator
		it1 = nodes.begin();
		for ( int i = config->s_nid ; i < id1; i++ )	it1++;	//id1's iterator

		//let's find id2's iterator
		it2 = nodes.begin();
		for ( int i = config->s_nid ; i < id2; i++ )	it2++;	//id2's iterator
	
		//read id1's and id2's buffers to see which messages should be copied to the other node's buffer
		int copy_flag = 0;	// to see if we should copy anything from the buffer of id1

		//let's go through id1.buffer to see if there is any msg to be copied from ID1--> ID2
		for(it_1 = (*it1).buffer_list.begin(); it_1 != (*it1).buffer_list.end(); ++it_1){
			copy_flag = 0;
			for(it_2 = (*it2).buffer_list.begin(); it_2 != (*it2).buffer_list.end(); ++it_2){		
				if ((*it_1).id == (*it_2).id){
					copy_flag = 1;	//this message has already been found in id2's buffer
					break;
				}
			}
			if(copy_flag == 0 && (*it_1).gen_t <= tEnd){
				log_file << "copy msg " << (*it_1).id << ": from: " << id1 << " to: " << id2 << "'s buffer" << endl;
				msg.id = (*it_1).id;
				msg.sid = (*it_1).sid;;
				msg.gen_t = (*it_1).gen_t;
				// copy the message to the end of the id2's buffer!	
				(*it2).buffer_list.push_back(msg);	
			}
		}
		copy_flag = 0;
		//let's go through id2's buffer to see if there is any msg to be copied from ID2--> ID1
		for(it_2 = (*it2).buffer_list.begin(); it_2 != (*it2).buffer_list.end(); ++it_2){
			copy_flag = 0;
			for(it_1 = (*it1).buffer_list.begin(); it_1 != (*it1).buffer_list.end(); ++it_1){		
				if ((*it_2).id == (*it_1).id){
					copy_flag = 1;	//this message has already been found in id1's buffer
					break;
				}
			}					
			if(copy_flag == 0 && (*it_2).gen_t <= tEnd){
				log_file << "copy msg " << (*it_2).id << ": from: " << id2 << " to: " << id1 << "'s buffer" << endl;
				msg.id = (*it_2).id;
				msg.sid = (*it_2).sid;;
				msg.gen_t = (*it_2).gen_t;
				// copy the message to the end of the id1's buffer!	
				(*it1).buffer_list.push_back(msg);	
			}
		}
		//let's see if id1 has got all messages
		if(done_nodes.find(id1) == done_nodes.end() && (*it1).buffer_list.size() == comm.size()){
			log_file << "node " << id1 << " is done after " << clock - (tStart + t_wup) << " steps!" << std::endl;
			done_nodes.insert ( pair<int,int>(id1,1) );
		}
		//let's see if id2 has got all messages
		if(done_nodes.find(id2) == done_nodes.end() && (*it2).buffer_list.size() == comm.size()){
			log_file << "node " << id2 << " is done after " << clock - (tStart + t_wup) << " steps!" << std::endl;
			done_nodes.insert ( pair<int,int>(id2,1) );
		}
		//let's see if all message has spread all over
		if(done_nodes.size() == comm.size()/*config->node*/){
			log_file << "Simulation finished after " << clock - (tStart + t_wup) << " seconds!" << std::endl;
			log_file << "total utilized contacts: " << totalCont << std::endl;
			break;
		}
		if(id1 == watched_id){
			for(int c_id = 0; c_id < 4; c_id++)
			if(print_once[c_id] == 1)	print_once[c_id] = has_received(watched_id, c_id, 0, 100);
		}
// 		log_file << "node: " << id1 << " has " << (*it1).buffer_list.size() << " msgs after " << clock - tStart << std::endl;
// 		log_file << "node: " << id2 << " has " << (*it2).buffer_list.size() << " msgs after " << clock - tStart << std::endl;
		}
		//read the next contact
		if(inx < queryRes.num_rows()){
			stime = (double)queryRes[inx][2];	//starting time
			etime = (double)queryRes[inx][3];	//end time
			id1 = queryRes[inx][0];	//user1
			id2 = queryRes[inx][1];	//user2
			inx++;
		}else break;
	}
}

int weakcond::has_received(int id, int comm_id, int no_bri, int comm_size){
	list<message>::iterator it_;
	list<Node>::iterator it;

	int id_start = no_bri + comm_id*comm_size;
	int id_end = id_start + comm_size - 1;
	int flag_one = 0;
	int flag_all = 1;

	//let's find id's iterator
	it = nodes.begin();
	for ( int i = config->s_nid ; i < id; i++ )	it++;	//id's iterator

	for(int z = id_start; z <= id_end; z++){
		for(it_ = (*it).buffer_list.begin(); it_ != (*it).buffer_list.end(); ++it_){
			if ((*it_).id == z){
				flag_one = 1;	//this message has been found in id1's buffer
				break;
			}
		}
		if(flag_one == 0){
			//id1 doesnt have all messages in corresponding comm!
			flag_all = 0;
			break;
		}
		else	flag_one = 0;	//let's check the next message
	}
	if(flag_all == 1){
		log_file << "node " << id << " has received all msgs in community " << comm_id + 1 << " in " << clock - (tStart + t_wup) << " steps!" << std::endl;
		return 0;
	}else	return 1;
}

int weakcond::set_watched_ids(int id){
	watched_id = id;
	return 0;
}
