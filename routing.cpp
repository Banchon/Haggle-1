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
#include "routing.h"
#include <string.h>

using std::ifstream; // input file stream

using namespace std;

//0: WAIT, 1: flooding, 2: greedy, 3: MCP, 4: LABEL
#define DEBUG 1

routing::routing(int TTL_val, int ts_val, configuration *cfg){
// 	node = nodes_val;//79;	//no of nodes in the network. node id can be anything between 21 and 99
 	TTL = TTL_val;	// what is the total TTL for all traffic in seconds : 9 hours!
 	ts = ts_val;	//start time of simulation
	te = ts_val + TTL_val;	//setting starting time and ending time for the emulation: 9:00AM on 24th --> 57600 
	relayed_msgs = 0;
	//node 99 does not have any contact in db, the rest donot have any social profile
	int excluded[16]  = { 29, 33, 38, 47, 50, 58, 61, 63, 78, 79, 85, 90, 91, 92, 94, 99};
	for(int i=0; i < 16; i++)
		non_addressed[i] = excluded[i];
	//let's setup the configuration for contactG class
	config = cfg;
	cg.config = cfg;
}
//we use nfr only for random scheme
//flag: the second arg denotes how to traffic should be generated vs time
//0: --> uniform distribution
//1: --> all msgs have the same generation time equal to ts
//excl: 0:--> no exclusion , 1: exclude forbidden nodes
int routing::create_traffic(int no_msgs_val, int flag, int excl){
no_msgs = no_msgs_val;
te = ts + TTL;	//let's update the te every time that we run with new TTL
 //let's in the first step generate the traffic for no_msg no of videos
 traffic t_prof(0, 0, 0);
//list of external nodes
int extN[20] = {22, 32, 36, 42, 47, 49, 52, 58, 61, 64, 68, 70, 71, 77, 81, 82, 84, 87, 93, 96};
//int num = 20;	//no of external nodes
//cg.genExt(0);	//generate external node lists

//here we generate the sender and receiver id and the generation time for traffic in random and then push the traffic in the corresponding position in link list array

 for (int i = 0; i < no_msgs; i++){
	int accepted = 0;

	int sid, rid;

	while(accepted == 0){
		sid = ceil(config->node*get_rand()) + config->s_nid - 1;	//sender id
		accepted = 1;
		if(excl){
			for (int j = 0; j < 15; j++){
				if(sid == non_addressed[j]){
					accepted = 0;
					//cout << "sid:" << sid << " is accepted!" << endl;
					break;
				}
			}
		}
	}

	//cout << "sid: " << sid << endl;
	accepted = 0;
	while(accepted == 0){
		rid = ceil(config->node*get_rand()) + config->s_nid - 1;	//receiver id
		while (rid == sid) rid = ceil(config->node*get_rand()) + config->s_nid - 1;	//rid has to be different from sid
		accepted = 1;
		if(excl){
			for (int j = 0; j < 15; j++){
				if(rid == non_addressed[j]){
					accepted = 0;
					//cout << "rid:" << rid << " is not accepted!" << endl;
					break;
				}
			}
		}
	}

	//let's modify the sender and receiver
	//we are trying to enumerate paths between external nodes
	// sid = extN[(int)ceil(num*get_rand()) - 1];
	// rid = extN[(int)ceil(num*get_rand()) - 1];
	// while(rid == sid)	rid = extN[(int)ceil(num*get_rand()) - 1];
	//cout << rid << endl; 

	//for testing social profiles
	int socNum = 39;
	int socialN[39] = {22, 23, 25, 26, 27, 28, 30, 31, 34, 35, 36, 37, 39, 40, 41, 45, 46, 49, 55, 59, 64, 65, 66, 67, 68, 69, 70, 71, 72, 74, 76, 77, 81, 82, 83, 87, 93, 96, 97};
	
	//int socialN[30] = {21, 24, 30, 32, 34, 37, 39, 41, 46, 48, 51, 54, 60, 62, 64, 66, 68, 69, 70, 71, 72, 74, 76, 82, 86, 87, 88, 89, 95, 96};
	
	//let's modify the receiver: for social profiles testing
//  	rid = socialN[(int)ceil(socNum*get_rand()) - 1];
//  	while(rid == sid)	rid = socialN[(int)ceil(socNum*get_rand()) - 1];

	int time;
	if(flag)	time = ts;	//the generated time for the messages
	else		time = ceil((te - ts)*get_rand()) + ts;	//trasnmission time: is uniformly distributed

	t_prof.sndr_id = sid;
	t_prof.rcvr_id = rid;
	t_prof.gen_t = time;
	t_prof.id = i;	//traffic id
//	t_prof.nfr = costs[i];	//set the cost

	traf_list[sid-config->s_nid].push_back(t_prof);  // Insert a new traffic at the end of the coressponding node
	//cout << "size: " << (int) traf_list[sid-21].size() << endl;
	curr_cost.insert ( pair<int,int>(i, 0) );	//every message has cost zero initially
}
list<traffic>::iterator k;	// an iterator for the link list


 //let's go through traffic profile to create the messages and push them into buffers of nodes
 message msg; 
 int id = 0;	//message ids
 for (int j = 0; j < config->node; j++){
 	for(k = traf_list[j].begin(); k != traf_list[j].end(); ++k){
		msg.id = (*k).id;	//set message id
		msg.sid = (*k).sndr_id;	//copy the sender id
		msg.rid = (*k).rcvr_id;	//copy the receiver id
		msg.gen_t =  (*k).gen_t;	//copy the transmission time
		msg.gRank = true;	//for Bubble scheme
		msg.maxSim = 0;	//GreedyB: the last node with the hisghest similarity to dest
		msg.hopttl = hopttl;	//hop ttl
		msg.mcp = mcp;	//no of copies
		msg.delivered = false;	//msg has not been delivered yet
		msg.relay_list.push_back((*k).sndr_id);	//let's push the sid to the beginnig of the path
		nodes[j].buffer_list.push_back(msg);	// Insert a new message at the end of the coressponding node
		id++;
		msg.relay_list.clear();	//clear the list afterwards for the next message
 	}
 }

}

int routing::print_traffic(){
list<traffic>::iterator k;	// an iterator for the link list

 //let's print the total traffic scheme
 cout << "traffic profile before delivery:" << endl;
 for (int j = 0; j < config->node; j++)
 for(k = traf_list[j].begin(); k != traf_list[j].end(); ++k) cout << "node: " << (*k).sndr_id << " has traffic for node: " << (*k).rcvr_id << " at: " << (*k).gen_t  << endl; // print member
 cout << endl;
return 0;
}
//this method prints all nodes buffer contents
int routing::print_buffers(){

	//let's print the buffer contenets
	
	cout << "buffer contents: " << endl;
	list<message>::iterator l;
	for (int j = 0; j < config->node; j++)
	for(l = nodes[j].buffer_list.begin(); l != nodes[j].buffer_list.end(); ++l) 
		cout << "node: " << (*l).sid << " has message id: " << (*l).id << " for node: " << (*l).rid << " @ " << (*l).gen_t <<  endl; // print member
}
//I have found the bug with access to NULL place can be solved if I virtually add some messages to all nodes' buffers and then remove them. This is strange, but at the moment I dont have any explnation for it. The only thing that I observed was that even if node[0] donot have anytjing in its buffer node[0],buffer_list but the program inside different functions like timeout, print_buffer or cleanup() goes an tries to read from node 0's buffer which crashes the code naturally!!!
int routing::fixbug(){
	message msg;
	for(int j = 0; j < config->node; j++){
		nodes[j].buffer_list.push_back(msg);
		nodes[j].buffer_list.clear();
	}
}
//this method cleans up buffer and traffic profile link lists
int routing::cleanup(){
	list<traffic>::iterator it_s, it_e;	// an iterator for the link list
	
	//let's cleanup the total traffic scheme
	cout << "cleaning traffic profile and buffers:" << endl << endl;
	for (int j = 0; j < config->node; j++){
		if(!traf_list[j].empty()){
			it_s = traf_list[j].begin(); 
			it_e = traf_list[j].end();
			traf_list[j].erase (it_s,it_e);
		}
	}

	//let's cleanup the buffer contents
	
	list<message>::iterator ls, le;
	for (int j = 0; j < config->node; j++){
		if(!nodes[j].buffer_list.empty()){
			ls = nodes[j].buffer_list.begin(); 
			le = nodes[j].buffer_list.end();
			nodes[j].buffer_list.erase (ls,le);
		}
	}

	relayed_msgs = 0;	//restart relayed_msgs counter

	return 0;
}
//this method write the delay and cost into the corresponding files as well as profile
//it also prints the delivery ratio and total cost per each messgae
int routing::write_profile(ofstream& delay, ofstream &cost, int time, int scheme){
	list<traffic>::iterator k;	// an iterator for the link list
        int costs[1000];
	//let's store delay in profile.dat file
	ofstream profile( "/home/kazem/Desktop/traces/logs/profile.dat", ios::app );
		
	// exit program if unable to create file
		
	if ( !profile ) // overloaded ! operator
	{
		cerr << "File profile.dat could not be opened" << endl;
		return -1;
	} // end if
	profile << "Scheme: " << scheme <<", Simulation#" << time << ", TTL: " << TTL << endl;
	delay << "Scheme: " << scheme <<", Simulation#" << time << ", TTL: " << TTL << endl;
	cost << "Scheme: " << scheme <<", Simulation#" << time << ", TTL: " << TTL << endl;

	//let's print the total traffic scheme to see the delivered messages!
	int failed = 0;	//let's count delivery ratio!
	int cost_t = 0;	//total cost for all message delivery
	for (int j = 0; j < config->node; j++)
	for(k = traf_list[j].begin(); k != traf_list[j].end(); ++k){
		if((*k).success == 1){
			//if delivery is successful print the result
			profile << "msg:" << (*k).id << ", " << (*k).sndr_id << "--> " << (*k).rcvr_id << " generated@ " << (*k).gen_t << ", delivered@ " << (*k).rcv_t << ", delay=" << (double)(*k).delay/(double)3600 << " hours" << " no of hops: " << (*k).hops << ", cost = " << (*k).cost << endl; // print member
			delay << (double)(*k).delay/(double)3600 << "," ;
			cost << (*k).id << " " << (*k).cost << ",";
			costs[(*k).id] = (*k).cost;
			cost_t += (*k).cost;
		}
		else if ((*k).success == -1){	
			profile << "msg:" << (*k).id << ", " << (*k).sndr_id << " --> " << (*k).rcvr_id << " generated@ " << (*k).gen_t << " failed!" << endl;
			failed++;	//let's calculate the delivery ratio
			if((*k).cost > 0){
				cost << (*k).cost << ",";
				cost_t += (*k).cost;
				costs[(*k).id] = (*k).cost;
			}
			else
				costs[(*k).id] = 0;
		}	
	}//for	
	
	profile << endl << "-----------------------------------------------------------------------" << endl;
	delay << endl << "-----------------------------------------------------------------------" << endl ;
        cost << endl << "-----------------------------------------------------------------------" << endl;
	profile.close();

	ofstream out;
  	out.open ("/home/kazem/Desktop/traces/logs/out.dat", ios::app);
	double delivery_ratio = (double) (no_msgs - failed)/ (double) no_msgs;
	double cost_ratio = (double) cost_t/(double)no_msgs;
	out << "Scheme: " << scheme <<", Simulation#" << time << ", TTL: " << TTL << " , SDR: " << delivery_ratio*100 << " %" << " , COST: " << cost_ratio << endl;
	out.close();
/*
	for (int i=0; i<1000;i++)
		cout << costs[i] <<",";
	cout<<endl;
*/
} 

//this method goes through id1 and id2 buffers and remove all timeout messages
int routing::timeout(int id1, int id2, int current_time, ofstream &log){

	//let's check if user1 or user2 has video for the other one at a time <= stime, therefore we have to check the buffers of two users
	list<message>::iterator y, z, it1, it2, it3,it4;	// an iterator for the buffer link list
        list<traffic>::iterator k;	// an iterator for the link list

	int flag3 = 0;
	
	//let's go through id1's buffer to find expired message, we should update the traffic profile and remove those message from the buffer
	for(z = nodes[id1-config->s_nid].buffer_list.begin(); z != nodes[id1-config->s_nid].buffer_list.end(); ++z){
		if(flag3){
			//for(it4 = buffer_list[id1-21].begin(); it4 != buffer_list[id1-21].end(); ++it4)
			//	cout << (*it4).id << endl;
			if (DEBUG) log << "11remove message " << (*it3).id  << " from: " << id1 << endl;
			if (DEBUG) log << "size before remove: " << nodes[id1-config->s_nid].buffer_list.size() << endl;
			nodes[id1-config->s_nid].buffer_list.erase (it3);	
			if (DEBUG) log << "size after remove: " << nodes[id1-config->s_nid].buffer_list.size() << endl;
			//for(it4 = buffer_list[id1-21].begin(); it4 != buffer_list[id1-21].end(); ++it4)
			//	cout << (*it4).id << endl;

			flag3 = 0;
		}

		if (current_time - (*z).gen_t > TTL){
				int cost = 0;	//let's calculate the cost of timedout message
				for (int j = 0; j < config->node; j++)
				for(it1 = nodes[j].buffer_list.begin(); it1 != nodes[j].buffer_list.end(); ++it1){ 
					if(((*it1).id == (*z).id))
						cost += (*it1).cost;	
				}//end for

				flag3 = 1;	//we should remove this message from id1's buffer too
				it3 = z;	//save the position of timeout message
				if (DEBUG) log << "1msg: " << (*z).id << " " << (*z).sid << "-->" << (*z).rid << "@" << (*z).gen_t << endl;
				if (DEBUG) log << "1contact between: " << id1 << " and " << id2 << "time: " << current_time << endl;
				//msg of T12 has been expired already
				//Go through all node's buffer and find all entries for msg
				if (DEBUG) log << "*1node: " << id1 << "current time: " << current_time<< "message id: " << (*z).id << " timed out, let's clean up the buffers!" << endl;

				int flag2 = 0;
				for (int j = 0; j < config->node; j++){
				for(it1 = nodes[j].buffer_list.begin(); it1 != nodes[j].buffer_list.end(); ++it1){ 
					if(((*it1).id == (*z).id) && (j != (id1-config->s_nid))){
						it2 = it1;
						flag2 = 1;	//let's et flag for cleaning up!
						break;
					}else flag2 = 0;

				}//end for
				//Remove the msg from all nodes buffer
				if(flag2 && j != (id1-config->s_nid)){
					//cout << "*1remove msg id: " <<  (*z).id << " from buffer of node: " << j+21 << endl;
					nodes[j].buffer_list.erase (it2);	//remove the message from node j's buffer since it has been delivered
					flag2 = 0;
				}

				}//end for
				//update the corresponding entry in traffic profile
				//let's update the traffic profile table for delivered videos
				for(k = traf_list[(*z).sid-config->s_nid].begin(); k != traf_list[(*z).sid-config->s_nid].end(); ++k){ 
					if((*k).sndr_id == (*z).sid && (*k).rcvr_id == (*z).rid && (*k).gen_t == (*z).gen_t && (*k).id == (*z).id){
						//timeout happened, let's save the statistics
						(*k).cost += cost;	
						if (DEBUG) log << "[1]updating profile for timed out message id: " << (*z).id << endl; // print member
						if (DEBUG) log << "1flag3: " << flag3 << endl;
					}
				}
				
			}

	}
	//let's remove the timeout message from id1's buffer
	if(flag3){
		if (DEBUG) log << "size before remove: " << nodes[id1-config->s_nid].buffer_list.size() << endl;
		if (DEBUG) log << "12remove message " << (*it3).id  << " from: " << id1 << endl;
		nodes[id1-config->s_nid].buffer_list.erase (it3);
		if (DEBUG) log << "size after remove: " << nodes[id1-config->s_nid].buffer_list.size() << endl;
		flag3 = 0;
	}

	flag3 = 0;
	
	//let's go through id2's buffer to find expired message, we should update the traffic profile and remove those message from the buffer
	for(z = nodes[id2-config->s_nid].buffer_list.begin(); z != nodes[id2-config->s_nid].buffer_list.end(); ++z){

		if(flag3){
			if (DEBUG) log << "size before remove: " << nodes[id2-config->s_nid].buffer_list.size() << endl;
			if (DEBUG) log << "21remove message " << (*it3).id  << " from: " << id2 << endl;
			nodes[id2-config->s_nid].buffer_list.erase (it3);
			if (DEBUG) log << "size after remove: " << nodes[id2-config->s_nid].buffer_list.size() << endl;
			flag3 = 0;
		}

		if (current_time - (*z).gen_t > TTL){
				int cost = 0;	//let's calculate the cost of timedout message
				for (int j = 0; j < config->node; j++)
				for(it1 = nodes[j].buffer_list.begin(); it1 != nodes[j].buffer_list.end(); ++it1){ 
					if(((*it1).id == (*z).id))
						cost += (*it1).cost;	
				}//end for

				flag3 = 1;	//we should remove this message from id2's buffer too
				it3 = z;	//save the position of timeout message
				if (DEBUG) log << "2msg: " << (*z).id << " " << (*z).sid << "-->" << (*z).rid << "@" << (*z).gen_t << endl;
				if (DEBUG) log << "2contact between: " << id2 << " and " << id1 << "time: " << current_time << endl;
				//msg of T21 has been expired already
				//Go through all node's buffer and find all entries for msg
				if (DEBUG) log << "*2node: " << id2 << "current time: " << current_time<< "message id: " << (*z).id << " timed out, let's clean up the buffers!" << endl;

				int flag2 = 0;
				for (int j = 0; j < config->node; j++){
				for(it1 = nodes[j].buffer_list.begin(); it1 != nodes[j].buffer_list.end(); ++it1){ 
					if(((*it1).id == (*z).id) && (j != (id2-config->s_nid))){
						it2 = it1;
						flag2 = 1;	//let's et flag for cleaning up!
						break;
					}else flag2 = 0;
				}//end for
				//Remove the msg from all nodes buffer
					if(flag2 && j != (id2-config->s_nid)){
						//cout << "*2remove msg id: " <<  (*z).id << " from buffer of node: " << j+21 << endl;
						nodes[j].buffer_list.erase (it2);	//remove the message from node j's buffer since it has been delivered
						flag2 = 0;
					}

				}//end for
				//update the corresponding entry in traffic profile
				//let's update the traffic profile table for delivered videos
				for(k = traf_list[(*z).sid-config->s_nid].begin(); k != traf_list[(*z).sid-config->s_nid].end(); ++k){ 
					if((*k).sndr_id == (*z).sid && (*k).rcvr_id == (*z).rid && (*k).gen_t == (*z).gen_t && (*k).id == (*z).id){
						//timeout happened, let's save the statistics
						(*k).cost += cost;	
						if (DEBUG) log << "[2]updating profile for timed out message id: " << (*z).id << endl; // print member
						if (DEBUG) log << "2flag3: " << flag3 << endl;
					}
				}
				
			}

	}
	//let's remove the timeout message from id2's buffer
	if(flag3){
		if (DEBUG) log << "size before remove: " << nodes[id2-config->s_nid].buffer_list.size() << endl;
		if (DEBUG) log << "22remove message " << (*it3).id  << " from: " << id2 << endl;
		nodes[id2-config->s_nid].buffer_list.erase (it3);
		if (DEBUG) log << "size after remove: " << nodes[id2-config->s_nid].buffer_list.size() << endl;
		flag3 = 0;
	}
}

//this method goes through id1's buffers and remove all timeout messages
int routing::timeout(int id1, int current_time, ofstream &log){
	//let's check if user1 or user2 has video for the other one at a time <= stime, therefore we have to check the buffers of two users
	list<message>::iterator y, z, it1, it2, it3,it4;	// an iterator for the buffer link list
        list<traffic>::iterator k;	// an iterator for the link list

	int flag3 = 0;
	
	//let's go through id1's buffer to find expired message, we should update the traffic profile and remove those message from the buffer
	for(z = nodes[id1-config->s_nid].buffer_list.begin(); z != nodes[id1-config->s_nid].buffer_list.end(); ++z){
		if(flag3){
			//for(it4 = buffer_list[id1-21].begin(); it4 != buffer_list[id1-21].end(); ++it4)
			//	cout << (*it4).id << endl;
			if (DEBUG) log << "11remove message " << (*it3).id  << " from: " << id1 << endl;
			if (DEBUG) log << "size before remove: " << nodes[id1-config->s_nid].buffer_list.size() << endl;
			nodes[id1-config->s_nid].buffer_list.erase (it3);	
			if (DEBUG) log << "size after remove: " << nodes[id1-config->s_nid].buffer_list.size() << endl;
			//for(it4 = buffer_list[id1-21].begin(); it4 != buffer_list[id1-21].end(); ++it4)
			//	cout << (*it4).id << endl;

			flag3 = 0;
		}


		if (current_time - (*z).gen_t > TTL){
				int cost = 0;	//let's calculate the cost of timedout message
				for (int j = 0; j < config->node; j++)
				for(it1 = nodes[j].buffer_list.begin(); it1 != nodes[j].buffer_list.end(); ++it1){ 
					if(((*it1).id == (*z).id))
						cost += (*it1).cost;	


				}//end for

				flag3 = 1;	//we should remove this message from id1's buffer too
				it3 = z;	//save the position of timeout message
				if (DEBUG) log << "1msg: " << (*z).id << " " << (*z).sid << "-->" << (*z).rid << "@" << (*z).gen_t << endl;
//				if (DEBUG) log << "1contact between: " << id1 << " and " << id2 << "time: " << current_time << endl;
				//msg of T12 has been expired already
				//Go through all node's buffer and find all entries for msg
				if (DEBUG) log << "*1node: " << id1 << "current time: " << current_time<< "message id: " << (*z).id << " timed out, let's clean up the buffers!" << endl;

				int flag2 = 0;
				for (int j = 0; j < config->node; j++){
				for(it1 = nodes[j].buffer_list.begin(); it1 != nodes[j].buffer_list.end(); ++it1){ 
					if(((*it1).id == (*z).id) && (j != (id1-config->s_nid))){
						it2 = it1;
						flag2 = 1;	//let's et flag for cleaning up!
						break;
					}else flag2 = 0;

				}//end for
				//Remove the msg from all nodes buffer
					if(flag2 && j != (id1-config->s_nid)){
						//cout << "*1remove msg id: " <<  (*z).id << " from buffer of node: " << j+21 << endl;
						nodes[j].buffer_list.erase (it2);	//remove the message from node j's buffer since it has been delivered
						flag2 = 0;
					}

				}//end for
				//update the corresponding entry in traffic profile
				//let's update the traffic profile table for delivered videos
				for(k = traf_list[(*z).sid-config->s_nid].begin(); k != traf_list[(*z).sid-config->s_nid].end(); ++k){ 
					if((*k).sndr_id == (*z).sid && (*k).rcvr_id == (*z).rid && (*k).gen_t == (*z).gen_t && (*k).id == (*z).id){
						//timeout happened, let's save the statistics
						(*k).cost += cost;	
						if (DEBUG) log << "[3]updating profile for timed out message id: " << (*z).id << endl; // print member
						if (DEBUG) log << "1flag3: " << flag3 << endl;
					}
				}
				
			}

	}
	//let's remove the timeout message from id1's buffer
	if(flag3){
		if (DEBUG) log << "size before remove: " << nodes[id1-config->s_nid].buffer_list.size() << endl;
		if (DEBUG) log << "12remove message " << (*it3).id  << " from: " << id1 << endl;
		nodes[id1-config->s_nid].buffer_list.erase (it3);
		if (DEBUG) log << "size after remove: " << nodes[id1-config->s_nid].buffer_list.size() << endl;
		flag3 = 0;
	}
}

//direct transmission scheme
int routing::Waiting(int time){

 list<traffic>::iterator k;	// an iterator for the link list

//let's store all interaction of networks in log.dat file
ofstream log( "/home/kazem/Desktop/traces/logs/log.dat", ios::app );

// exit program if unable to create file

if ( !log ) // overloaded ! operator
{
	cerr << "File log.dat could not be opened" << endl;
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
		cout << "Communication using wait scheme for routing:" << endl;
		for (size_t i = 0; i < res.num_rows(); ++i) {
			//going through db and finding contact duration time for each contact and store them inside map table
			int stime = res[i][2];	//starting time
			int etime = res[i][3];	//end time
			int id1 = res[i][0];	//user1
			int id2 = res[i][1];	//user2
			int enumr = res[i][4];
			int current_time = stime;	//emulation clock is updated
			//let's check if user1 or user2 has video for the other one at a time <= stime, therefore we have to check the buffers of two users
			//list<message>::iterator z, it1, it2;	// an iterator for the buffer link list

			list<message>::iterator y, z, it1, it2, it3,it4;	// an iterator for the buffer link list

			timeout(id1, id2, current_time, log);	//let's go through id1 and id2 buffers and remove all timeout messages

			//let's print the buffer contents
			int flag = 0;	// to see if we should erase anything from the buffer of id1  
			for(z = nodes[id1-config->s_nid].buffer_list.begin(); z != nodes[id1-config->s_nid].buffer_list.end(); ++z) 
			//id1 has a video for id2
			if ((*z).sid == id1 && (*z).rid == id2 && (*z).gen_t <= stime){
				if (DEBUG) log << "contact between: (" << id1 << "," << id2 << ")" << ", starting at: " << stime << ", ending at: " << etime << endl;
				if (DEBUG) log << "message " << (*z).id << " generated at " << (*z).gen_t << " delivered at " << stime << " by " << id1 << endl;
				double delay = (double)(stime - (*z).gen_t)/(double)3600.0;
				if (DEBUG) log << "delay of delivery: " << delay << endl;
				
				it1 = z;	//let's save the iterator to remove this buffer from id1's buffer
				flag = 1;	//set the flag

				//let's update the traffic profile table for delivered videos
				for(k = traf_list[id1-config->s_nid].begin(); k != traf_list[id1-config->s_nid].end(); ++k){ 
					if((*k).sndr_id == id1 && (*k).rcvr_id == id2 && (*k).gen_t == (*z).gen_t){
						(*k).success = 1;	//successful delivery
						(*k).hops = 1;		//one hop transmission			
						(*k).cost = 1;		//one transmission for WAIT scheme
						(*k).delay = stime - (*z).gen_t;	//delay in sec
						(*k).rcv_t = stime;	//received time in sec
						if (DEBUG) log << "updating profile for node: " << (*k).sndr_id << " to node: " << (*k).rcvr_id << " in traffic profile table!" << endl; // print member
					}
				}
			}
			if(flag){
				nodes[id1-config->s_nid].buffer_list.erase (it1);	//remove the message from id1's buffer since it has been delivered
				flag = 0;
			}
			
			//id2 has a video for id1
			for(z = nodes[id2-config->s_nid].buffer_list.begin(); z != nodes[id2-config->s_nid].buffer_list.end(); ++z) 
			if ((*z).sid == id2 && (*z).rid == id1 && (*z).gen_t <= stime){
				if (DEBUG) log << "contact between: (" << id1 << "," << id2 << ")" << ", starting at: " << stime << ", ending at: " << etime << endl;
				if (DEBUG) log << "message " << (*z).id << " generated at " << (*z).gen_t << " delivered at " << stime << " by " << id2 << endl;
				double delay = (double)(stime - (*z).gen_t)/(double)3600.0;
				if (DEBUG) log << "delay: " << delay << " hours" << endl;
				it2 = z;	//let's save the iterator to remove this buffer from id1's buffer
				flag = 1;	//set the flag

				//let's update the traffic profile table for delivered videos
				for(k = traf_list[id2-config->s_nid].begin(); k != traf_list[id2-config->s_nid].end(); ++k){ 
					if((*k).sndr_id == id2 && (*k).rcvr_id == id1 && (*k).gen_t == (*z).gen_t){
						(*k).success = 1;	//successful delivery
						(*k).hops = 1;		//one hop transmission			
						(*k).cost = 1;		//one transmission for WAIT scheme
						(*k).delay = stime - (*z).gen_t;	//delay in sec
						(*k).rcv_t = stime;	//received time in sec
						if (DEBUG) log << "updating profile for node: " << (*k).sndr_id << " to node: " << (*k).rcvr_id << " in traffic profile table!" << endl; // print member
					}
				}
			}
			if(flag){
				nodes[id2-config->s_nid].buffer_list.erase (it2);	//remove the message from id2's buffer since it has been delivered
			}

		}
		//let's virtually clean up all the timeout messages from nodes buffers
		int i;
		int current_time = ts + 2*TTL + 1; //update for solving the bug of msgs with uniform gen times
		for (i = config->s_nid; i <= config->e_nid; i++){
			//let's go through id1's buffer to find expired message, we should update the traffic profile and remove those message from the buffer
			timeout(i, current_time, log);
		}

		//let's store delay in delayW.dat file: Waiting Scheme
		ofstream delayW( "/home/kazem/Desktop/traces/logs/delayW.dat", ios::app );
			
		// exit program if unable to create file
			
		if ( !delayW ) // overloaded ! operator
		{
		cerr << "File delayW.dat could not be opened" << endl;
		return -1;
		} // end if
	
		//let's store delay in costW.dat file: Waiting Scheme
		ofstream costW( "/home/kazem/Desktop/traces/logs/costW.dat", ios::app );
			
		// exit program if unable to create file
			
		if ( !costW ) // overloaded ! operator
		{
			cerr << "File costW.dat could not be opened" << endl;
			return -1;
		} // end if
		//write delay and cost, delivery ratio and cost	
		write_profile(delayW, costW, time, 0);
		delayW.close();
		costW.close();
	        log << "-----------------------------------------------------------------------" << endl;
		log.close();
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


int routing::Epidemic(int time){

	int paths[79];	//let's store paths in this array: the no of times node i appears in a short delay path
	for (int i = 0; i< 79; i++) paths[i] = 0;	//initialize the paths array

	list<traffic>::iterator k;	// an iterator for the link list
	
	message msg; 
	//let's store all interaction of networks in log.dat file
	ofstream log( "/home/kazem/Desktop/traces/logs/log.dat", ios::app );
	
	// exit program if unable to create file
	
	if ( !log ) // overloaded ! operator
	{
		cerr << "File log.dat could not be opened" << endl;
		return -1;
	}

	//let's store all paths in path.dat file
	ofstream path( "/home/kazem/Desktop/traces/logs/path.dat", ios::app );
	
	// exit program if unable to create file
	
	if ( !path ) // overloaded ! operator
	{
		cerr << "File path.dat could not be opened" << endl;
		return -1;
	}

// Connect to database 
	mysqlpp::Connection conn(false);

	/* Connect to database */
	if (conn.connect(config->db, "localhost", "root", "mysql")) {
		// Retrieve a subset of the sample stock table set up by resetdb
		// and display it
		cout << "Connected to db!" << endl;
		
		char _query [500];
		bzero(_query, 500);
		//we should count all contacts: not only long ones!!!
		sprintf (_query, "select user1, user2, starttime, endtime from %s where user1 <= %d and user1 >= %d and user2 <= %d and user2 >= %d and starttime > %d and starttime < %d order by starttime", config->table, config->e_nid, config->s_nid, config->e_nid, config->s_nid, ts, ts + TTL);
		mysqlpp::Query query = conn.query(_query);

		if (mysqlpp::StoreQueryResult res = query.store()){
		
//flooding scheme
		cout << "Communication using flooding scheme for routing:" << endl;
		for (size_t i = 0; i < res.num_rows(); ++i) {
			//going through db and finding contact duration time for each contact and store them inside map table
			int stime = res[i][2];	//starting time
			int etime = res[i][3];	//end time
			int id1 = res[i][0];	//user1
			int id2 = res[i][1];	//user2
			int current_time = stime;	//emulation clock is updated

			list<message>::iterator y, z, it1, it2, it3,it4;	// an iterator for the buffer link list

			timeout(id1, id2, current_time, log);	//let's go through id1 and id2 buffers and remove all timeout messages

			//now let's compare T1 and T2 to see what should we flood
			//read id1's and id2's buffers to see which messages should be copied to the ther one's buffer
			int flag1 = 0;	// to see if we should erase anything from the buffer of id1  
			//let's go through id1.buffer to see if there is any msg to be copied from ID1--> ID2
			//We are looking for T12 = T1-T2
			for(z = nodes[id1-config->s_nid].buffer_list.begin(); z != nodes[id1-config->s_nid].buffer_list.end(); ++z){
				flag1 = 0;
				for(y = nodes[id2-config->s_nid].buffer_list.begin(); y != nodes[id2-config->s_nid].buffer_list.end(); ++y){		
					if ((*z).id == (*y).id){
						flag1 = 1;	//this message has already been found in id2's buffer
						break;		//we dont need to copy this message
					}
				}					
				if(flag1 == 0 && (*z).gen_t <= current_time){

//if((*z).id == 11)	cout << "gen: " << (*z).gen_t << ", ctime: " << current_time << endl;
					//number of flooding is bounded by cg array
					//cout << "mesage id: " << (*z).id << " does not exist in " << id2 << "'s buffer" << endl;
					//T12 != null
					//we should copy the message from id1's buffer to id2's buffer
					if((current_time - (*z).gen_t <= TTL) && ((*z).delivered == false)){//check if msg has been expired
						//message has not been expired
						if(id1 != (*z).rid){//check if ID1 is the receiver of the message, if so then we dont need o copy the msg from ID1 to ID2!
						//core of routing scheme decision: Flooding
							if (DEBUG) log << "message id " << (*z).id << " is copied from " << id1 << " to " << id2 << " buffer at " << current_time << endl;
							msg.id = (*z).id;
							msg.sid = (*z).sid;;
							msg.rid = (*z).rid;
							msg.gen_t = (*z).gen_t;
							msg.no_hops = (*z).no_hops + 1;

							//ID1 --> ID2: relay=ID2
//let's read the path of (*z) and copy to a msg for the new relay node
//if((*z).id == 20 || (*z).id == 70){
//   cout << "id2: " << id2 << " " << endl;
   list<int>::iterator it_r;

   for ( it_r = (*z).relay_list.begin() ; it_r != (*z).relay_list.end(); it_r++ )
     msg.relay_list.push_back(*it_r);
     msg.relay_list.push_back(id2);
//log the time profile of relay time
   for ( it_r = (*z).time_list.begin() ; it_r != (*z).time_list.end(); it_r++ )
     msg.time_list.push_back(*it_r);
     msg.time_list.push_back(current_time);

//  for ( it_r = msg.relay_list.begin() ; it_r != msg.relay_list.end(); it_r++ )
//    cout << *it_r << " ";
//  cout << endl;
//}

							if(id2 == (*z).rid)	msg.delivered = true;//this is the destination of msg
							else msg.delivered = false;
							nodes[id2-config->s_nid].buffer_list.push_back(msg);	// copy the message to the end of the id2's buffer!



							(*z).cost++;	//since we have transmitted this message from id1 to id2
							//let's check if ID2 is the msg.rcvr
							if(id2 == (*z).rid && (*z).gen_t <= stime){
								(*z).delivered = true;	//stop copying this message anymore
								//Go to the corresponding entry of traffic profile and updates the attributes
								//id1 has a video for id2
								if (DEBUG) log << "node " << id2 << " is receiver" << endl;
								if (DEBUG) log << "message " << (*z).id << " generated at " << (*z).gen_t << " delivered at " << stime << " by " << id1 << endl;
								if (DEBUG && (closeness[id1-config->s_nid][id2-config->s_nid] > 0)) log << "relay sim " << closeness[id1-config->s_nid][id2-config->s_nid] << endl;
								double delay = (double)(stime - (*z).gen_t)/(double)3600.0;
								if (DEBUG) log << "delay of delivery: " << delay << endl;
								it1 = z;	//let's save the iterator to remove this buffer from id1's buffer???
//we have reached the destination: the full path report
//if((*z).id == 20 || (*z).id == 70){  
// cout << "src: " << (*z).sid << " " << "rcv: " << (*z).rid << endl;
  list<int>::iterator it_r;
int no_relays = 0;
  path << "message " << (*z).id << " generated at " << (*z).gen_t << " delivered at " << stime << " by " << id1 << endl;
  path << "path:";
  for ( it_r = msg.relay_list.begin() ; it_r != msg.relay_list.end(); it_r++ ){
    paths[*it_r-config->s_nid]++;	//let's count the relay node in its corresponding place
    path << " " << *it_r;
    no_relays++;
  }
path << endl << "time: ";
  for ( it_r = msg.time_list.begin() ; it_r != msg.time_list.end(); it_r++ ){
    path << " " << *it_r;
  }

path << endl << "relay no: " << no_relays << ", delay: " << stime - *msg.time_list.begin();
path << endl << "D*Nhop: " << no_relays*(stime - *msg.time_list.begin());
path << endl << "--------------------------" << endl;
//}
								//let's update the traffic profile table for delivered videos
								for(k = traf_list[(*z).sid-config->s_nid].begin(); k != traf_list[(*z).sid-config->s_nid].end(); ++k){ 
									if((*k).sndr_id == (*z).sid && (*k).rcvr_id == id2 && (*k).gen_t == (*z).gen_t){
										//successful delivery
										(*k).success = 1;	
										(*k).hops = (*z).no_hops + 1;	//one hop transmission	
										(*k).delay = stime - (*z).gen_t;//delay in sec
										(*k).rcv_t = stime;	//received time in sec
									}
										
									if (DEBUG) log << "1updating profile for node: " << (*k).sndr_id << " to node: " << (*k).rcvr_id << " in traffic profile table!" << endl; // print member
									//cout << endl;
								}
							}
						msg.relay_list.clear();	//remove the path
						msg.time_list.clear();
						}//else since ID1 is the msg.rcvr --> NOP
					}
				}	
			}//end for


			//read id2's and id1's buffers to see which messages should be copied to the ther one's buffer
			flag1 = 0;	// to see if we should erase anything from the buffer of id2  
			//let's go through id2.buffer to see if there is any msg to be copied from ID2--> ID1
			//We are looking for T21 = T2-T1
			for(z = nodes[id2-config->s_nid].buffer_list.begin(); z != nodes[id2-config->s_nid].buffer_list.end(); ++z){
				flag1 = 0;
				for(y = nodes[id1-config->s_nid].buffer_list.begin(); y != nodes[id1-config->s_nid].buffer_list.end(); ++y){		
					if ((*z).id == (*y).id){
						flag1 = 1;	//this message has already been copied to id2's buffer
						break;		//stop processing more
					}
				}

				if(flag1 == 0 && (*z).gen_t <= current_time){

//if((*z).id == 11)	cout << "gen: " << (*z).gen_t << ", ctime: " << current_time << endl;

					//cout << "mesage id: " << (*z).id << " does not exist in " << id1 << "'s buffer" << endl;
					//T12 != null
					//we should copy the message from id1's buffer to id2's buffer
					if((current_time - (*z).gen_t <= TTL) && ((*z).delivered == false)){//check if msg has been expired
						//message has not been expired
						if(id2 != (*z).rid){//check if ID2 is the receiver of the message, if so then we dont need o copy the msg from ID1 to ID2!
						//core of routing scheme decision: Flooding
							
							if (DEBUG) log << "mesage id " << (*z).id << " is copied from " << id2 << " to " << id1 << " 's buffer at " << current_time << endl;

							msg.id = (*z).id;
							msg.sid = (*z).sid;;
							msg.rid = (*z).rid;
							msg.gen_t = (*z).gen_t;
							msg.no_hops = (*z).no_hops + 1;
							//ID2 --> ID1: relay = ID1
//let's read the path of (*z) and copy to a msg for the new relay node							
//if((*z).id == 20 || (*z).id == 70){
//  cout << "id1: " << id1 << " " << endl;
  list<int>::iterator it_r;

  for ( it_r = (*z).relay_list.begin() ; it_r != (*z).relay_list.end(); it_r++ )
    msg.relay_list.push_back(*it_r);
    msg.relay_list.push_back(id1);

  for ( it_r = (*z).time_list.begin() ; it_r != (*z).time_list.end(); it_r++ )
    msg.time_list.push_back(*it_r);
    msg.time_list.push_back(current_time);

// for ( it_r = msg.relay_list.begin() ; it_r != msg.relay_list.end(); it_r++ )
//    cout << *it_r << " ";
// cout << endl;
//}
//if((*z).id == 0)	cout << id1 << " ";

							if(id1 == (*z).rid)	msg.delivered = true;//this is the destination of msg
							else msg.delivered = false;
							nodes[id1-config->s_nid].buffer_list.push_back(msg);	// copy the message to the end of the id2's buffer!


							(*z).cost++;	//since we have transmitted this message from id1 to id2
							//let's check if ID2 is the msg.rcvr
							if(id1 == (*z).rid && (*z).gen_t <= stime){
								(*z).delivered = true;	//stop copying anymore!
								//Go to the corresponding entry of traffic profile and updates the attributes
								//id2 has a video for id1
								if (DEBUG) log << "node " << id1 << " is receiver" << endl;
								if (DEBUG) log << "message " << (*z).id << " generated at " << (*z).gen_t << " delivered at " << stime << " by " << id2 << endl;
								if (DEBUG && (closeness[id2-config->s_nid][id1-config->s_nid] > 0)) log << "relay sim " <<  closeness[id2-config->s_nid][id1-config->s_nid] << endl;
								double delay = (double)(stime - (*z).gen_t)/(double)3600.0;
								
								if (DEBUG) log << "delivery delay: " << delay << endl;
								it1 = z;	//let's save the iterator to remove this buffer from id1's buffer
//we have reached the destination: the full path report
//if((*z).id == 20 || (*z).id == 70){
// cout << "src: " << (*z).sid << " " << "rcv: " << (*z).rid << endl;
  list<int>::iterator it_r;
int no_relays = 0;
path << "message " << (*z).id << " generated at " << (*z).gen_t << " delivered at " << stime << " by " << id2 << endl;
  path << "path:";
  for ( it_r = msg.relay_list.begin() ; it_r != msg.relay_list.end(); it_r++ ){
    //we are incrementing the no every node appears in optimal path
    paths[*it_r-config->s_nid]++;	//let's count the relay node in its corresponding place
    path << " " << *it_r;
    no_relays++;
  }

path << endl << "time: ";
  for ( it_r = msg.time_list.begin() ; it_r != msg.time_list.end(); it_r++ ){
    path << " " << *it_r;
  }

path << endl << "relay no: " << no_relays << ", delay: " << stime - *msg.time_list.begin();
path << endl << "D*Nhop: " << no_relays*(stime - *msg.time_list.begin());
path << endl << "--------------------------" << endl;
//}
								//let's update the traffic profile table for delivered videos
								for(k = traf_list[(*z).sid-config->s_nid].begin(); k != traf_list[(*z).sid-config->s_nid].end(); ++k){ 
									if((*k).sndr_id == (*z).sid && (*k).rcvr_id == id1 && (*k).gen_t == (*z).gen_t){
										//successful delivery
										(*k).success = 1;	
										(*k).hops = (*z).no_hops + 1;	//one hop transmission	
										(*k).delay = stime - (*z).gen_t;//delay in sec
										(*k).rcv_t = stime;	//received time in sec
									}
										
									if (DEBUG) log << "2updating profile for node: " << (*k).sndr_id << " to node: " << (*k).rcvr_id << " in traffic profile table!" << endl; // print member
									//cout << endl;
								}
							}
						msg.relay_list.clear();	//remove the path
						msg.time_list.clear();
						}//else since ID1 is the msg.rcvr --> NOP
					}
				}	
			}//end for
		}//end for

		//let's virtually clean up all the timeout messages from nodes buffers
		int i;
		int current_time = ts + 2*TTL + 1; //update for solving the bug of msgs with uniform gen times
		for (i = config->s_nid; i <= config->e_nid; i++){
			//let's go through id1's buffer to find expired message, we should update the traffic profile and remove those message from the buffer
			timeout(i, current_time, log);
		}

		//let's store delay in delayF.dat file: Waiting Scheme
		ofstream delayF( "/home/kazem/Desktop/traces/logs/delayF.dat", ios::app );
			
		// exit program if unable to create file
		ofstream costF( "/home/kazem/Desktop/traces/logs/costF.dat", ios::app );
		
		// exit program if unable to create file
		
		if ( !delayF || !costF ) // overloaded ! operator
		{
		cerr << "File delayF.dat or costF could not be opened" << endl;
		return -1;
		} // end if
		//write into cost and delay files			
		write_profile(delayF, costF, time, 1);
		costF.close();
		delayF.close();
	        log << "-----------------------------------------------------------------------" << endl;
		log.close();
		path.close();
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

cout << "let's check the paths: " << endl;
for (int i=0; i < 79; i++)
    cout << i << " ";

cout << endl;

for (int i=0; i < 79; i++)
    cout << paths[i] << " ";

//   if(paths[i] != 0) cout << " (" << i + config->s_nid << "," << paths[i] << ") ";

	return 0;
}
//this methods find the social distance table
//only take into account social dimensions
int routing::find_social_dist(unsigned char social){
	map<int,int> addr_nodes;	//find addressable nodes
	distr dist_(config);
	int no_cont = 0;
	int cont_dur = 0;

	cout << "starting pre-processing!" << endl;

	ofstream distF( "/home/kazem/Desktop/traces/logs/jdist.dat", ios::app );
		
	// exit program if unable to create file
		
	if ( !distF ) // overloaded ! operator
	{
		cerr << "File dist.dat could not be opened" << endl;
		return -1;
	} // end if

	for (int i = config->s_nid; i <= config->e_nid; i++)
		for (int j = i; j <= config->e_nid; j++){
			char u1[ 10 ];	//user1
			char u2[ 10 ];	//user2
			bzero ( u1, 10 );
			bzero ( u2, 10 );
			sprintf ( u1,"%d", i );
			sprintf ( u2,"%d", j );
//			cout << "closeness of (" << u1 << "," << u2 << ") = " << dist_.closeness ( u1,u2 ) << endl;
//			closeness[i-config->s_nid][j-config->s_nid] = max(dist_.closeness(u1, u2, social), 1.0/79.0);
			closeness[i-config->s_nid][j-config->s_nid] = max(dist_.sim_watts(u1, u2, social), 1.0/79.0);

			closeness[j-config->s_nid][i-config->s_nid] = closeness[i-config->s_nid][j-config->s_nid];	//symmetric matrix
		
			//if (closeness[i-config->s_nid][j-config->s_nid] >= 0.5 && i != j){
			if (i != j){
			no_cont = dist_.no_of_contacts(u1, u2, 57600, 57600 + 57*3600, 1);	//--> aggregate contacts
			cont_dur = dist_.contact_duration(u1, u2, 57600, 57600 + 57*3600);
			//distF << "(" << i << "," << j << ") = " << closeness[i-config->s_nid][j-config->s_nid] << ", no: " << no_cont << ", dur: " << cont_dur << endl; 
			//to parse the weighted graph
			distF << i << "," << j << "," << 1.0/closeness[i-config->s_nid][j-config->s_nid] << endl; 
			addr_nodes.insert ( pair<int,int>(i,1) );
			addr_nodes.insert ( pair<int,int>(j,1) );
			}
		}
	distF.close();
	cout << "endinig pre-processing!" << endl;
	//let's print the list of addressable nodes: those nodes which have socially close nodes based on distance
	map<int,int>::iterator it;
	for ( it=addr_nodes.begin() ; it != addr_nodes.end(); it++ )
    		cout << (*it).first << ", ";
	std::cout << endl << addr_nodes.size() << endl;
	return 0;
}

//this methods find the new social distance table
//only take into account social dimensions
//wflag: 1 for write to file and 0 otherwise
int routing::find_focus_dist(unsigned char social, int wflag){
	map<int,int> addr_nodes;	//find addressable nodes
	distr dist_(config);
	int no_cont = 0;
	int cont_dur = 0;

	ofstream distF( "/home/kazem/Desktop/traces/logs/fdist.dat", ios::app );
		
	// exit program if unable to create file
		
	if ( !distF ) // overloaded ! operator
	{
		cerr << "File dist.dat could not be opened" << endl;
		return -1;
	} // end if

	cout << "starting pre-processing!" << endl;

	for (int i = config->s_nid; i <= config->e_nid; i++)
		for (int j = i; j <= config->e_nid; j++){
			char u1[ 10 ];	//user1
			char u2[ 10 ];	//user2
			bzero ( u1, 10 );
			bzero ( u2, 10 );
			sprintf ( u1,"%d", i );
			sprintf ( u2,"%d", j );
//			cout << "closeness of (" << u1 << "," << u2 << ") = " << dist_.closeness ( u1,u2 ) << endl;
			double ndistance = dist_.socialdist ( u1,u2, social );	//this is the actual distance
			
			double prob_f = max(1.0/ndistance, 1.0/79.0);	//rank based friendship based on Nowell and Kleinberg model, the main question is that 1/dist^q 
			closeness[i-config->s_nid][j-config->s_nid] = prob_f;//dist_.socialdist(u1, u2);
			closeness[j-config->s_nid][i-config->s_nid] = closeness[i-config->s_nid][j-config->s_nid];	//symmetric matrix

			if (wflag && prob_f >= 0.25 && i != j){		
				no_cont = dist_.no_of_contacts(u1, u2, 57600, 57600 + 33*3600, 1);	//--> aggregate contacts
				cont_dur = dist_.contact_duration(u1, u2, 57600, 57600 + 33*3600);
				distF << "(" << i << "," << j << ") = " << closeness[i-config->s_nid][j-config->s_nid] << ", no: " << no_cont << ", dur: " << cont_dur << endl; 
				addr_nodes.insert ( pair<int,int>(i,1) );
				addr_nodes.insert ( pair<int,int>(j,1) );
			}
		}
	cout << "endinig pre-processing!" << endl;
	distF.close();

	//let's print the list of addressable nodes: those nodes which have socially close nodes based on distance
	map<int,int>::iterator it;
	for ( it=addr_nodes.begin() ; it != addr_nodes.end(); it++ )
    		cout << (*it).first << ", ";
	std::cout << endl << addr_nodes.size() << endl;

	return 0;
}

//this methods find the social distance table
//only take into account social dimensions
//r: homophily exponent
int routing::find_cont_prob(unsigned char social, double r){
	map<int,int> addr_nodes;	//find addressable nodes
	cout << "starting pre-processing!" << endl;

	ofstream contPr( "/home/kazem/Desktop/traces/logs/contP.dat", ios::app );
		
	// exit program if unable to create file
		
	if ( !contPr ) // overloaded ! operator
	{
		cerr << "File contP.dat could not be opened" << endl;
		return -1;
	} // end if

	distr dist_(config);
	for (int i = config->s_nid; i <= config->e_nid; i++)
		for (int j = i; j <= config->e_nid; j++){
			char u1[ 10 ];	//user1
			char u2[ 10 ];	//user2
			bzero ( u1, 10 );
			bzero ( u2, 10 );
			sprintf ( u1,"%d", i );
			sprintf ( u2,"%d", j );
//			cout << "closeness of (" << u1 << "," << u2 << ") = " << dist_.closeness ( u1,u2 ) << endl;
			double ndistance = dist_.socialdist ( u1,u2, social );
			double prob_f = 1.0/ndistance;
//			double close_ = pow(max(prob_f, 1.0/80.0), r);
			double close_ = pow(max(dist_.closeness(u1, u2, social), 1.0/80.0), r);
//			std::cout << dist_.closeness(u1, u2, social) << " " << 1.0/80.0 << " max: " << max(dist_.closeness(u1, u2, social), 1.0/80.0) << endl;
			closeness[i-config->s_nid][j-config->s_nid] = close_;
			closeness[j-config->s_nid][i-config->s_nid] = close_;	//symmetric matrix
		}
	
	cout << "endinig pre-processing!" << endl;
	//let's calculate the contact probs

	for (int i = config->s_nid; i <= config->e_nid; i++){
		double norm_ = 0;	//normalized factor
		for (int j = config->s_nid; j <= config->e_nid; j++)
			if(i != j)	norm_ += closeness[i-config->s_nid][j-config->s_nid];	

		for (int j = config->s_nid; j <= config->e_nid; j++)
			if(i != j){
				contP[i-config->s_nid][j-config->s_nid] = closeness[i-config->s_nid][j-config->s_nid]/norm_;
				contPr << "(" << i << ", " << j << ")=" << contP[i-config->s_nid][j-config->s_nid] << endl; 
			}	
	}
	contPr.close();
	return 0;
}
//Greedy forewarding method
//thresh: the threshold parameter for routing
int routing::GreedyA(int time, double thresh){
	list<traffic>::iterator k;	// an iterator for the link list
	
	message msg; 
	//let's store all interaction of networks in log.dat file
	ofstream log( "/home/kazem/Desktop/traces/logs/log.dat", ios::app );
	
	// exit program if unable to create file
	
	if ( !log ) // overloaded ! operator
	{
		cerr << "File log.dat could not be opened" << endl;
		return -1;
	}

	// Connect to database 
	mysqlpp::Connection conn(false);

	/* Connect to database */
	if (conn.connect(config->db, "localhost", "root", "mysql")) {
		// Retrieve a subset of the sample stock table set up by resetdb
		// and display it
		cout << "Connected to db!" << endl;
		
		char _query [500];
		bzero(_query, 500);
		//we should count all contacts: not only long ones!!!
		sprintf (_query, "select user1, user2, starttime, endtime from %s where user1 <= %d and user1 >= %d and user2 <= %d and user2 >= %d and starttime > %d and starttime < %d order by starttime", config->table, config->e_nid, config->s_nid, config->e_nid, config->s_nid, ts, ts + TTL);
		mysqlpp::Query query = conn.query(_query);

		if (mysqlpp::StoreQueryResult res = query.store()){
			//greedy scheme
			cout << "Communication using Social-Greedy I scheme for routing:" << endl;
//			double thresh = 0.1;	//threshod for routing
			double thresh_b = thresh;	//backup threshold
	
			for (size_t i = 0; i < res.num_rows(); ++i) {
				//going through db and finding contact duration time for each contact and store them inside map table
				int stime = res[i][2];	//starting time
				int etime = res[i][3];	//end time
				int id1 = res[i][0];	//user1
				int id2 = res[i][1];	//user2
				int current_time = stime;	//emulation clock is updated
				//let's check if user1 or user2 has video for the other one at a time <= stime, therefore we have to check the buffers of two users
				list<message>::iterator y, z, it1, it2, it3,it4;	// an iterator for the buffer link list
				
				//let's go through id1's buffer to find expired message, we should update the traffic profile and remove those message from the buffer
				timeout(id1, id2, current_time, log);	//let's go through id1 and id2 buffers and remove all timeout messages
	
				
				//now let's compare T1 and T2 to see what should we flood
				//read id1's and id2's buffers to see which messages should be copied to the ther one's buffer
				int flag1 = 0;	// to see if we should erase anything from the buffer of id1  
				//let's go through id1.buffer to see if there is any msg to be copied from ID1--> ID2
				//We are looking for T12 = T1-T2
				for(z = nodes[id1-config->s_nid].buffer_list.begin(); z != nodes[id1-config->s_nid].buffer_list.end(); ++z){
					flag1 = 0;
	
					for(y = nodes[id2-config->s_nid].buffer_list.begin(); y != nodes[id2-config->s_nid].buffer_list.end(); ++y){		
						if ((*z).id == (*y).id){
							flag1 = 1;	//this message has already been found in id2's buffer
							break;
						}
					}
					if(flag1 == 0 && (*z).gen_t <= current_time){
						//cout << "mesage id: " << (*z).id << " does not exist in " << id2 << "'s buffer" << endl;
						//T12 != null
						//we should copy the message from id1's buffer to id2's buffer
						if(current_time - (*z).gen_t <= TTL){//check if msg has been expired
							//message has not been expired
							if(id1 != (*z).rid){//check if ID1 is the receiver of the message, if so then we dont need to copy the msg from ID1 to ID2!
							//core of routing scheme decision: Flooding

							if(id2 == (*z).rid){	
								thresh = 0.0;
							}
							else {
								thresh = thresh_b;
							}	
							//threshold
							if((closeness[id1-config->s_nid][(*z).rid-config->s_nid] < closeness[id2-config->s_nid][(*z).rid-config->s_nid] - thresh) && ((*z).delivered == false)){

//							if((closeness[id1-config->s_nid][(*z).rid-config->s_nid] > closeness[id2-config->s_nid][(*z).rid-config->s_nid]) && ((*z).delivered == false)){
	
									if (DEBUG) log << "1message id: " << (*z).id << " is copied from: " << id1 << " to: " << id2 << "'s buffer at: " << current_time << ", closeness (sender, dst): " << closeness[id1-config->s_nid][(*z).rid-config->s_nid] << ", closeness (receiver, dst): " << closeness[id2-config->s_nid][(*z).rid-config->s_nid]  << endl << endl;
								
								
									//greedy: copy of the relay is closer to desination
									msg.id = (*z).id;
									msg.sid = (*z).sid;;
									msg.rid = (*z).rid;
									msg.gen_t = (*z).gen_t;
									msg.no_hops = (*z).no_hops + 1;
									if(id2 == (*z).rid)	msg.delivered = true;//this is the destination of msg
									else msg.delivered = false;
									nodes[id2-config->s_nid].buffer_list.push_back(msg);	// copy the message to the end of the id2's buffer!
									if(id2 != (*z).rid)	(*z).cost++;	//since we have transmitted this message from id1 to id2
								}
								//let's check if ID2 is the msg.rcvr
								if(id2 == (*z).rid && (*z).gen_t <= stime){
									(*z).cost++;	//we have to count the last step delivery
									(*z).delivered = true;	//stop sending this data anymore
									//Go to the corresponding entry of traffic profile and updates the attributes
									//id1 has a video for id2
									if (DEBUG) log << "node " << id2 << " is the receiver" << endl;
									if (DEBUG) log << "message " << (*z).id << " generated at " << (*z).gen_t << " delivered at " << stime << " by " << id1 << endl;
									
									double delay = (double)(stime - (*z).gen_t)/(double)3600.0;
									if (DEBUG) log << "delay of delivery: " << delay << endl;
									it1 = z;	//let's save the iterator to remove this buffer from id1's buffer???
	
									//let's update the traffic profile table for delivered videos
									for(k = traf_list[(*z).sid-config->s_nid].begin(); k != traf_list[(*z).sid-config->s_nid].end(); ++k){ 
										if((*k).sndr_id == (*z).sid && (*k).rcvr_id == id2 && (*k).gen_t == (*z).gen_t && (*k).id == (*z).id){
											//successful delivery
											(*k).success = 1;	
											(*k).hops = (*z).no_hops + 1;	//one hop transmission	
											(*k).delay = stime - (*z).gen_t;//delay in sec
											(*k).rcv_t = stime;	//received time in sec
										}
											
										if (DEBUG) log << "1updating profile for node: " << (*k).sndr_id << " to node: " << (*k).rcvr_id << " in traffic profile table!" << endl; // print member
										//cout << endl;
									}
								}
							}//else since ID1 is the msg.rcvr --> NOP
						}
					}	
				}
	
	
				//read id2's and id1's buffers to see which messages should be copied to the ther one's buffer
				flag1 = 0;	// to see if we should erase anything from the buffer of id2  
				//let's go through id2.buffer to see if there is any msg to be copied from ID2--> ID1
				//We are looking for T21 = T2-T1
				for(z = nodes[id2-config->s_nid].buffer_list.begin(); z != nodes[id2-config->s_nid].buffer_list.end(); ++z){
					flag1 = 0;
					for(y = nodes[id1-config->s_nid].buffer_list.begin(); y != nodes[id1-config->s_nid].buffer_list.end(); ++y){		
						if ((*z).id == (*y).id){
							flag1 = 1;	//this message has already been copied to id2's buffer
							break;		//we should break since this message (*z).id is in id2's buffer
						}
					}
					if(flag1 == 0 && (*z).gen_t <= current_time){
						//cout << "mesage id: " << (*z).id << " does not exist in " << id1 << "'s buffer" << endl;
						//T12 != null
						//we should copy the message from id1's buffer to id2's buffer
						if(current_time - (*z).gen_t <= TTL){//check if msg has been expired
							//message has not been expired
							if(id2 != (*z).rid){//check if ID2 is the receiver of the message, if so then we dont need o copy the msg from ID1 to ID2!
							//core of routing scheme decision: Flooding
							if(id1 == (*z).rid){	
								thresh = 0.0;
							}
							else {
								thresh = thresh_b;
							}	
							//threshold
							if((closeness[id2-config->s_nid][(*z).rid-config->s_nid] < closeness[id1-config->s_nid][(*z).rid-config->s_nid] - thresh) && ((*z).delivered == false)){

//							if((closeness[id2-config->s_nid][(*z).rid-config->s_nid] > closeness[id1-config->s_nid][(*z).rid-config->s_nid]) && ((*z).delivered == false)){
	
									if (DEBUG) log << "2message id: " << (*z).id << " is copied from: " << id2 << " to: " << id1 << "'s buffer at: " << current_time << ", closeness (sender, dst): " << closeness[id2-config->s_nid][(*z).rid-config->s_nid] << ", closeness (receiver, dst): " << closeness[id1-config->s_nid][(*z).rid-config->s_nid]  << endl << endl;
								
									//if relay is closer to destination, copy the file
									msg.id = (*z).id;
									msg.sid = (*z).sid;;
									msg.rid = (*z).rid;
									msg.gen_t = (*z).gen_t;
									msg.no_hops = (*z).no_hops + 1;
									if(id1 == (*z).rid)	msg.delivered = true;//this is the destination of msg
									else msg.delivered = false;
									nodes[id1-config->s_nid].buffer_list.push_back(msg);	// copy the message to the end of the id2's buffer!
									if(id1 != (*z).rid)	(*z).cost++;	//since we have transmitted this message from id1 to id2
								}
								//let's check if ID1 is the msg.rcvr
								if(id1 == (*z).rid && (*z).gen_t <= stime){
									(*z).cost++;	//we have to count the last step delivery
									(*z).delivered = true;
									//Go to the corresponding entry of traffic profile and updates the attributes
									//id1 has a video for id2
									if (DEBUG) log << "node " << id1 << " is the receiver" << endl;
									if (DEBUG) log << "message " << (*z).id << " generated at " << (*z).gen_t << " delivered at " << stime << " by " << id2 << endl;
									double delay = (double)(stime - (*z).gen_t)/(double)3600.0;
									
									if (DEBUG) log << "delivery delay: " << delay << endl;
									it1 = z;	//let's save the iterator to remove this buffer from id1's buffer
	
									//let's update the traffic profile table for delivered videos
									for(k = traf_list[(*z).sid-config->s_nid].begin(); k != traf_list[(*z).sid-config->s_nid].end(); ++k){ 
										if((*k).sndr_id == (*z).sid && (*k).rcvr_id == id1 && (*k).gen_t == (*z).gen_t && (*k).id == (*z).id){
											//successful delivery
											(*k).success = 1;	
											(*k).hops = (*z).no_hops + 1;	//one hop transmission	
											(*k).delay = stime - (*z).gen_t;//delay in sec
											(*k).rcv_t = stime;	//received time in sec
										}
											
										if (DEBUG) log << "2updating profile for node: " << (*k).sndr_id << " to node: " << (*k).rcvr_id << " in traffic profile table!" << endl; // print member
										//cout << endl;
									}
								}
							}//else since ID1 is the msg.rcvr --> NOP
						}
					}	
				}
			}//end for

			//let's virtually clean up all the timeout messages from nodes buffers
			int i;
			int current_time = ts + 2*TTL + 1; //update for solving the bug of msgs with uniform gen times
			for (i = config->s_nid; i <= config->e_nid; i++){
				//let's go through id1's buffer to find expired message, we should update the traffic profile and remove those message from the buffer
				timeout(i, current_time, log);
			}

			//let's store delay in delayG.dat file: Greedy I Scheme
			ofstream delayGA( "/home/kazem/Desktop/traces/logs/delayGA.dat", ios::app );
				
			// exit program if unable to create file
				
			ofstream costGA( "/home/kazem/Desktop/traces/logs/costGA.dat", ios::app );
			
			// exit program if unable to create file
		
			if ( !delayGA || !costGA ) // overloaded ! operator
			{
				cerr << "File delayGA.dat or costGA.dat could not be opened" << endl;
				return -1;
			} // end if
			//write delay and cost
			write_profile(delayGA, costGA, time, 2);
			delayGA.close();
			costGA.close();
		        log << "-----------------------------------------------------------------------" << endl;
			log.close();
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

//MCP forwarding method
int routing::MCP(int hopttl_val, int mcp_val, int time){

	hopttl = hopttl_val;	//the initial time-to-live hop count limit: for MCP forwarding scheme
	mcp = mcp_val;	//no of copies upper bound: for MCP forwarding scheme

	list<traffic>::iterator k;	// an iterator for the link list
	message msg; 
	//let's store all interaction of networks in log.dat file
	ofstream log( "/home/kazem/Desktop/traces/logs/log.dat", ios::app );
	
	// exit program if unable to create file
	if ( !log ) // overloaded ! operator
	{
		cerr << "File log.dat could not be opened" << endl;
		return -1;
	}

	// Connect to database 
	mysqlpp::Connection conn(false);

	/* Connect to database */
	if (conn.connect(config->db, "localhost", "root", "mysql")) {
		// Retrieve a subset of the sample stock table set up by resetdb
		// and display it
		cout << "Connected to db!" << endl;
		
		char _query [500];
		bzero(_query, 500);
		//we should count all contacts: not only long ones!!!
		sprintf (_query, "select user1, user2, starttime, endtime from %s where user1 <= %d and user1 >= %d and user2 <= %d and user2 >= %d and starttime > %d and starttime < %d order by starttime", config->table, config->e_nid, config->s_nid, config->e_nid, config->s_nid, ts, ts + TTL);
		mysqlpp::Query query = conn.query(_query);

		if (mysqlpp::StoreQueryResult res = query.store()){
			//greedy scheme
		//mcp scheme
		cout << "Communication using MCP scheme for routing:" << endl;
			for (size_t i = 0; i < res.num_rows(); ++i) {
				//going through db and finding contact duration time for each contact and store them inside map table
				int stime = res[i][2];	//starting time
				int etime = res[i][3];	//end time
				int id1 = res[i][0];	//user1
				int id2 = res[i][1];	//user2
				int current_time = stime;	//emulation clock is updated
				//let's check if user1 or user2 has video for the other one at a time <= stime, therefore we have to check the buffers of two users
				list<message>::iterator y, z, it1, it2, it3,it4;	// an iterator for the buffer link list
				
				//let's go through id1's buffer to find expired message, we should update the traffic profile and remove those message from the buffer
				timeout(id1, id2, current_time, log);	//let's go through id1 and id2 buffers and remove all timeout messages

				
				//now let's compare T1 and T2 to see what should we flood
				//read id1's and id2's buffers to see which messages should be copied to the ther one's buffer
				int flag1 = 0;	// to see if we should erase anything from the buffer of id1  
				//let's go through id1.buffer to see if there is any msg to be copied from ID1--> ID2
				//We are looking for T12 = T1-T2
				for(z = nodes[id1-config->s_nid].buffer_list.begin(); z != nodes[id1-config->s_nid].buffer_list.end(); ++z){
					flag1 = 0;
					for(y = nodes[id2-config->s_nid].buffer_list.begin(); y != nodes[id2-config->s_nid].buffer_list.end(); ++y){		
						if ((*z).id == (*y).id){
							flag1 = 1;	//this message has already been found in id2's buffer
							break;		//we dont need to copy this message
						}
					}
					//let's uniformly transmit the messages
					
					if(flag1 == 0 && (*z).gen_t <= current_time && (*z).hopttl >= 1 && (*z).mcp > 0){
						//number of flooding is bounded by cg array
						//cout << "mesage id: " << (*z).id << " does not exist in " << id2 << "'s buffer" << endl;
						//T12 != null
						//we should copy the message from id1's buffer to id2's buffer
						if((current_time - (*z).gen_t <= TTL) && ((*z).delivered == false)){//check if msg has been expired
							//message has not been expired
							if(id1 != (*z).rid){//check if ID1 is the receiver of the message, if so then we dont need o copy the msg from ID1 to ID2!
							//core of routing scheme decision: Flooding
								(*z).mcp--;//update the number of flooding for this message
								if (DEBUG) log << "message id " << (*z).id << " is copied from " << id1 << " to " << id2 << " buffer at: " << current_time <<  " ttl " << (*z).hopttl << " mcp " << (*z).mcp << endl;
								msg.id = (*z).id;
								msg.sid = (*z).sid;;
								msg.rid = (*z).rid;
								msg.gen_t = (*z).gen_t;
								msg.no_hops = (*z).no_hops + 1;
								msg.mcp = mcp;	//no of copies is originally mcp
								msg.hopttl = (*z).hopttl - 1;	//hop count decrements
								if(id2 == (*z).rid)	msg.delivered = true;//this is the destination of msg
								else msg.delivered = false;
								nodes[id2-config->s_nid].buffer_list.push_back(msg);	// copy the message to the end of the id2's buffer!
								(*z).cost++;	//since we have transmitted this message from id1 to id2
								//let's check if ID2 is the msg.rcvr
								if(id2 == (*z).rid && (*z).gen_t <= stime){
									(*z).delivered = true;	//top transmission of this msg
									//Go to the corresponding entry of traffic profile and updates the attributes
									//id1 has a video for id2
									if (DEBUG) log << "node " << id2 << " is the receiver" << endl;
									if (DEBUG) log << "message " << (*z).id << " generated at " << (*z).gen_t << " delivered at " << stime << " by " << id1 << endl;
									
									double delay = (double)(stime - (*z).gen_t)/(double)3600.0;
									if (DEBUG) log << "delay of delivery: " << delay << endl;
									it1 = z;	//let's save the iterator to remove this buffer from id1's buffer???
	
									//let's update the traffic profile table for delivered videos
									for(k = traf_list[(*z).sid-config->s_nid].begin(); k != traf_list[(*z).sid-config->s_nid].end(); ++k){ 
										if((*k).sndr_id == (*z).sid && (*k).rcvr_id == id2 && (*k).gen_t == (*z).gen_t){
											//successful delivery
											(*k).success = 1;	
											(*k).hops = (*z).no_hops + 1;	//one hop transmission	
											(*k).delay = stime - (*z).gen_t;//delay in sec
											(*k).rcv_t = stime;	//received time in sec
										}
											
										if (DEBUG) log << "1updating profile for node: " << (*k).sndr_id << " to node: " << (*k).rcvr_id << " in traffic profile table!" << endl; // print member
										//cout << endl;
									}
								}
							}//else since ID1 is the msg.rcvr --> NOP
						}
					}	
				}


				//read id2's and id1's buffers to see which messages should be copied to the ther one's buffer
				flag1 = 0;	// to see if we should erase anything from the buffer of id2  
				//let's go through id2.buffer to see if there is any msg to be copied from ID2--> ID1
				//We are looking for T21 = T2-T1
				for(z = nodes[id2-config->s_nid].buffer_list.begin(); z != nodes[id2-config->s_nid].buffer_list.end(); ++z){
					flag1 = 0;
					for(y = nodes[id1-config->s_nid].buffer_list.begin(); y != nodes[id1-config->s_nid].buffer_list.end(); ++y){		
						if ((*z).id == (*y).id){
							flag1 = 1;	//this message has already been copied to id2's buffer
							break;		//stop processing more
						}
					}

					if(flag1 == 0 && (*z).gen_t <= current_time && (*z).hopttl >= 1 && (*z).mcp > 0){
						//cout << "mesage id: " << (*z).id << " does not exist in " << id1 << "'s buffer" << endl;
						//T12 != null
						//we should copy the message from id1's buffer to id2's buffer
						if((current_time - (*z).gen_t <= TTL) && ((*z).delivered == false)){//check if msg has been expired
							//message has not been expired
							if(id2 != (*z).rid){//check if ID2 is the receiver of the message, if so then we dont need o copy the msg from ID1 to ID2!
							//core of routing scheme decision: Flooding
								(*z).mcp--;	//update the number of flooding for this message
								if (DEBUG) log << "mesage id " << (*z).id << " copied from " << id2 << " to " << id1 << " 's buffer at: " << current_time <<  " ttl " << (*z).hopttl << " mcp " << (*z).mcp << endl;

								msg.id = (*z).id;
								msg.sid = (*z).sid;;
								msg.rid = (*z).rid;
								msg.gen_t = (*z).gen_t;
								msg.no_hops = (*z).no_hops + 1;
								msg.hopttl = (*z).hopttl - 1;	//decremet hop ttl by one
								msg.mcp = mcp;	//no of copies
								if(id1 == (*z).rid)	msg.delivered = true;//this is the destination of msg
								else msg.delivered = false;
								nodes[id1-config->s_nid].buffer_list.push_back(msg);	// copy the message to the end of the id2's buffer!
								(*z).cost++;	//since we have transmitted this message from id1 to id2
								//let's check if ID2 is the msg.rcvr
								if(id1 == (*z).rid && (*z).gen_t <= stime){
									(*z).delivered = true;	//stop sending this message
									//Go to the corresponding entry of traffic profile and updates the attributes
									//id1 has a video for id2
									if (DEBUG) log << "node " << id1 << " is receiver" << endl;
									if (DEBUG) log << "message " << (*z).id << " generated at " << (*z).gen_t << " delivered at: " << stime << " by " << id2 << endl;
									double delay = (double)(stime - (*z).gen_t)/(double)3600.0;
									
									if (DEBUG) log << "delivery delay: " << delay << endl;
									it1 = z;	//let's save the iterator to remove this buffer from id1's buffer
	
									//let's update the traffic profile table for delivered videos
									for(k = traf_list[(*z).sid-config->s_nid].begin(); k != traf_list[(*z).sid-config->s_nid].end(); ++k){ 
										if((*k).sndr_id == (*z).sid && (*k).rcvr_id == id1 && (*k).gen_t == (*z).gen_t){
											//successful delivery
											(*k).success = 1;	
											(*k).hops = (*z).no_hops + 1;	//one hop transmission	
											(*k).delay = stime - (*z).gen_t;//delay in sec
											(*k).rcv_t = stime;	//received time in sec
										}
											
										if (DEBUG) log << "2updating profile for node: " << (*k).sndr_id << " to node: " << (*k).rcvr_id << " in traffic profile table!" << endl; // print member
										//cout << endl;
									}
								}
							}//else since ID1 is the msg.rcvr --> NOP
						}
					}	
				}
			}//end for

			//let's virtually clean up all the timeout messages from nodes buffers
			int i;
			int current_time = ts + 2*TTL + 1; //update for solving the bug of msgs with uniform gen times
			for (i = config->s_nid; i <= config->e_nid; i++){
				//let's go through id1's buffer to find expired message, we should update the traffic profile and remove those message from the buffer
				timeout(i, current_time, log);
			}

			//let's store delay in delayM.dat file: Waiting Scheme
			ofstream delayM( "/home/kazem/Desktop/traces/logs/delayM.dat", ios::app );
				
			// exit program if unable to create file
				
			ofstream costM( "/home/kazem/Desktop/traces/logs/costM.dat", ios::app );
			
			// exit program if unable to create file
			
			if ( !delayM || !costM ) // overloaded ! operator
			{
			cerr << "File delayM.dat or costM could not be opened" << endl;
			return -1;
			} // end if
			
			write_profile(delayM, costM, time, 3);
			delayM.close();
			costM.close();
		        log << "-----------------------------------------------------------------------" << endl;
			log.close();
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
//let's create the label table for all nodes: which community each node belongs to
int routing::fill_labels(){
	cout << "preprocessing for LABEL table" << endl;
	//let's cache all of social distances in a two-dimensional array for pre-processing
	cg.findLabels();
	cout << "endinig pre-processing!" << endl;
	return 0;
}
 
//Label forewarding method
int routing::Label(int time){
	list<traffic>::iterator k;	// an iterator for the link list
	message msg; 
	//let's store all interaction of networks in log.dat file
	ofstream log( "/home/kazem/Desktop/traces/logs/log.dat", ios::app );
	
	// exit program if unable to create file
	if ( !log ) // overloaded ! operator
	{
		cerr << "File log.dat could not be opened" << endl;
		return -1;
	}


	// Connect to database 
	mysqlpp::Connection conn(false);

	/* Connect to database */
	if (conn.connect(config->db, "localhost", "root", "mysql")) {
		// Retrieve a subset of the sample stock table set up by resetdb
		// and display it
		cout << "Connected to db!" << endl;
		
		char _query [500];
		bzero(_query, 500);
		//we should count all contacts: not only long ones!!!
		sprintf (_query, "select user1, user2, starttime, endtime from %s where user1 <= %d and user1 >= %d and user2 <= %d and user2 >= %d and starttime > %d and starttime < %d order by starttime", config->table, config->e_nid, config->s_nid, config->e_nid, config->s_nid, ts, ts + TTL);	
		mysqlpp::Query query = conn.query(_query);

		if (mysqlpp::StoreQueryResult res = query.store()){
			//LABEL SCHEME
			cout << "Communication using LABEL scheme for routing:" << endl;
		//let's cache all of social distances in a two-dimensional array for pre-processing
			for (size_t i = 0; i < res.num_rows(); ++i) {
				//going through db and finding contact duration time for each contact and store them inside map table
				int stime = res[i][2];	//starting time
				int etime = res[i][3];	//end time
				int id1 = res[i][0];	//user1
				int id2 = res[i][1];	//user2
				int current_time = stime;	//emulation clock is updated
				//let's check if user1 or user2 has video for the other one at a time <= stime, therefore we have to check the buffers of two users
				list<message>::iterator y, z, it1, it2, it3,it4;	// an iterator for the buffer link list

				//let's go through id1's buffer to find expired message, we should update the traffic profile and remove those message from the buffer
				timeout(id1, id2, current_time, log);	//let's go through id1 and id2 buffers and remove all timeout messages
				
				//now let's compare T1 and T2 to see what should we flood
				//read id1's and id2's buffers to see which messages should be copied to the ther one's buffer
				int flag1 = 0;	// to see if we should erase anything from the buffer of id1  
				//let's go through id1.buffer to see if there is any msg to be copied from ID1--> ID2
				//We are looking for T12 = T1-T2
				for(z = nodes[id1-config->s_nid].buffer_list.begin(); z != nodes[id1-config->s_nid].buffer_list.end(); ++z){
					flag1 = 0;

					for(y = nodes[id2-config->s_nid].buffer_list.begin(); y != nodes[id2-config->s_nid].buffer_list.end(); ++y){		
						if ((*z).id == (*y).id){
							flag1 = 1;	//this message has already been found in id2's buffer
							break;
						}
					}
					if(flag1 == 0 && (*z).gen_t <= current_time){
						//cout << "mesage id: " << (*z).id << " does not exist in " << id2 << "'s buffer" << endl;
						//T12 != null
						//we should copy the message from id1's buffer to id2's buffer
						if(current_time - (*z).gen_t <= TTL){//check if msg has been expired
							//message has not been expired
							if(id1 != (*z).rid){//check if ID1 is the receiver of the message, if so then we dont need o copy the msg from ID1 to ID2!
							//core of routing scheme decision: Flooding

							//LABEL: forward if the id2 has the same label as the destination
							if(cg.label(id2, (*z).rid) && ((*z).delivered == false)){

									if (DEBUG) log << "message id " << (*z).id << " copied from " << id1 << " to " << id2 << " 's buffer at " << current_time << " closeness (sender, dst) " << closeness[id1-config->s_nid][(*z).rid-config->s_nid] << " closeness (receiver, dst) " << closeness[id2-config->s_nid][(*z).rid-config->s_nid]  << endl << endl;
								
								
									//greedy: copy of the relay is closer to desination
									msg.id = (*z).id;
									msg.sid = (*z).sid;;
									msg.rid = (*z).rid;
									msg.gen_t = (*z).gen_t;
									msg.no_hops = (*z).no_hops + 1;
									if(id2 == (*z).rid)	msg.delivered = true;//this is the destination of msg
									else msg.delivered = false;
									nodes[id2-config->s_nid].buffer_list.push_back(msg);	// copy the message to the end of the id2's buffer!
									if(id2 != (*z).rid) (*z).cost++;	//since we have transmitted this message from id1 to id2
								}
								//let's check if ID2 is the msg.rcvr
								if(id2 == (*z).rid && (*z).gen_t <= stime){
									(*z).cost++;
									(*z).delivered = true;	//stp sending this msg anymore
									//Go to the corresponding entry of traffic profile and updates the attributes
									//id1 has a video for id2
									if (DEBUG) log << "node " << id2 << " is receiver" << endl;
									if (DEBUG) log << "message " << (*z).id << " generated at " << (*z).gen_t << " delivered at " << stime << " by " << id1 << endl;
									
									double delay = (double)(stime - (*z).gen_t)/(double)3600.0;
									if (DEBUG) log << "delay of delivery: " << delay << endl;
									it1 = z;	//let's save the iterator to remove this buffer from id1's buffer???
	
									//let's update the traffic profile table for delivered videos
									for(k = traf_list[(*z).sid-config->s_nid].begin(); k != traf_list[(*z).sid-config->s_nid].end(); ++k){ 
										if((*k).sndr_id == (*z).sid && (*k).rcvr_id == id2 && (*k).gen_t == (*z).gen_t){
											//successful delivery
											(*k).success = 1;	
											(*k).hops = (*z).no_hops + 1;	//one hop transmission	
											(*k).delay = stime - (*z).gen_t;//delay in sec
											(*k).rcv_t = stime;	//received time in sec
										}
											
										if (DEBUG) log << "1updating profile for node: " << (*k).sndr_id << " to node: " << (*k).rcvr_id << " in traffic profile table!" << endl; // print member
										//cout << endl;
									}
								}
							}//else since ID1 is the msg.rcvr --> NOP
						}
					}	
				}


				//read id2's and id1's buffers to see which messages should be copied to the ther one's buffer
				flag1 = 0;	// to see if we should erase anything from the buffer of id2  
				//let's go through id2.buffer to see if there is any msg to be copied from ID2--> ID1
				//We are looking for T21 = T2-T1
				for(z = nodes[id2-config->s_nid].buffer_list.begin(); z != nodes[id2-config->s_nid].buffer_list.end(); ++z){
					flag1 = 0;
					for(y = nodes[id1-config->s_nid].buffer_list.begin(); y != nodes[id1-config->s_nid].buffer_list.end(); ++y){		
						if ((*z).id == (*y).id){
							flag1 = 1;	//this message has already been copied to id2's buffer
							break;		//we should break since this message (*z).id is in id2's buffer
						}
					}
					if(flag1 == 0 && (*z).gen_t <= current_time){
						//cout << "mesage id: " << (*z).id << " does not exist in " << id1 << "'s buffer" << endl;
						//T12 != null
						//we should copy the message from id1's buffer to id2's buffer
						if(current_time - (*z).gen_t <= TTL){//check if msg has been expired
							//message has not been expired
							if(id2 != (*z).rid){//check if ID2 is the receiver of the message, if so then we dont need o copy the msg from ID1 to ID2!
							//core of routing scheme decision: Flooding

							//LABEL: forward iff the destination has the same label as id1
							if(cg.label(id1, (*z).rid) && ((*z).delivered == false)){

									if (DEBUG) log << "message id " << (*z).id << " copied from " << id2 << " to " << id1 << " 's buffer at " << current_time << " closeness (sender, dst) " << closeness[id2-config->s_nid][(*z).rid-config->s_nid] << " closeness (receiver, dst) " << closeness[id1-config->s_nid][(*z).rid-config->s_nid]  << endl << endl;
								
									//if relay is closer to destination, copy the file
									msg.id = (*z).id;
									msg.sid = (*z).sid;;
									msg.rid = (*z).rid;
									msg.gen_t = (*z).gen_t;
									msg.no_hops = (*z).no_hops + 1;
									if(id2 == (*z).rid)	msg.delivered = true;//this is the destination of msg
									else msg.delivered = false;
									nodes[id1-config->s_nid].buffer_list.push_back(msg);	// copy the message to the end of the id2's buffer!
									if(id1 != (*z).rid) (*z).cost++;	//since we have transmitted this message from id1 to id2
								}
								//let's check if ID2 is the msg.rcvr
								if(id1 == (*z).rid && (*z).gen_t <= stime){
									(*z).cost++;
									(*z).delivered = true;	//stop sending anymore
									//Go to the corresponding entry of traffic profile and updates the attributes
									//id1 has a video for id2
									if (DEBUG) log << "node: " << id1 << " is receiver" << endl;
									if (DEBUG) log << "message " << (*z).id << " generated at " << (*z).gen_t << " delivered at " << stime << " by " << id2 << endl;
									double delay = (double)(stime - (*z).gen_t)/(double)3600.0;
									
									if (DEBUG) log << "delivery delay: " << delay << endl;
									it1 = z;	//let's save the iterator to remove this buffer from id1's buffer
	
									//let's update the traffic profile table for delivered videos
									for(k = traf_list[(*z).sid-config->s_nid].begin(); k != traf_list[(*z).sid-config->s_nid].end(); ++k){ 
										if((*k).sndr_id == (*z).sid && (*k).rcvr_id == id1 && (*k).gen_t == (*z).gen_t){
											//successful delivery
											(*k).success = 1;	
											(*k).hops = (*z).no_hops + 1;	//one hop transmission	
											(*k).delay = stime - (*z).gen_t;//delay in sec
											(*k).rcv_t = stime;	//received time in sec
										}
											
										if (DEBUG) log << "2updating profile for node: " << (*k).sndr_id << " to node: " << (*k).rcvr_id << " in traffic profile table!" << endl; // print member
										//cout << endl;
									}
								}
							}//else since ID1 is the msg.rcvr --> NOP
						}
					}	
				}
			}//end for
			//let's virtually clean up all the timeout messages from nodes buffers
			int i;
			int current_time = ts + 2*TTL + 1; //update for solving the bug of msgs with uniform gen times
			for (i = config->s_nid; i <= config->e_nid; i++){
				//let's go through id1's buffer to find expired message, we should update the traffic profile and remove those message from the buffer
				timeout(i, current_time, log);
			}

			//let's store delay in delayL.dat file: Waiting Scheme
			ofstream delayL( "/home/kazem/Desktop/traces/logs/delayL.dat", ios::app );
				
			// exit program if unable to create file
				
			ofstream costL( "/home/kazem/Desktop/traces/logs/costL.dat", ios::app );
			
			// exit program if unable to create file
			
			if ( !delayL || !costL ) // overloaded ! operator
			{
			cerr << "File delayL.dat or costL could not be opened" << endl;
			return -1;
			} // end if
			
			write_profile(delayL, costL, time, 4);
			delayL.close();
			costL.close();
		        log << "-----------------------------------------------------------------------" << endl;
			log.close();
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

//Bubble forwarding method
//arguments: time = scheme no, wLen= window length for counting unique encouners of each node, and uTime= update duration for calling uRank function

int routing::Bubble(int time, int wLen, int uTime){
	list<traffic>::iterator k;	// an iterator for the link list
	message msg; 
	int r_id1 = 0;	//the rank of node id1
	int r_id2 = 0;	//rank of node id2
	bool l_id1D = false;	//is true if id1 has the same label (group) as the msg's dest
	bool l_id2D = false;	//is true if id2 has the same label (group) as the msg's dest

	int lUpdate = 0;	//this is for ranking updates
	//let's store all interaction of networks in log.dat file
	ofstream log( "/home/kazem/Desktop/traces/logs/log.dat", ios::app );
	
	// exit program if unable to create file
	if ( !log ) // overloaded ! operator
	{
		cerr << "File log.dat could not be opened" << endl;
		return -1;
	}


	// Connect to database 
	mysqlpp::Connection conn(false);

	/* Connect to database */
	if (conn.connect(config->db, "localhost", "root", "mysql")) {
		// Retrieve a subset of the sample stock table set up by resetdb
		// and display it
		cout << "Connected to db!" << endl;
		
		char _query [500];
		bzero(_query, 500);
		//we should count all contacts: not only long ones!!!
//		sprintf (_query, "select user1, user2, starttime, endtime from %s where (endtime-starttime) > 100 and user1 <= %d and user1 >= %d and user2 <= %d and user2 >= %d and starttime > %d and starttime < %d order by starttime", config->table, config->e_nid, config->s_nid, config->e_nid, config->s_nid, ts, ts + TTL);
		//we have found a bug: not only long contacts!!!
		//we have found that by removing the long contacts from our mysql query, it release much more energy for delivering more messages to their destination

		sprintf (_query, "select user1, user2, starttime, endtime from %s where user1 <= %d and user1 >= %d and user2 <= %d and user2 >= %d and starttime > %d and starttime < %d order by starttime", config->table, config->e_nid, config->s_nid, config->e_nid, config->s_nid, ts, ts + TTL);

		mysqlpp::Query query = conn.query(_query);

		if (mysqlpp::StoreQueryResult res = query.store()){
			//BUBBLE SCHEME
			cout << "Communication using Bubble scheme for routing:" << endl;
			for (size_t i = 0; i < res.num_rows(); ++i) {
				//going through db and finding contact duration time for each contact and store them inside map table
				int stime = res[i][2];	//starting time
				int etime = res[i][3];	//end time
				int id1 = res[i][0];	//user1
				int id2 = res[i][1];	//user2
				int current_time = stime;	//emulation clock is updated
				//let's check if user1 or user2 has video for the other one at a time <= stime, therefore we have to check the buffers of two users
				list<message>::iterator y, z, it1, it2, it3,it4;	// an iterator for the buffer link list

				//let's go through id1's buffer to find expired message, we should update the traffic profile and remove those message from the buffer
				timeout(id1, id2, current_time, log);	//let's go through id1 and id2 buffers and remove all timeout messages
				if((lUpdate == 0 || (current_time - lUpdate) > uTime) && ((current_time - ts) <= TTL)){
					//we have to update the rank list for all users
					cg.updateRanks(ts, current_time, wLen, 0);
					lUpdate = current_time;		//update the last update pointer for rank lists
				}
				//now let's compare T1 and T2 to see what should we copy
				//read id1's and id2's buffers to see which messages should be copied to the ther one's buffer
				int flag1 = 0;	// to see if we should erase anything from the buffer of id1  
				//let's go through id1.buffer to see if there is any msg to be copied from ID1--> ID2
				//We are looking for T12 = T1-T2
				for(z = nodes[id1-config->s_nid].buffer_list.begin(); z != nodes[id1-config->s_nid].buffer_list.end(); ++z){
					flag1 = 0;

					for(y = nodes[id2-config->s_nid].buffer_list.begin(); y != nodes[id2-config->s_nid].buffer_list.end(); ++y){		
						if ((*z).id == (*y).id){
							flag1 = 1;	//this message has already been found in id2's buffer
							break;
						}
					}
					if(flag1 == 0 && (*z).gen_t <= current_time){
						//cout << "mesage id: " << (*z).id << " does not exist in " << id2 << "'s buffer" << endl;
						//T12 != null
						//we should copy the message from id1's buffer to id2's buffer
						if((current_time - (*z).gen_t <= TTL)  && ((*z).delivered == false)){//check if msg has been expired
							//message has not been expired
							if(id1 != (*z).rid){//check if ID1 is the receiver of the message, if so then we dont need o copy the msg from ID1 to ID2!
							//core of routing scheme decision: Flooding

							//Bubble: forward if the id2 has the same label as the destination and higher rank!
							//this means that id1 has the same label as dest
//							if((*z).gRank == true){
							r_id1 = cg.rank(id1);
							r_id2 = cg.rank(id2);
							l_id1D = cg.label(id1, (*z).rid);
							l_id2D = cg.label(id2, (*z).rid);
							if(l_id1D/*cg.label(id1, (*z).rid)*/){
								if(r_id2 > r_id1 && l_id2D/*cg.rank(id2) > cg.rank(id1) && cg.label(id2, (*z).rid)*/){
									if (DEBUG) log << "message id " << (*z).id << " copied from " << id1 << " to " << id2 << " 's buffer at " << current_time << " id2 has the same label as dest and rank(s) " << r_id1 << " relay's rank " << r_id2 << endl << endl;
								
									//greedy: copy of the relay is closer to desination
									msg.id = (*z).id;
									msg.sid = (*z).sid;;
									msg.rid = (*z).rid;
									msg.gen_t = (*z).gen_t;
									msg.no_hops = (*z).no_hops + 1;
									if(id2 == (*z).rid)	msg.delivered = true;//this is the destination of msg
									else msg.delivered = false;
									(*z).cost++;	//since we have transmitted this message from id1 to id2
									if(cg.label(id2, (*z).rid)){
										(*z).gRank = false;
										msg.gRank = false;	//id2 has the same label as dest, so the data hast to be only forwarded to nodes from the same community 
									}
									nodes[id2-config->s_nid].buffer_list.push_back(msg);	// copy the message to the end of the id2's buffer!				
								}
							}
							else{//if the cureent node does not have the same label as dest, we will forward the message to encountered node if the encountered node has the same label as the messgae destination or has higher rank than current node
								if(r_id2 > r_id1 || l_id2D /*cg.rank(id2) > cg.rank(id1) || cg.label(id2, (*z).rid)*/){
									if (DEBUG) log << "message id " << (*z).id << " copied from: " << id1 << " to " << id2 << " 's buffer at " << current_time  << " id2 has the same label as dest or higher rank than current node or both rank(s) " << r_id1 << " relay's rank " << r_id2 << endl << endl;
								
									//greedy: copy of the relay is closer to desination
									msg.id = (*z).id;
									msg.sid = (*z).sid;;
									msg.rid = (*z).rid;
									msg.gen_t = (*z).gen_t;
									msg.no_hops = (*z).no_hops + 1;
									msg.gRank = false;	//id2 has the same label as
									if(id2 == (*z).rid)	msg.delivered = true;//this is the destination of msg
									else msg.delivered = false;
									nodes[id2-config->s_nid].buffer_list.push_back(msg);	// copy the message to the end of the id2's buffer!
									(*z).cost++;	//since we have transmitted this message from id1 to id2
								}
							}

								//let's check if ID2 is the msg.rcvr
								if(id2 == (*z).rid && (*z).gen_t <= stime){
									(*z).delivered = true;
									//Go to the corresponding entry of traffic profile and updates the attributes
									//id1 has a video for id2
									if (DEBUG) log << "node: " << id2 << " is receiver" << endl;
									if (DEBUG) log << "message " << (*z).id << " generated at " << (*z).gen_t << " delivered at " << stime << " by " << id1 << endl;
									
									double delay = (double)(stime - (*z).gen_t)/(double)3600.0;
									if (DEBUG) log << "delay of delivery: " << delay << endl;
									it1 = z;	//let's save the iterator to remove this buffer from id1's buffer???
	
									//let's update the traffic profile table for delivered videos
									for(k = traf_list[(*z).sid-config->s_nid].begin(); k != traf_list[(*z).sid-config->s_nid].end(); ++k){ 
										if((*k).sndr_id == (*z).sid && (*k).rcvr_id == id2 && (*k).gen_t == (*z).gen_t){
											//successful delivery
											(*k).success = 1;	
											(*k).hops = (*z).no_hops + 1;	//one hop transmission	
											(*k).delay = stime - (*z).gen_t;//delay in sec
											(*k).rcv_t = stime;	//received time in sec
										}
											
										if (DEBUG) log << "1updating profile for node: " << (*k).sndr_id << " to node: " << (*k).rcvr_id << " in traffic profile table!" << endl; // print member
										//cout << endl;
									}
								}
							}//else since ID1 is the msg.rcvr --> NOP
						}
					}	
				}


				//read id2's and id1's buffers to see which messages should be copied to the ther one's buffer
				flag1 = 0;	// to see if we should erase anything from the buffer of id2  
				//let's go through id2.buffer to see if there is any msg to be copied from ID2--> ID1
				//We are looking for T21 = T2-T1
				for(z = nodes[id2-config->s_nid].buffer_list.begin(); z != nodes[id2-config->s_nid].buffer_list.end(); ++z){
					flag1 = 0;
					for(y = nodes[id1-config->s_nid].buffer_list.begin(); y != nodes[id1-config->s_nid].buffer_list.end(); ++y){		
						if ((*z).id == (*y).id){
							flag1 = 1;	//this message has already been copied to id2's buffer
							break;		//we should break since this message (*z).id is in id2's buffer
						}
					}
					if(flag1 == 0 && (*z).gen_t <= current_time){
						//cout << "mesage id: " << (*z).id << " does not exist in " << id1 << "'s buffer" << endl;
						//T12 != null
						//we should copy the message from id2's buffer to id1's buffer
						if((current_time - (*z).gen_t <= TTL) && ((*z).delivered == false)){//check if msg has been expired
							//message has not been expired
							if(id2 != (*z).rid){//check if ID2 is the receiver of the message, if so then we dont need o copy the msg from ID1 to ID2!
							//core of routing scheme decision: Flooding
							//Bubble: forward if the id2 has the same label as the destination or higher rank!
//							if((*z).gRank == true){
								//if the current node has the same label as dest, we will forward the message only if id1 has the same label as the dest and higher local rank than id2 as well
								r_id1 = cg.rank(id1);
								r_id2 = cg.rank(id2);
								l_id1D = cg.label(id1, (*z).rid);
								l_id2D = cg.label(id2, (*z).rid);
								if(l_id2D/*cg.label(id2, (*z).rid)*/){
								if(r_id1 > r_id2 && l_id1D/*cg.rank(id1) > cg.rank(id2) && cg.label(id1, (*z).rid)*/){
									if (DEBUG) log << "message id " << (*z).id << " copied from " << id2 << " to " << id1 << " 's buffer at " << current_time << " id1 has the same label as dest and, rank(s) " << r_id2 << " relay's rank " << r_id1 << endl << endl;
								
								
									//greedy: copy of the relay is closer to desination
									msg.id = (*z).id;
									msg.sid = (*z).sid;;
									msg.rid = (*z).rid;
									msg.gen_t = (*z).gen_t;
									msg.no_hops = (*z).no_hops + 1;
									(*z).cost++;	//since we have transmitted this message from id2 to id1
									if(id1 == (*z).rid)	msg.delivered = true;//this is the destination of msg
									else msg.delivered = false;
									if(cg.label(id1, (*z).rid)){
										(*z).gRank = false;
										msg.gRank = false;	//the id1 is in the same comunity
									}
									nodes[id1-config->s_nid].buffer_list.push_back(msg);	// copy the message to the end of the id1's buffer!
								}
							}
							else{//if the current node does not have the same label as the message destination, we will forward the message if either the encountered node is in the same community as dest or has higher global rank
								if(r_id1 > r_id2 || l_id1D/*cg.rank(id1) > cg.rank(id2) || cg.label(id1, (*z).rid)*/){
									if (DEBUG) log << "message id " << (*z).id << " copied from " << id2 << " to " << id1 << " 's buffer at " << current_time  << " id1 has the same label or higher ranks or both, rank(s) " << r_id2 << " relay's rank " << r_id1 << endl << endl;
								
									//greedy: copy of the relay is closer to desination
									msg.id = (*z).id;
									msg.sid = (*z).sid;;
									msg.rid = (*z).rid;
									msg.gen_t = (*z).gen_t;
									msg.no_hops = (*z).no_hops + 1;
									msg.gRank = false;	//id1 has the same label as
									if(id1 == (*z).rid)	msg.delivered = true;//this is the destination of msg
									else msg.delivered = false;
									nodes[id1-config->s_nid].buffer_list.push_back(msg);	// copy the message to the end of the id1's buffer!
									(*z).cost++;	//since we have transmitted this message from id2 to id1

								}
							}

								//let's check if ID1 is the msg.rcvr
								if(id1 == (*z).rid && (*z).gen_t <= stime){
									(*z).delivered = true;	//stop sending this msg anymore
									//Go to the corresponding entry of traffic profile and updates the attributes
									//id1 has a video for id2
									if (DEBUG) log << "node: " << id1 << " is receiver" << endl;
									if (DEBUG) log << "message " << (*z).id << " generated at " << (*z).gen_t << " delivered at " << stime << " by " << id2 << endl;
									double delay = (double)(stime - (*z).gen_t)/(double)3600.0;
									
									if (DEBUG) log << "delivery delay: " << delay << endl;
									it1 = z;	//let's save the iterator to remove this buffer from id1's buffer
	
									//let's update the traffic profile table for delivered videos
									for(k = traf_list[(*z).sid-config->s_nid].begin(); k != traf_list[(*z).sid-config->s_nid].end(); ++k){ 
										if((*k).sndr_id == (*z).sid && (*k).rcvr_id == id1 && (*k).gen_t == (*z).gen_t){
											//successful delivery
											(*k).success = 1;	
											(*k).hops = (*z).no_hops + 1;	//one hop transmission	
											(*k).delay = stime - (*z).gen_t;//delay in sec
											(*k).rcv_t = stime;	//received time in sec
										}
											
										if (DEBUG) log << "2updating profile for node: " << (*k).sndr_id << " to node: " << (*k).rcvr_id << " in traffic profile table!" << endl; // print member
										//cout << endl;
									}
								}
							}//else since ID1 is the msg.rcvr --> NOP
						}
					}	
				}
			}//end for

			//let's virtually clean up all the timeout messages from nodes buffers
			int i;
			int current_time = ts + 2*TTL + 1; //update for solving the bug of msgs with uniform gen times
			for (i = config->s_nid; i <= config->e_nid; i++){
				//let's go through id1's buffer to find expired message, we should update the traffic profile and remove those message from the buffer
				timeout(i, current_time, log);
			}

			//let's store delay in delayB.dat file: Bubble Scheme
			ofstream delayB( "/home/kazem/Desktop/traces/logs/delayB.dat", ios::app );
				
			// exit program if unable to create file
				
			ofstream costB( "/home/kazem/Desktop/traces/logs/costB.dat", ios::app );
			
			// exit program if unable to create file
			
			if ( !delayB || !costB ) // overloaded ! operator
			{
			cerr << "File delayB.dat or costB could not be opened" << endl;
			return -1;
			} // end if
			
			write_profile(delayB, costB, time, 5);
			delayB.close();
			costB.close();
		        log << "-----------------------------------------------------------------------" << endl;
			log.close();
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

//GreedyB forwarding method
//metric:0 --> use only similarity
//metric:1 --> use centrality and similarity
//metric:2 --> only centrality
int routing::GreedyB(int time, int wLen, int uTime, int metric, double thresh){
	list<traffic>::iterator k;	// an iterator for the link list
	double util_id1 = 0;	//id1's utility
	double util_id2 = 0;	//id2's utility
	int r_id1 = 0;		//id1's rank
	int r_id2 = 0;		//id2's rank
	int lUpdate = 0;	//this is for ranking updates
	int upCost = 23;	//upper bound for the cost 
	double alpha = 1.0;	//the update factor
	double update = 1.0;

	message msg; 
	//let's store all interaction of networks in log.dat file
	ofstream log( "/home/kazem/Desktop/traces/logs/log.dat", ios::app );
	
	// exit program if unable to create file
	
	if ( !log ) // overloaded ! operator
	{
		cerr << "File log.dat could not be opened" << endl;
		return -1;
	}

	// Connect to database 
	mysqlpp::Connection conn(false);

	/* Connect to database */
	if (conn.connect(config->db, "localhost", "root", "mysql")) {
		// Retrieve a subset of the sample stock table set up by resetdb
		// and display it
		cout << "Connected to db!" << endl;
		
		char _query [500];
		bzero(_query, 500);
		//we should count all contacts: not only long ones!!!
//		sprintf (_query, "select user1, user2, starttime, endtime from %s where (endtime-starttime) > 100 and user1 <= %d and user1 >= %d and user2 <= %d and user2 >= %d and starttime > %d and starttime < %d order by starttime", config->table, config->e_nid, config->s_nid, config->e_nid, config->s_nid, ts, ts + TTL);
		sprintf (_query, "select user1, user2, starttime, endtime from %s where user1 <= %d and user1 >= %d and user2 <= %d and user2 >= %d and starttime > %d and starttime < %d order by starttime", config->table, config->e_nid, config->s_nid, config->e_nid, config->s_nid, ts, ts + TTL);

		mysqlpp::Query query = conn.query(_query);

		if (mysqlpp::StoreQueryResult res = query.store()){
			//greedy scheme
			cout << "Communication using Social-Greedy II scheme for routing:" << endl;
//			double thresh = 0.1;	//threshod for routing
	
			for (size_t i = 0; i < res.num_rows(); ++i) {
				//going through db and finding contact duration time for each contact and store them inside map table
				int stime = res[i][2];	//starting time
				int etime = res[i][3];	//end time
				int id1 = res[i][0];	//user1
				int id2 = res[i][1];	//user2
				int current_time = stime;	//emulation clock is updated
				//let's check if user1 or user2 has video for the other one at a time <= stime, therefore we have to check the buffers of two users
				list<message>::iterator y, z, it1, it2, it3, it4;	// an iterator for the buffer link list
				map<int,int>::iterator itM;	//map for keeping track of costs
				
				if(metric != 0){
					//we are intersted in contact rates per unit time (total no of contacts for the last wlen)
					if((lUpdate == 0 || (current_time - lUpdate) > uTime) && ((current_time - ts) <= TTL)){
						//we have to update the rank list for all users
						cg.updateRanks(ts, current_time, wLen, 0);
						lUpdate = current_time;		//update the last update pointer for rank lists
					}
					//there is a contact between id1 and id2
					r_id1 = cg.rank(id1);
					r_id2 = cg.rank(id2);
				}
				//let's go through id1's buffer to find expired message, we should update the traffic profile and remove those message from the buffer
				timeout(id1, id2, current_time, log);	//let's go through id1 and id2 buffers and remove all timeout messages
	
				
				//now let's compare T1 and T2 to see what should we flood
				//read id1's and id2's buffers to see which messages should be copied to the ther one's buffer
				int flag1 = 0;	// to see if we should erase anything from the buffer of id1  
				//let's go through id1.buffer to see if there is any msg to be copied from ID1--> ID2
				//We are looking for T12 = T1-T2
				for(z = nodes[id1-config->s_nid].buffer_list.begin(); z != nodes[id1-config->s_nid].buffer_list.end(); ++z){
					flag1 = 0;
	
					for(y = nodes[id2-config->s_nid].buffer_list.begin(); y != nodes[id2-config->s_nid].buffer_list.end(); ++y){		
						if ((*z).id == (*y).id){
							flag1 = 1;	//this message has already been found in id2's buffer
							break;
						}
					}
					if(flag1 == 0 && (*z).gen_t <= current_time){
						//cout << "mesage id: " << (*z).id << " does not exist in " << id2 << "'s buffer" << endl;
						//T12 != null
						//we should copy the message from id1's buffer to id2's buffer
						if((current_time - (*z).gen_t <= TTL) && ((*z).delivered == false)){//check if msg has been expired
							//message has not been expired
							if(id1 != (*z).rid){//check if ID1 is the receiver of the message, if so then we dont need o copy the msg from ID1 to ID2!
							//core of routing scheme decision: Flooding
				if(metric == 1){
					//let's find utilities for id1 and id2 to contact the target. this utility incorporates both similarity and centrality of  nodes
					util_id1 = 1.0 - pow(1.0 - contP[id1-config->s_nid][(*z).rid-config->s_nid], (double) r_id1);
					util_id2 = 1.0 - pow(1.0 - contP[id2-config->s_nid][(*z).rid-config->s_nid], (double) r_id2);
				}else if (metric == 2){
					util_id1 = (double) r_id1;
					util_id2 = (double) r_id2;
				}else{
					util_id1 = (double) closeness[id1-config->s_nid][(*z).rid-config->s_nid];
					util_id2 = (double) closeness[id2-config->s_nid][(*z).rid-config->s_nid];
					double nclose = alpha*closeness[id1-config->s_nid][id2-config->s_nid] + (1.0-alpha)*update;
					closeness[id1-config->s_nid][id2-config->s_nid] = nclose;
					closeness[id2-config->s_nid][id1-config->s_nid] = nclose;				
				}	
							//threshold
							if(metric == 2)	itM = curr_cost.find((*z).id);
							if((metric != 2) || itM->second <= upCost)
							if((util_id1 < util_id2 - thresh) && ((*z).maxSim < util_id2)){	
									if (DEBUG) log << "message id " << (*z).id << " copied from " << id1 << " to " << id2 << " 's buffer at " << current_time << " closeness (sender, dst) " << util_id1 << " closeness (receiver, dst) " << util_id2  << " max sim " << (*z).maxSim << endl << endl;
								
									if(id2 != (*z).rid && metric == 2) itM->second++;//another copy
									//greedy: copy of the relay is closer to desination
									msg.id = (*z).id;
									msg.sid = (*z).sid;
									msg.rid = (*z).rid;
									msg.gen_t = (*z).gen_t;
									msg.no_hops = (*z).no_hops + 1;
									msg.maxSim = 0;
									if(id2 == (*z).rid)	msg.delivered = true;//this is the destination of msg
									else msg.delivered = false;

									nodes[id2-config->s_nid].buffer_list.push_back(msg);	// copy the message to the end of the id2's buffer!
									if(id2 != (*z).rid) (*z).cost++;	//since we have transmitted this message from id1 to id2
									(*z).maxSim = util_id2;	//this is the last node with the maximum similarity to dest!
								}
								//let's check if ID2 is the msg.rcvr
								if(id2 == (*z).rid && (*z).gen_t <= stime){
									if(metric == 2) itM->second++;//another copy
									(*z).cost++;
									(*z).delivered = true;	//stop sending anymore
									//Go to the corresponding entry of traffic profile and updates the attributes
									//id1 has a video for id2
									if (DEBUG) log << "node " << id2 << " is receiver" << endl;
									if (DEBUG) log << "message " << (*z).id << " generated at " << (*z).gen_t << " delivered at " << stime << " by " << id1 << endl;
									
									double delay = (double)(stime - (*z).gen_t)/(double)3600.0;
									if (DEBUG) log << "delay of delivery: " << delay << endl;
									it1 = z;	//let's save the iterator to remove this buffer from id1's buffer???
	
									//let's update the traffic profile table for delivered videos
									for(k = traf_list[(*z).sid-config->s_nid].begin(); k != traf_list[(*z).sid-config->s_nid].end(); ++k){ 
										if((*k).sndr_id == (*z).sid && (*k).rcvr_id == id2 && (*k).gen_t == (*z).gen_t){
											//successful delivery
											(*k).success = 1;	
											(*k).hops = (*z).no_hops + 1;	//one hop transmission	
											(*k).delay = stime - (*z).gen_t;//delay in sec
											(*k).rcv_t = stime;	//received time in sec
										}
											
										if (DEBUG) log << "1updating profile for node: " << (*k).sndr_id << " to node: " << (*k).rcvr_id << " in traffic profile table!" << endl; // print member
										//cout << endl;
									}
								}
							}//else since ID1 is the msg.rcvr --> NOP
						}
					}	
				}
	
	
				//read id2's and id1's buffers to see which messages should be copied to the ther one's buffer
				flag1 = 0;	// to see if we should erase anything from the buffer of id2  
				//let's go through id2.buffer to see if there is any msg to be copied from ID2--> ID1
				//We are looking for T21 = T2-T1
				for(z = nodes[id2-config->s_nid].buffer_list.begin(); z != nodes[id2-config->s_nid].buffer_list.end(); ++z){
					flag1 = 0;
					for(y = nodes[id1-config->s_nid].buffer_list.begin(); y != nodes[id1-config->s_nid].buffer_list.end(); ++y){		
						if ((*z).id == (*y).id){
							flag1 = 1;	//this message has already been copied to id2's buffer
							break;		//we should break since this message (*z).id is in id2's buffer
						}
					}
					if(flag1 == 0 && (*z).gen_t <= current_time){
						//cout << "mesage id: " << (*z).id << " does not exist in " << id1 << "'s buffer" << endl;
						//T12 != null
						//we should copy the message from id1's buffer to id2's buffer
						if((current_time - (*z).gen_t <= TTL) && ((*z).delivered == false)){//check if msg has been expired
							//message has not been expired
							if(id2 != (*z).rid){//check if ID2 is the receiver of the message, if so then we dont need o copy the msg from ID1 to ID2!
							//core of routing scheme decision: Flooding
				if(metric == 1){
					//let's find utilities for id1 and id2 to contact the target. this utility incorporates both similarity and centrality of  nodes
					util_id1 = 1.0 - pow(1.0 - contP[id1-config->s_nid][(*z).rid-config->s_nid], (double) r_id1);
					util_id2 = 1.0 - pow(1.0 - contP[id2-config->s_nid][(*z).rid-config->s_nid], (double) r_id2);
				}else if (metric == 2){
					util_id1 = (double) r_id1;
					util_id2 = (double) r_id2;
				}else {
					util_id1 = (double) closeness[id1-config->s_nid][(*z).rid-config->s_nid];
					util_id2 = (double) closeness[id2-config->s_nid][(*z).rid-config->s_nid];
					//we update the socail distance between id1 and id2
					double nclose = alpha*closeness[id1-config->s_nid][id2-config->s_nid] + (1.0-alpha)*update;
					closeness[id1-config->s_nid][id2-config->s_nid] = nclose;
					closeness[id2-config->s_nid][id1-config->s_nid] = nclose;				
				}	
							//threshold
							if(metric == 2)	itM = curr_cost.find((*z).id);
							if((metric != 2) || (itM->second <= upCost))
							if((util_id2 < util_id1 - thresh) && ((*z).maxSim < util_id1)){	
									if (DEBUG) log << "message id " << (*z).id << " is copied from " << id2 << " to " << id1 << " 's buffer at " << current_time << " closeness (sender, dst) " << util_id2 << " closeness (receiver, dst) " << util_id1 << " max sim " << (*z).maxSim << endl << endl;
									
									if(id1 != (*z).rid && metric == 2) itM->second++; //another copy
									//if relay is closer to destination, copy the file
									msg.id = (*z).id;
									msg.sid = (*z).sid;;
									msg.rid = (*z).rid;
									msg.gen_t = (*z).gen_t;
									msg.no_hops = (*z).no_hops + 1;
									msg.maxSim = 0;
									if(id1 == (*z).rid)	msg.delivered = true;//this is the destination of msg
									else msg.delivered = false;
									nodes[id1-config->s_nid].buffer_list.push_back(msg);	// copy the message to the end of the id2's buffer!
									if(id1 != (*z).rid)	(*z).cost++;	//since we have transmitted this message from id1 to id2
									(*z).maxSim = util_id1;	//this is the last node with the maximum similarity to dest!
								}
								//let's check if ID2 is the msg.rcvr
								if(id1 == (*z).rid && (*z).gen_t <= stime){
									if(metric == 2) itM->second++; //another copy
									(*z).cost++;
									(*z).delivered = true;	//stop sending this message anymore
									//Go to the corresponding entry of traffic profile and updates the attributes
									//id1 has a video for id2
									if (DEBUG) log << "node " << id1 << " is receiver" << endl;
									if (DEBUG) log << "message " << (*z).id << " generated at " << (*z).gen_t << " delivered at " << stime << " by " << id2 << endl;
									double delay = (double)(stime - (*z).gen_t)/(double)3600.0;
									
									if (DEBUG) log << "delivery delay: " << delay << endl;
									it1 = z;	//let's save the iterator to remove this buffer from id1's buffer
	
									//let's update the traffic profile table for delivered videos
									for(k = traf_list[(*z).sid-config->s_nid].begin(); k != traf_list[(*z).sid-config->s_nid].end(); ++k){ 
										if((*k).sndr_id == (*z).sid && (*k).rcvr_id == id1 && (*k).gen_t == (*z).gen_t){
											//successful delivery
											(*k).success = 1;	
											(*k).hops = (*z).no_hops + 1;	//one hop transmission	
											(*k).delay = stime - (*z).gen_t;//delay in sec
											(*k).rcv_t = stime;	//received time in sec
										}
											
										if (DEBUG) log << "2updating profile for node: " << (*k).sndr_id << " to node: " << (*k).rcvr_id << " in traffic profile table!" << endl; // print member
										//cout << endl;
									}
								}
							}//else since ID1 is the msg.rcvr --> NOP
						}
					}	
				}
			}//end for

			//let's virtually clean up all the timeout messages from nodes buffers
			int i;
			int current_time = ts + 2*TTL + 1;	//solving a bug
			for (i = config->s_nid; i <= config->e_nid; i++){
				//let's go through id1's buffer to find expired message, we should update the traffic profile and remove those message from the buffer
				timeout(i, current_time, log);
			}

			//let's store delay in delayG.dat file: Waiting Scheme
			ofstream delayGB( "/home/kazem/Desktop/traces/logs/delayGB.dat", ios::app );
				
			// exit program if unable to create file
				
			ofstream costGB( "/home/kazem/Desktop/traces/logs/costGB.dat", ios::app );
			
			// exit program if unable to create file
		
			if ( !delayGB || !costGB ) // overloaded ! operator
			{
				cerr << "File delayGB.dat or costGB.dat could not be opened" << endl;
				return -1;
			} // end if
			//write delay and cost
			write_profile(delayGB, costGB, time, 6);
			delayGB.close();
			costGB.close();
		        log << "-----------------------------------------------------------------------" << endl;
			log.close();
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


//Greedy forwarding method
int routing::GreedyC(int time, double thresh){
	list<traffic>::iterator k;	// an iterator for the link list
	
	message msg; 
	//let's store all interaction of networks in log.dat file
	ofstream log( "/home/kazem/Desktop/traces/logs/log.dat", ios::app );
	
	// exit program if unable to create file
	
	if ( !log ) // overloaded ! operator
	{
		cerr << "File log.dat could not be opened" << endl;
		return -1;
	}

	// Connect to database 
	mysqlpp::Connection conn(false);

	/* Connect to database */
	if (conn.connect(config->db, "localhost", "root", "mysql")) {
		// Retrieve a subset of the sample stock table set up by resetdb
		// and display it
		cout << "Connected to db!" << endl;
		
		char _query [500];
		bzero(_query, 500);
		//we should count all contacts: not only long ones!!!
		sprintf (_query, "select user1, user2, starttime, endtime from %s where user1 <= %d and user1 >= %d and user2 <= %d and user2 >= %d and starttime > %d and starttime < %d order by starttime", config->table, config->e_nid, config->s_nid, config->e_nid, config->s_nid, ts, ts + TTL);
		mysqlpp::Query query = conn.query(_query);

		if (mysqlpp::StoreQueryResult res = query.store()){
			//greedy scheme
			cout << "Communication using Social-Greedy III scheme for routing:" << endl;
//			double thresh = 0.1;	//threshod for routing
	
			for (size_t i = 0; i < res.num_rows(); ++i) {
				//going through db and finding contact duration time for each contact and store them inside map table
				int stime = res[i][2];	//starting time
				int etime = res[i][3];	//end time
				int id1 = res[i][0];	//user1
				int id2 = res[i][1];	//user2
				int current_time = stime;	//emulation clock is updated
				//let's check if user1 or user2 has video for the other one at a time <= stime, therefore we have to check the buffers of two users
				list<message>::iterator y, z, it1, it2, it3,it4;	// an iterator for the buffer link list
				
				//let's go through id1's buffer to find expired message, we should update the traffic profile and remove those message from the buffer
				timeout(id1, id2, current_time, log);	//let's go through id1 and id2 buffers and remove all timeout messages
	
				
				//now let's compare T1 and T2 to see what should we flood
				//read id1's and id2's buffers to see which messages should be copied to the ther one's buffer
				int flag1 = 0;	// to see if we should erase anything from the buffer of id1  
				//let's go through id1.buffer to see if there is any msg to be copied from ID1--> ID2
				//We are looking for T12 = T1-T2
				for(z =  nodes[id1-config->s_nid].buffer_list.begin(); z != nodes[id1-config->s_nid].buffer_list.end(); ++z){
					flag1 = 0;
	
					for(y = nodes[id2-config->s_nid].buffer_list.begin(); y != nodes[id2-config->s_nid].buffer_list.end(); ++y){		
						if ((*z).id == (*y).id){
							flag1 = 1;	//this message has already been found in id2's buffer
							break;
						}
					}
					if(flag1 == 0 && (*z).gen_t <= current_time){
						//cout << "mesage id: " << (*z).id << " does not exist in " << id2 << "'s buffer" << endl;
						//T12 != null
						//we should copy the message from id1's buffer to id2's buffer
						//A message M only forwards once
						if((current_time - (*z).gen_t <= TTL) && ((*z).delivered == false) && ((*z).cost==0)){//check if msg has been expired
							//message has not been expired
							if(id1 != (*z).rid){//check if ID1 is the receiver of the message, if so then we dont need to copy the msg from ID1 to ID2!
							//core of routing scheme decision: Flooding
	
							//threshold
							//node id1 hands off the message
							if((closeness[id1-config->s_nid][(*z).rid-config->s_nid] < closeness[id2-config->s_nid][(*z).rid-config->s_nid] - thresh)){
	
									if (DEBUG) log << "message id " << (*z).id << " copied from " << id1 << " to " << id2 << " 's buffer at " << current_time << " closeness (sender, dst) " << closeness[id1-config->s_nid][(*z).rid-config->s_nid] << " closeness (receiver, dst) " << closeness[id2-config->s_nid][(*z).rid-config->s_nid]  << endl << endl;
								
									//greedy: copy of the relay is closer to desination
									msg.id = (*z).id;
									msg.sid = (*z).sid;;
									msg.rid = (*z).rid;
									msg.gen_t = (*z).gen_t;
									msg.no_hops = (*z).no_hops + 1;
									if(id2 == (*z).rid)	msg.delivered = true;//this is the destination of msg
									else msg.delivered = false;
									nodes[id2-config->s_nid].buffer_list.push_back(msg);	// copy the message to the end of the id2's buffer!
									if(id2 != (*z).rid)	(*z).cost++;	//since we have transmitted this message from id1 to id2
								}
								//let's check if ID2 is the msg.rcvr
								if(id2 == (*z).rid && (*z).gen_t <= stime){
									(*z).cost++;	//we have to count the last step delivery
									(*z).delivered = true;	//stop sending this data anymore
									//Go to the corresponding entry of traffic profile and updates the attributes
									//id1 has a video for id2
									if (DEBUG) log << "node " << id2 << " is receiver" << endl;
									if (DEBUG) log << "message " << (*z).id << " generated at " << (*z).gen_t << " delivered at " << stime << " by " << id1 << endl;
									
									double delay = (double)(stime - (*z).gen_t)/(double)3600.0;
									if (DEBUG) log << "delay of delivery: " << delay << endl;
									it1 = z;	//let's save the iterator to remove this buffer from id1's buffer???
	
									//let's update the traffic profile table for delivered videos
									for(k = traf_list[(*z).sid-config->s_nid].begin(); k != traf_list[(*z).sid-config->s_nid].end(); ++k){ 
										if((*k).sndr_id == (*z).sid && (*k).rcvr_id == id2 && (*k).gen_t == (*z).gen_t){
											//successful delivery
											(*k).success = 1;	
											(*k).hops = (*z).no_hops + 1;	//one hop transmission	
											(*k).delay = stime - (*z).gen_t;//delay in sec
											(*k).rcv_t = stime;	//received time in sec
										}
											
										if (DEBUG) log << "1updating profile for node: " << (*k).sndr_id << " to node: " << (*k).rcvr_id << " in traffic profile table!" << endl; // print member
										//cout << endl;
									}
								}
							}//else since ID1 is the msg.rcvr --> NOP
						}
					}	
				}
	
	
				//read id2's and id1's buffers to see which messages should be copied to the ther one's buffer
				flag1 = 0;	// to see if we should erase anything from the buffer of id2  
				//let's go through id2.buffer to see if there is any msg to be copied from ID2--> ID1
				//We are looking for T21 = T2-T1
				for(z = nodes[id2-config->s_nid].buffer_list.begin(); z != nodes[id2-config->s_nid].buffer_list.end(); ++z){
					flag1 = 0;
					for(y = nodes[id1-config->s_nid].buffer_list.begin(); y != nodes[id1-config->s_nid].buffer_list.end(); ++y){		
						if ((*z).id == (*y).id){
							flag1 = 1;	//this message has already been copied to id2's buffer
							break;		//we should break since this message (*z).id is in id2's buffer
						}
					}
					if(flag1 == 0 && (*z).gen_t <= current_time){
						//cout << "mesage id: " << (*z).id << " does not exist in " << id1 << "'s buffer" << endl;
						//T12 != null
						//we should copy the message from id1's buffer to id2's buffer
						//Message M is only copied once
						if((current_time - (*z).gen_t <= TTL) && ((*z).delivered == false) && ((*z).cost==0)){//check if msg has been expired
							//message has not been expired
							if(id2 != (*z).rid){//check if ID2 is the receiver of the message, if so then we dont need o copy the msg from ID1 to ID2!
							//core of routing scheme decision: Flooding
	
							//threshold
							if((closeness[id2-config->s_nid][(*z).rid-config->s_nid] < closeness[id1-config->s_nid][(*z).rid-config->s_nid] - thresh)){
	
									if (DEBUG) log << "message id " << (*z).id << " copied from " << id2 << " to " << id1 << " 's buffer at " << current_time << " closeness (sender, dst) " << closeness[id2-config->s_nid][(*z).rid-config->s_nid] << " closeness (receiver, dst) " << closeness[id1-config->s_nid][(*z).rid-config->s_nid]  << endl << endl;
								
									//if relay is closer to destination, copy the file
									msg.id = (*z).id;
									msg.sid = (*z).sid;;
									msg.rid = (*z).rid;
									msg.gen_t = (*z).gen_t;
									msg.no_hops = (*z).no_hops + 1;
									if(id1 == (*z).rid)	msg.delivered = true;//this is the destination of msg
									else msg.delivered = false;
									nodes[id1-config->s_nid].buffer_list.push_back(msg);	// copy the message to the end of the id2's buffer!
									if(id1 != (*z).rid)	(*z).cost++;	//since we have transmitted this message from id1 to id2
								}
								//let's check if ID2 is the msg.rcvr
								if(id1 == (*z).rid && (*z).gen_t <= stime){
									(*z).cost++;	//we have to count the last step delivery
									(*z).delivered = true;
									//Go to the corresponding entry of traffic profile and updates the attributes
									//id1 has a video for id2
									if (DEBUG) log << "node " << id1 << " is receiver" << endl;
									if (DEBUG) log << "message " << (*z).id << " generated at " << (*z).gen_t << " delivered at " << stime << " by " << id2 << endl;
									double delay = (double)(stime - (*z).gen_t)/(double)3600.0;
									
									if (DEBUG) log << "delivery delay: " << delay << endl;
									it1 = z;	//let's save the iterator to remove this buffer from id1's buffer
	
									//let's update the traffic profile table for delivered videos
									for(k = traf_list[(*z).sid-config->s_nid].begin(); k != traf_list[(*z).sid-config->s_nid].end(); ++k){ 
										if((*k).sndr_id == (*z).sid && (*k).rcvr_id == id1 && (*k).gen_t == (*z).gen_t){
											//successful delivery
											(*k).success = 1;	
											(*k).hops = (*z).no_hops + 1;	//one hop transmission	
											(*k).delay = stime - (*z).gen_t;//delay in sec
											(*k).rcv_t = stime;	//received time in sec
										}
											
										if (DEBUG) log << "2updating profile for node: " << (*k).sndr_id << " to node: " << (*k).rcvr_id << " in traffic profile table!" << endl; // print member
										//cout << endl;
									}
								}
							}//else since ID1 is the msg.rcvr --> NOP
						}
					}	
				}

			}//end for

			//let's virtually clean up all the timeout messages from nodes buffers
			int i;
			int current_time = ts + 2*TTL + 1; //update for solving the bug of msgs with uniform gen times
			for (i = config->s_nid; i <= config->e_nid; i++){
				//let's go through id1's buffer to find expired message, we should update the traffic profile and remove those message from the buffer
				timeout(i, current_time, log);
			}

			//let's store delay in delayGC.dat file: Greedy III Scheme
			ofstream delayGC( "/home/kazem/Desktop/traces/logs/delayGC.dat", ios::app );
				
			// exit program if unable to create file
				
			ofstream costGC( "/home/kazem/Desktop/traces/logs/costGC.dat", ios::app );
			
			// exit program if unable to create file
		
			if ( !delayGC || !costGC ) // overloaded ! operator
			{
				cerr << "File delayGC.dat or costGC.dat could not be opened" << endl;
				return -1;
			} // end if
			//write delay and cost
			write_profile(delayGC, costGC, time, 7);
			delayGC.close();
			costGC.close();
		        log << "-----------------------------------------------------------------------" << endl;
			log.close();
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

//this methods fills the social distance randomly for random scheme
//this method should be called before Random scheme
int routing::random_social_dist(){
	cout << "generating random social distances!" << endl;
	distr dist_(config);
	for (int i = config->s_nid; i <= config->e_nid; i++)
		for (int j = i; j <= config->e_nid; j++){
			//cout << "closeness of (" << u1 << "," << u2 << ") = " << dist_.closeness ( u1,u2 ) << endl;
			if(i==j)	closeness[i-config->s_nid][j-config->s_nid] = 1;
			else{
				double rand = get_rand();
				closeness[i-config->s_nid][j-config->s_nid] = rand;
				closeness[j-config->s_nid][i-config->s_nid] = rand;	//symmetric matrix
				//cout << "(" << i << "," << j << ") = " << closeness[i-config->s_nid][j-config->s_nid] << endl; 
			}
			//cout << closeness[i-config->s_nid][j-config->s_nid] << endl;
		}
	cout << "endinig pre-processing!" << endl;
	return 0;
}

//A random scheme in which every message will be forwarded only NF times
//hoff_prob = hand off probabilitay
int routing::Random(int time, double hoff_prob){
	list<traffic>::iterator k;	// an iterator for the link list
	
	message msg; 
	//let's store all interaction of networks in log.dat file
	ofstream log( "/home/kazem/Desktop/traces/logs/log.dat", ios::app );
	
	// exit program if unable to create file
	
	if ( !log ) // overloaded ! operator
	{
		cerr << "File log.dat could not be opened" << endl;
		return -1;
	}

	// Connect to database 
	mysqlpp::Connection conn(false);

	/* Connect to database */
	if (conn.connect(config->db, "localhost", "root", "mysql")) {
		// Retrieve a subset of the sample stock table set up by resetdb
		// and display it
		cout << "Connected to db!" << endl;
		
		char _query [500];
		bzero(_query, 500);
		//we should count all contacts: not only long ones!!!
		sprintf (_query, "select user1, user2, starttime, endtime from %s where user1 <= %d and user1 >= %d and user2 <= %d and user2 >= %d and starttime > %d and starttime < %d order by starttime", config->table, config->e_nid, config->s_nid, config->e_nid, config->s_nid, ts, ts + TTL);
		mysqlpp::Query query = conn.query(_query);

		if (mysqlpp::StoreQueryResult res = query.store()){
			//greedy scheme
			cout << "Communication using Random scheme for routing:" << endl;
//			double thresh = 0.1;	//threshod for routing
//			double thresh_b = thresh;	//backup threshold
	
			for (size_t i = 0; i < res.num_rows(); ++i) {
				//going through db and finding contact duration time for each contact and store them inside map table
				int stime = res[i][2];	//starting time
				int etime = res[i][3];	//end time
				int id1 = res[i][0];	//user1
				int id2 = res[i][1];	//user2
				int current_time = stime;	//emulation clock is updated
				//let's check if user1 or user2 has video for the other one at a time <= stime, therefore we have to check the buffers of two users
				list<message>::iterator y, z, it1, it2, it3,it4;	// an iterator for the buffer link list
				
				//let's go through id1's buffer to find expired message, we should update the traffic profile and remove those message from the buffer
				timeout(id1, id2, current_time, log);	//let's go through id1 and id2 buffers and remove all timeout messages
	
				
				//now let's compare T1 and T2 to see what should we flood
				//read id1's and id2's buffers to see which messages should be copied to the ther one's buffer
				int flag1 = 0;	// to see if we should erase anything from the buffer of id1  
				//let's go through id1.buffer to see if there is any msg to be copied from ID1--> ID2
				//We are looking for T12 = T1-T2
				for(z = nodes[id1-config->s_nid].buffer_list.begin(); z != nodes[id1-config->s_nid].buffer_list.end(); ++z){
					flag1 = 0;
	
					for(y = nodes[id2-config->s_nid].buffer_list.begin(); y != nodes[id2-config->s_nid].buffer_list.end(); ++y){		
						if ((*z).id == (*y).id){
							flag1 = 1;	//this message has already been found in id2's buffer
							break;
						}
					}
					if(flag1 == 0 && (*z).gen_t <= current_time){
						//cout << "mesage id: " << (*z).id << " does not exist in " << id2 << "'s buffer" << endl;
						//T12 != null
						//we should copy the message from id1's buffer to id2's buffer
						if(current_time - (*z).gen_t <= TTL){//check if msg has been expired
							//message has not been expired
							if(id1 != (*z).rid){//check if ID1 is the receiver of the message, if so then we dont need to copy the msg from ID1 to ID2!
							//core of routing scheme decision: Flooding
							double rand = 0;
							if(id2 == (*z).rid){	
								rand = 0.0;
//								thresh = 0.0;
							}
							else {
								rand = get_rand();	
//								thresh = thresh_b;
							}
							//threshold
//							if((closeness[id1-config->s_nid][(*z).rid-config->s_nid] < closeness[id2-config->s_nid][(*z).rid-config->s_nid] - thresh) && 
							if(((*z).delivered == false) && rand <= hoff_prob){
	
									if (DEBUG) log << "1message id: " << (*z).id << " is copied from: " << id1 << " to: " << id2 << "'s buffer at: " << current_time << endl << endl;
								
									//greedy: copy of the relay is closer to desination
									msg.id = (*z).id;
									msg.sid = (*z).sid;;
									msg.rid = (*z).rid;
									msg.gen_t = (*z).gen_t;
									msg.no_hops = (*z).no_hops + 1;
									if(id2 == (*z).rid)	msg.delivered = true;//this is the destination of msg
									else msg.delivered = false;
									nodes[id2-config->s_nid].buffer_list.push_back(msg);	// copy the message to the end of the id2's buffer!
									if(id2 != (*z).rid)	(*z).cost++;	//since we have transmitted this message from id1 to id2
								}
								//let's check if ID2 is the msg.rcvr
								if(id2 == (*z).rid && (*z).gen_t <= stime){
									(*z).cost++;	
									(*z).delivered = true;	//stop sending this data anymore
									//Go to the corresponding entry of traffic profile and updates the attributes
									//id1 has a video for id2
									if (DEBUG) log << "1node: " << id2 << " is the receiver" << endl;
									if (DEBUG) log << "message: " << (*z).id << " generated at: " << (*z).gen_t << ", delivered at: " << stime << "by: " << id1 << endl;
									
									double delay = (double)(stime - (*z).gen_t)/(double)3600.0;
									if (DEBUG) log << "delay of delivery: " << delay << endl;
									it1 = z;	//let's save the iterator to remove this buffer from id1's buffer???
	
									//let's update the traffic profile table for delivered videos
									for(k = traf_list[(*z).sid-config->s_nid].begin(); k != traf_list[(*z).sid-config->s_nid].end(); ++k){ 
										if((*k).sndr_id == (*z).sid && (*k).rcvr_id == id2 && (*k).gen_t == (*z).gen_t && (*k).id == (*z).id){
											//successful delivery
										(*k).success = 1;	
										(*k).hops = (*z).no_hops + 1;//one hop transmission	
										(*k).delay = stime - (*z).gen_t;//delay in sec
										(*k).rcv_t = stime;	//received time in sec
										}
											
										if (DEBUG) log << "1updating profile for node: " << (*k).sndr_id << " to node: " << (*k).rcvr_id << " in traffic profile table!" << endl; // print member
										//cout << endl;
									}
								}
							}//else since ID1 is the msg.rcvr --> NOP
						}
					}	
				}
	
	
				//read id2's and id1's buffers to see which messages should be copied to the ther one's buffer
				flag1 = 0;	// to see if we should erase anything from the buffer of id2  
				//let's go through id2.buffer to see if there is any msg to be copied from ID2--> ID1
				//We are looking for T21 = T2-T1
				for(z = nodes[id2-config->s_nid].buffer_list.begin(); z != nodes[id2-config->s_nid].buffer_list.end(); ++z){
					flag1 = 0;
					for(y = nodes[id1-config->s_nid].buffer_list.begin(); y != nodes[id1-config->s_nid].buffer_list.end(); ++y){		
						if ((*z).id == (*y).id){
							flag1 = 1;	//this message has already been copied to id2's buffer
							break;		//we should break since this message (*z).id is in id2's buffer
						}
					}
					if(flag1 == 0 && (*z).gen_t <= current_time){
						//cout << "mesage id: " << (*z).id << " does not exist in " << id1 << "'s buffer" << endl;
						//T12 != null
						//we should copy the message from id1's buffer to id2's buffer
						if(current_time - (*z).gen_t <= TTL){//check if msg has been expired
							//message has not been expired
							if(id2 != (*z).rid){//check if ID2 is the receiver of the message, if so then we dont need o copy the msg from ID2 to ID1!
							//core of routing scheme decision: Flooding
							double rand = 0;
							if(id1 == (*z).rid){	
								rand = 0.0;
//								thresh = 0.0;
							}
							else {
								rand = get_rand();	
//								thresh = thresh_b;
							}
							//threshold
//							if((closeness[id2-config->s_nid][(*z).rid-config->s_nid] < closeness[id1-config->s_nid][(*z).rid-config->s_nid] - thresh) && 
							if(((*z).delivered == false) && rand < hoff_prob){
	
									if (DEBUG) log << "2message id: " << (*z).id << " is copied from: " << id2 << " to: " << id1 << "'s buffer at: " << current_time << endl << endl;
								
									//if relay is closer to destination, copy the file
									msg.id = (*z).id;
									msg.sid = (*z).sid;;
									msg.rid = (*z).rid;
									msg.gen_t = (*z).gen_t;
									msg.no_hops = (*z).no_hops + 1;
									if(id1 == (*z).rid)	msg.delivered = true;//this is the destination of msg
									else msg.delivered = false;
									nodes[id1-config->s_nid].buffer_list.push_back(msg);	// copy the message to the end of the id2's buffer!
									if(id1 != (*z).rid) (*z).cost++;	//since we have transmitted this message from id1 to id2
								}
								//let's check if ID2 is the msg.rcvr
								if(id1 == (*z).rid && (*z).gen_t <= stime){
									(*z).cost++;
									(*z).delivered = true;
									//Go to the corresponding entry of traffic profile and updates the attributes
									//id1 has a video for id2
									if (DEBUG) log << "2node: " << id1 << " is the receiver" << endl;
									if (DEBUG) log << "2message: " << (*z).id << " generated at: " << (*z).gen_t << ", delivered at: " << stime << "by: " << id2 << endl;
									double delay = (double)(stime - (*z).gen_t)/(double)3600.0;
									
									if (DEBUG) log << "delivery delay: " << delay << endl;
									it1 = z;	//let's save the iterator to remove this buffer from id1's buffer
	
									//let's update the traffic profile table for delivered videos
									for(k = traf_list[(*z).sid-config->s_nid].begin(); k != traf_list[(*z).sid-config->s_nid].end(); ++k){ 
										if((*k).sndr_id == (*z).sid && (*k).rcvr_id == id1 && (*k).gen_t == (*z).gen_t && (*k).id == (*z).id){
											//successful delivery
											(*k).success = 1;	
											(*k).hops = (*z).no_hops + 1;	//one hop transmission	
											(*k).delay = stime - (*z).gen_t;//delay in sec
											(*k).rcv_t = stime;	//received time in sec
										}
											
										if (DEBUG) log << "2updating profile for node: " << (*k).sndr_id << " to node: " << (*k).rcvr_id << " in traffic profile table!" << endl; // print member
										//cout << endl;
									}
								}
							}//else since ID1 is the msg.rcvr --> NOP
						}
					}	
				}
			}//end for

			//let's virtually clean up all the timeout messages from nodes buffers
			int i;
			int current_time = ts + 2*TTL + 1; //update for solving the bug of msgs with uniform gen times
			for (i = config->s_nid; i <= config->e_nid; i++){
				//let's go through id1's buffer to find expired message, we should update the traffic profile and remove those message from the buffer
				timeout(i, current_time, log);
			}

			//let's store delay in delayR.dat file: Greedy I Scheme
			ofstream delayR( "/home/kazem/Desktop/traces/logs/delayR.dat", ios::app );
				
			// exit program if unable to create file
				
			ofstream costR( "/home/kazem/Desktop/traces/logs/costR.dat", ios::app );
			
			// exit program if unable to create file
		
			if ( !delayR || !costR ) // overloaded ! operator
			{
				cerr << "File delayR.dat or costR.dat could not be opened" << endl;
				return -1;
			} // end if
			//write delay and cost
			write_profile(delayR, costR, time, 8);
			delayR.close();
			costR.close();
		        log << "-----------------------------------------------------------------------" << endl;
			log.close();
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
/*
Combination of Global Rank and Secretary problem
RankSec forwarding method
arguments: time = scheme no, wLen= window length for counting unique encouners of each node, and uTime= update duration for calling uRank function, 
Here, we observe the first N/e=79/e=29 nodes (in this problem, N is known!) to see who is the best in terms of degree, then we forward the message to the node who has a better degree than the nodes we have visited so far!
Note, there is a chance that we visit the same node twice. So, this is not exactly the same as secretary problem.
Remember: we should first set ro.mcp = 1; and the call create_traffic()
*/
int routing::RankSec(int time, int wLen, int uTime, int stopT){//, int waitingT, double alpha, int strategy){
	list<traffic>::iterator k;	// an iterator for the link list
	map<int, int>::iterator ii;
	message msg;
	int r_id1 = 0;	//the rank of node id1
	int r_id2 = 0;	//rank of node id2
	bool l_id1D = false;	//is true if id1 has the same label (group) as the msg's dest
	bool l_id2D = false;	//is true if id2 has the same label (group) as the msg's dest
	int lUpdate = 0;	//this is for ranking updates
	mcp = 1;	//no of copies per message
	//let's store all interaction of networks in log.dat file
	ofstream log( "/home/kazem/Desktop/traces/logs/log.dat", ios::app );
	
	// exit program if unable to create file
	if ( !log ) // overloaded ! operator
	{
		cerr << "File log.dat could not be opened" << endl;
		return -1;
	}


	// Connect to database 
	mysqlpp::Connection conn(false);

	/* Connect to database */
	if (conn.connect(config->db, "localhost", "root", "mysql")) {
		// Retrieve a subset of the sample stock table set up by resetdb
		// and display it
		cout << "Connected to db!" << endl;
		// and display it
		char _query [500];
		bzero(_query, 500);
		//we should count all contacts: not only long ones!!!
		sprintf (_query, "select user1, user2, starttime, endtime from %s where user1 <= %d and user1 >= %d and user2 <= %d and user2 >= %d and starttime > %d and starttime < %d order by starttime", config->table, config->e_nid, config->s_nid, config->e_nid, config->s_nid, ts, ts + TTL);

		mysqlpp::Query query = conn.query(_query);

		if (mysqlpp::StoreQueryResult res = query.store()){
			//BUBBLE SCHEME
			cout << "Communication using Rank Secretary scheme for routing:" << endl;
			for (size_t i = 0; i < res.num_rows(); ++i){
				//going through db and finding contact duration time for each contact and store them inside map table
				int stime = res[i][2];	//starting time
				int etime = res[i][3];	//end time
				int id1 = res[i][0];	//user1
				int id2 = res[i][1];	//user2
				int current_time = stime;	//emulation clock is updated
				//let's check if user1 or user2 has video for the other one at a time <= stime, therefore we have to check the buffers of two users
				list<message>::iterator y, z, it1, it2, it3,it4;	// an iterator for the buffer link list

				if((lUpdate == 0 || (current_time - lUpdate) > uTime) && ((current_time - ts) <= TTL)){
					//we have to update the rank list for all users
					cg.updateRanks(ts, current_time, wLen, 0);
					lUpdate = current_time;		//update the last update pointer for rank lists
				}

				//there is a contact between id1 and id2
				r_id1 = cg.rank(id1);
				r_id2 = cg.rank(id2);
				
				//we keep updating the list of observed nodes to calculate the actual max of the list and compare it with the candidate we have chosen for passing over the message
				if(1){
					//we need to log the actual max of the observed node
					if (r_id1 > nodes[id2-config->s_nid].maxSeq)	nodes[id2-config->s_nid].maxSeq = r_id1;

					//node id2 is in observation state
					ii = nodes[id2-config->s_nid].nodesMet.find(id1);
	
					if((*ii).first != id1 && r_id1 != 0){
//						cout << '\t' << "met node: " << id1 << endl;
						nodes[id2-config->s_nid].nodesMet.insert(std::make_pair(id1, r_id1));
						//let's calculate the most popular node which has been met so far if the time is less than optimal stopping time
						if ((nodes[id2-config->s_nid].nodesMet.size() < stopT) && (r_id1 > nodes[id2-config->s_nid].maxRank))	nodes[id2-config->s_nid].maxRank = r_id1;
					}
				}
				//keep storing the qualities of all met nodes
				if(1){
					//we need to log the actual max of the observed node
					if (r_id2 > nodes[id1-config->s_nid].maxSeq)	nodes[id1-config->s_nid].maxSeq = r_id2;

					//node id1 is in observation state
					ii = nodes[id1-config->s_nid].nodesMet.find(id2);
	
					if(((*ii).first != id2) &&(r_id2 != 0)){
//						cout << '\t' << "met node: " << id2 << endl;
						nodes[id1-config->s_nid].nodesMet.insert(std::make_pair(id2, r_id2));
						//we only update the MAX threshold if we are below stopT
						if ((nodes[id1-config->s_nid].nodesMet.size() < stopT) && (r_id2 > nodes[id1-config->s_nid].maxRank))	nodes[id1-config->s_nid].maxRank = r_id2;
					}

				}
				//let's log all nodes that 50 met after 1:00 pm on Tuesday
/*				if (id1 == 50 && stime > 57600+4*3600 && (nodes[id1-config->s_nid].nodesMet.size() < stopT)){
					cout << "no of observation till " << (double)stime/3600.0 << ": " << nodes[id1-config->s_nid].nodesMet.size() << " & max: " << nodes[id1-config->s_nid].maxRank << endl;
					
					for ( ii = nodes[id1-config->s_nid].nodesMet.begin() ; ii != nodes[id1-config->s_nid].nodesMet.end(); ii++ )
    						cout << (*ii).second << " ";
					cout << endl;
				}*/

				//let's go through id1's buffer to find expired messages, we should update the traffic profile and remove those message from the buffer
				timeout(id1, id2, current_time, log);	//let's go through id1 and id2 buffers and remove all timeout messages


				//now let's compare T1 and T2 to see what should we copy
				//read id1's and id2's buffers to see which messages should be copied to the ther one's buffer
				int flag1 = 0;	// to see if we should erase anything from the buffer of id1  
				//let's go through id1.buffer to see if there is any msg to be copied from ID1--> ID2
				//We are looking for T12 = T1-T2
							//let's check if id2 has higher rank the best node which id1 has met so far
				for(z = nodes[id1-config->s_nid].buffer_list.begin(); z != nodes[id1-config->s_nid].buffer_list.end(); ++z){//#for #1 
					flag1 = 0;

					for(y = nodes[id2-config->s_nid].buffer_list.begin(); y != nodes[id2-config->s_nid].buffer_list.end(); ++y){		
						if ((*z).id == (*y).id){
							flag1 = 1;	//this message has already been found in id2's buffer
							break;
						}
					}
					if(flag1 == 0 && (*z).gen_t <= current_time){//if #1
						//cout << "mesage id: " << (*z).id << " does not exist in " << id2 << "'s buffer" << endl;
						//T12 != null
						//we should copy the message from id1's buffer to id2's buffer
						if((current_time - (*z).gen_t <= TTL)  && ((*z).delivered == false)){//if #2

							//check if msg has been expired
							//message has not been expired
							if(id1 != (*z).rid){//(if #3)check if ID1 is the receiver of the message, if so then we dont need o copy the msg from ID1 to ID2!
				//we start routing after optimal stopping time
				if(((r_id2 >= nodes[id1-config->s_nid].maxRank) && (nodes[id1-config->s_nid].nodesMet.size() >= stopT) && ((*z).mcp > 0)) || (id2 == (*z).rid && (*z).gen_t <= stime)){

//let's log the chosen candidate and compare it with max of observed nodes to see how eefective it is
if((r_id2 >= nodes[id1-config->s_nid].maxRank) && ((*z).mcp > 0)){
	nodes[id1-config->s_nid].candN = r_id2;
}
								if (DEBUG) log << "MSG(" << (*z).id << ") copied " << id1 << "-->" << id2 << " @ " << current_time << ", rank(s)=" << r_id1 << ", max Rank(s)=" << nodes[id1-config->s_nid].maxRank << ", relay's rank=" << r_id2 << endl << endl;
								(*z).mcp--;	//no copies anymore
								//greedy: copy of the relay is closer to desination
								msg.id = (*z).id;
								msg.sid = (*z).sid;;
								msg.rid = (*z).rid;
								msg.gen_t = (*z).gen_t;
								msg.no_hops = (*z).no_hops + 1;
								if(id2 == (*z).rid)	msg.delivered = true;//this is the destination of msg
								else msg.delivered = false;
								(*z).cost++;	//since we have transmitted this message from id1 to id2

								msg.mcp = (*z).mcp;
								nodes[id2-config->s_nid].buffer_list.push_back(msg);	// copy the message to the end of the id2's buffer!				
								}
								//let's check if ID2 is the msg.rcvr
								if(id2 == (*z).rid && (*z).gen_t <= stime){//if #4
									(*z).delivered = true;
									//Go to the corresponding entry of traffic profile and updates the attributes
									//id1 has a video for id2
									if (DEBUG) log << "Node " << id2 << " is Rcvr" << endl;
									if (DEBUG) log << "MSG: " << (*z).id << ", gen @: " << (*z).gen_t << ", delivered  @: " << stime << ", by: " << id1 << endl;

									//let's see if the msg has been delivered by a relay node
									if(id1 != (*z).sid)	relayed_msgs++;
									
									double delay = (double)(stime - (*z).gen_t)/(double)3600.0;
									if (DEBUG) log << "delivery delay: " << delay << endl;
									it1 = z;	//let's save the iterator to remove this buffer from id1's buffer???
	
									//let's update the traffic profile table for delivered videos
									for(k = traf_list[(*z).sid-config->s_nid].begin(); k != traf_list[(*z).sid-config->s_nid].end(); ++k){ 
										if((*k).sndr_id == (*z).sid && (*k).rcvr_id == id2 && (*k).gen_t == (*z).gen_t){
											//successful delivery
											(*k).success = 1;	
											(*k).hops = (*z).no_hops + 1;	//one hop transmission	
											(*k).delay = stime - (*z).gen_t;//delay in sec
											(*k).rcv_t = stime;	//received time in sec
										}
											
										if (DEBUG) log << "updating prof of node: " << (*k).sndr_id << " to node: " << (*k).rcvr_id << " in traffic profile table!" << endl; // print member
										//cout << endl;
									}//end for #2
								}//end if #4
							}//if #3 since ID1 is the msg.rcvr --> NOP//end if #3
						}//end if #2
					}//end if #1	
				}//for #1

				//read id2's and id1's buffers to see which messages should be copied to the ther one's buffer
				flag1 = 0;	// to see if we should erase anything from the buffer of id2  
				//let's go through id2.buffer to see if there is any msg to be copied from ID2--> ID1
				//We are looking for T21 = T2-T1
				for(z = nodes[id2-config->s_nid].buffer_list.begin(); z != nodes[id2-config->s_nid].buffer_list.end(); ++z){//for #1
					flag1 = 0;
					for(y = nodes[id1-config->s_nid].buffer_list.begin(); y != nodes[id1-config->s_nid].buffer_list.end(); ++y){		
						if ((*z).id == (*y).id){
							flag1 = 1;	//this message has already been copied to id2's buffer
							break;		//we should break since this message (*z).id is in id2's buffer
						}
					}

					if(flag1 == 0 && (*z).gen_t <= current_time){//if #1
						//cout << "mesage id: " << (*z).id << " does not exist in " << id1 << "'s buffer" << endl;
						//T12 != null
						//we should copy the message from id2's buffer to id1's buffer
						if((current_time - (*z).gen_t <= TTL) && ((*z).delivered == false)){//if #2
							//check if msg has been expired
							//message has not been expired
							if(id2 != (*z).rid){//if # 3: check if ID2 is the receiver of the message, if so then we dont need o copy the msg from ID1 to ID2!
							//core of routing scheme decision: Flooding
							//Bubble: forward if the id2 has the same label as the destination or higher rank!
								//if the current node has the same label as dest, we will forward the message only if id1 has the same label as the dest and higher local rank than id2 as well
							//let's check if id1 has a higher rank than the best node which id2 has met so far
				//we start routing after optimal stopping time
				if(((r_id1 >= nodes[id2-config->s_nid].maxRank) && (nodes[id2-config->s_nid].nodesMet.size() >= stopT) && ((*z).mcp > 0)) || (id1 == (*z).rid && (*z).gen_t <= stime)){

//let's log the chosen candidate and compare it with max of observed nodes to see how eefective it is
if((r_id1 >= nodes[id2-config->s_nid].maxRank) && ((*z).mcp > 0)){
	nodes[id2-config->s_nid].candN = r_id1;
}
									if (DEBUG) log << "MSG (" << (*z).id << ") copied " << id2 << "-->" << id1 << " @ " << current_time << ", rank(s)=" << r_id2 << ", max Rank(s)=" << nodes[id2-config->s_nid].maxRank << ", relay's rank=" << r_id1 << endl << endl;
									(*z).mcp--;
									//greedy: copy of the relay is closer to desination
									msg.id = (*z).id;
									msg.sid = (*z).sid;;
									msg.rid = (*z).rid;
									msg.gen_t = (*z).gen_t;
									msg.no_hops = (*z).no_hops + 1;
									msg.mcp = (*z).mcp;
									(*z).cost++;	//since we have transmitted this message from id2 to id1
									if(id1 == (*z).rid)	msg.delivered = true;//this is the destination of msg
									else msg.delivered = false;
									
									nodes[id1-config->s_nid].buffer_list.push_back(msg);	// copy the message to the end of the id1's buffer!
								}
								//let's check if ID1 is the msg.rcvr
								if(id1 == (*z).rid && (*z).gen_t <= stime){//if #4
									(*z).delivered = true;	//stop sending this msg anymore
									//Go to the corresponding entry of traffic profile and updates the attributes
									//id1 has a video for id2
									if (DEBUG) log << "Node: " << id1 << " is Rcvr" << endl;
									if (DEBUG) log << "MSG: " << (*z).id << " gen @ " << (*z).gen_t << ", delivered  @: " << stime << ", by: " << id2 << endl;

									//let's see if the msg has been delivered by a relay node
									if(id2 != (*z).sid)	relayed_msgs++;

									double delay = (double)(stime - (*z).gen_t)/(double)3600.0;
									
									if (DEBUG) log << "delivery delay: " << delay << endl;
									it1 = z;	//let's save the iterator to remove this buffer from id1's buffer
	
									//let's update the traffic profile table for delivered videos
									for(k = traf_list[(*z).sid-config->s_nid].begin(); k != traf_list[(*z).sid-config->s_nid].end(); ++k){ 
										if((*k).sndr_id == (*z).sid && (*k).rcvr_id == id1 && (*k).gen_t == (*z).gen_t){
											//successful delivery
											(*k).success = 1;	
											(*k).hops = (*z).no_hops + 1;	//one hop transmission	
											(*k).delay = stime - (*z).gen_t;//delay in sec
											(*k).rcv_t = stime;	//received time in sec
										}
											
										if (DEBUG) log << "updating prof of node: " << (*k).sndr_id << " to node: " << (*k).rcvr_id << " in traffic profile table!" << endl; // print member
										//cout << endl;
									}//end for #2
								}//end if #4
							}//end if #3 since ID1 is the msg.rcvr --> NOP
						}//end if #2
					}//end if#1
				}//end for #1
//				}//if for copy the message

			}//end for to go throug all returned value from mysql
			//let's virtually clean up all the timeput messages from nodes buffers
			int i;
			int current_time = ts + 2*TTL + 1; //update for solving the bug of msgs with uniform gen times
			for (i = config->s_nid; i <= config->e_nid; i++){
				//let's go through id1's buffer to find expired message, we should update the traffic profile and remove those message from the buffer
				timeout(i, current_time, log);
			}

			//we have to clean up the whole observed nodes map for all nodes
			for (i = config->s_nid; i <= config->e_nid; i++)	nodes[i - config->s_nid].nodesMet.clear();

			//let's print all the chosen candidates and compare them with the actual max of the list
			for (i = config->s_nid; i <= config->e_nid; i++)
				if(nodes[i-config->s_nid].candN > 0){
					cout << nodes[i-config->s_nid].candN << " " << nodes[i-config->s_nid].maxSeq << ";";
				}

			//let's store delay in delayRS.dat file: RsnkSec Scheme
			ofstream delayRS( "/home/kazem/Desktop/traces/logs/delayRS.dat", ios::app );
				
			// exit program if unable to create file
				
			ofstream costRS( "/home/kazem/Desktop/traces/logs/costRS.dat", ios::app );
			
			// exit program if unable to create file
			
			if ( !delayRS || !costRS ) // overloaded ! operator
			{
			cerr << "File delayRS.dat or costRS.dat could not be opened" << endl;
			return -1;
			} // end if

			cout << "no of messgae routed by relay nodes: " << relayed_msgs << endl;

			write_profile(delayRS, costRS, time, 9);
			delayRS.close();
			costRS.close();
			log << "-----------------------------------------------------------------------" << endl;
			log.close();
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

/*
Delegation forwarding which is similar to SocialGreedy II
quality metric: rank
arguments: time = scheme no, wLen= window length for counting unique encouners of each node, and uTime= update duration for calling uRank function, 
Note, there is a chance that we visit the same node twice. So, this is not exactly the same as secretary problem.
routing metric: 0 --> centrality, 1 --> total contact no (which we assume that we know in advance)
*/
int routing::Delegation(int time, int wLen, int uTime, int metric){
	list<traffic>::iterator k;	// an iterator for the link list
	message msg;
	int r_id1 = 0;	//the rank of node id1
	int r_id2 = 0;	//rank of node id2
	bool l_id1D = false;	//is true if id1 has the same label (group) as the msg's dest
	bool l_id2D = false;	//is true if id2 has the same label (group) as the msg's dest
	bool rMode = false;	//routing mode which is based on the conference schedule
	int lUpdate = 0;	//this is for ranking updates
	//let's store all interaction of networks in log.dat file

	if(metric){
		//we have to collect the total no of contacts in advance
		distr dist(config);
		for (int i=21; i <=99; i++){
			char id1[ 10 ];
			bzero(id1, 10);
			sprintf(id1,"%d", i); 
			//now we are observing the no of contacts of every node from ts to ts+TTL
			int no_cont = dist.no_of_contacts(id1,ts, ts + TTL, 0);
			nodes[i-config->s_nid].contact_no = no_cont;	//save it
		}
	}

	ofstream log( "/home/kazem/Desktop/traces/logs/log.dat", ios::app );
	
	// exit program if unable to create file
	if ( !log ) // overloaded ! operator
	{
		cerr << "File log.dat could not be opened" << endl;
		return -1;
	}

	// Connect to database 
	mysqlpp::Connection conn(false);

	/* Connect to database */
	if (conn.connect(config->db, "localhost", "root", "mysql")) {
		// Retrieve a subset of the sample stock table set up by resetdb
		// and display it
		cout << "Connected to db!" << endl;
		// and display it
		char _query [500];
		bzero(_query, 500);
		//we should count all contacts: not only long ones!!!
		sprintf (_query, "select user1, user2, starttime, endtime from %s where user1 <= %d and user1 >= %d and user2 <= %d and user2 >= %d and starttime > %d and starttime < %d order by starttime", config->table, config->e_nid, config->s_nid, config->e_nid, config->s_nid, ts, ts + TTL);

//		cout << "query: " << _query << endl;

		mysqlpp::Query query = conn.query(_query);
		if (mysqlpp::StoreQueryResult res = query.store()){
			//BUBBLE SCHEME
			cout << "Communication using Delegation scheme for routing:" << endl;
			for (size_t i = 0; i < res.num_rows(); ++i){
				//going through db and finding contact duration time for each contact and store them inside map table
				int stime = res[i][2];	//starting time
				int etime = res[i][3];	//end time
				int id1 = res[i][0];	//user1
				int id2 = res[i][1];	//user2
				int current_time = stime;	//emulation clock is updated
				//let's check if user1 or user2 has video for the other one at a time <= stime, therefore we have to check the buffers of two users
				list<message>::iterator y, z, it1, it2, it3,it4;	// an iterator for the buffer link list
				
				if(metric){
					//there is a contact between id1 and id2
					//metric = total no of contacts
					r_id1 = nodes[id1-config->s_nid].contact_no;
					r_id2 = nodes[id2-config->s_nid].contact_no;
					//cout << id1 << " " << r_id1 << ", " << id2 << " " << r_id2 << endl;
				}else{
					//we are intersted in contact rates per unit time (total no of contacts for the last wlen)
					if((lUpdate == 0 || (current_time - lUpdate) > uTime) && ((current_time - ts) <= TTL)){
						//we have to update the rank list for all users
						cg.updateRanks(ts, current_time, wLen, 0);
						lUpdate = current_time;		//update the last update pointer for rank lists
					}
					//there is a contact between id1 and id2
					r_id1 = cg.rank(id1);
					r_id2 = cg.rank(id2);
				}
				//schedule-aware routing, TTL=9 hours
			
				//let's go through id1's buffer to find expired messages, we should update the traffic profile and remove those message from the buffer
				timeout(id1, id2, current_time, log);	//let's go through id1 and id2 buffers and remove all timeout messages

				//now let's compare T1 and T2 to see what should we flood
				//read id1's and id2's buffers to see which messages should be copied to the ther one's buffer
				int flag1 = 0;	// to see if we should erase anything from the buffer of id1  
				//let's go through id1.buffer to see if there is any msg to be copied from ID1--> ID2
				//We are looking for T12 = T1-T2
				for(z = nodes[id1-config->s_nid].buffer_list.begin(); z != nodes[id1-config->s_nid].buffer_list.end(); ++z){
					flag1 = 0;
	
					for(y = nodes[id2-config->s_nid].buffer_list.begin(); y != nodes[id2-config->s_nid].buffer_list.end(); ++y){		
						if ((*z).id == (*y).id){
							flag1 = 1;	//this message has already been found in id2's buffer
							break;
						}
					}
					if(flag1 == 0 && (*z).gen_t <= current_time){
						//cout << "mesage id: " << (*z).id << " does not exist in " << id2 << "'s buffer" << endl;
						//T12 != null
						//we should copy the message from id1's buffer to id2's buffer
						if((current_time - (*z).gen_t <= TTL) && ((*z).delivered == false)){//check if msg has been expired
							//message has not been expired
							if(id1 != (*z).rid){//check if ID1 is the receiver of the message, if so then we dont need o copy the msg from ID1 to ID2!
							//core of routing scheme decision: Flooding
	
							//threshold
							if((r_id1 < r_id2) && ((*z).maxRank < r_id2)){
	
									if (DEBUG) log << "message id " << (*z).id << " copied from " << id1 << " to " << id2 << " 's buffer at " << current_time << " rank(s) " << r_id1 << " rank(r) " << r_id2  << " M's max rank " << (*z).maxRank << endl << endl;
								
								
									//greedy: copy of the relay is closer to desination
									msg.id = (*z).id;
									msg.sid = (*z).sid;;
									msg.rid = (*z).rid;
									msg.gen_t = (*z).gen_t;
									msg.no_hops = (*z).no_hops + 1;
									msg.maxRank = 0;
									if(id2 == (*z).rid)	msg.delivered = true;//this is the destination of msg
									else msg.delivered = false;

									nodes[id2-config->s_nid].buffer_list.push_back(msg);	// copy the message to the end of the id2's buffer!
									if(id2 != (*z).rid)	(*z).cost++;	//since we have transmitted this message from id1 to id2
									(*z).maxRank = r_id2;	//this is the last node with the maximum similarity to dest!
								}
								//let's check if ID2 is the msg.rcvr
								if(id2 == (*z).rid && (*z).gen_t <= stime){
									(*z).cost++;	//we have to count the last step delivery
									(*z).delivered = true;	//stop sending anymore
									//Go to the corresponding entry of traffic profile and updates the attributes
									//id1 has a video for id2
									if (DEBUG) log << "node: " << id2 << " is receiver" << endl;
									if (DEBUG) log << "message " << (*z).id << " generated at " << (*z).gen_t << " delivered at " << stime << " by " << id1 << endl;
									
									double delay = (double)(stime - (*z).gen_t)/(double)3600.0;
									if (DEBUG) log << "delay of delivery: " << delay << endl;
									it1 = z;	//let's save the iterator to remove this buffer from id1's buffer???
	
									//let's update the traffic profile table for delivered videos
									for(k = traf_list[(*z).sid-config->s_nid].begin(); k != traf_list[(*z).sid-config->s_nid].end(); ++k){ 
										if((*k).sndr_id == (*z).sid && (*k).rcvr_id == id2 && (*k).gen_t == (*z).gen_t){
											//successful delivery
											(*k).success = 1;	
											(*k).hops = (*z).no_hops + 1;	//one hop transmission	
											(*k).delay = stime - (*z).gen_t;//delay in sec
											(*k).rcv_t = stime;	//received time in sec
										}
											
										if (DEBUG) log << "1updating profile for node: " << (*k).sndr_id << " to node: " << (*k).rcvr_id << " in traffic profile table!" << endl; // print member
										//cout << endl;
									}
								}
							}//else since ID1 is the msg.rcvr --> NOP
						}
					}	
				}
	
	
				//read id2's and id1's buffers to see which messages should be copied to the ther one's buffer
				flag1 = 0;	// to see if we should erase anything from the buffer of id2  
				//let's go through id2.buffer to see if there is any msg to be copied from ID2--> ID1
				//We are looking for T21 = T2-T1
				for(z = nodes[id2-config->s_nid].buffer_list.begin(); z != nodes[id2-config->s_nid].buffer_list.end(); ++z){
					flag1 = 0;
					for(y = nodes[id1-config->s_nid].buffer_list.begin(); y != nodes[id1-config->s_nid].buffer_list.end(); ++y){		
						if ((*z).id == (*y).id){
							flag1 = 1;	//this message has already been copied to id2's buffer
							break;		//we should break since this message (*z).id is in id2's buffer
						}
					}
					if(flag1 == 0 && (*z).gen_t <= current_time){
						//cout << "mesage id: " << (*z).id << " does not exist in " << id1 << "'s buffer" << endl;
						//T12 != null
						//we should copy the message from id1's buffer to id2's buffer
						if((current_time - (*z).gen_t <= TTL) && ((*z).delivered == false)){//check if msg has been expired
							//message has not been expired
							if(id2 != (*z).rid){//check if ID2 is the receiver of the message, if so then we dont need o copy the msg from ID1 to ID2!
							//core of routing scheme decision: Flooding

							//threshold
							if((r_id2 < r_id1) && ((*z).maxRank < r_id1)){
	
									if (DEBUG) log << "message id " << (*z).id << " copied from " << id2 << " to " << id1 << " 's buffer at " << current_time << " rank(s) " << r_id2 << " rank(r) " << r_id1  << " M's max rank " << (*z).maxRank << endl << endl;

									//if relay is closer to destination, copy the file
									msg.id = (*z).id;
									msg.sid = (*z).sid;;
									msg.rid = (*z).rid;
									msg.gen_t = (*z).gen_t;
									msg.no_hops = (*z).no_hops + 1;
									msg.maxRank = 0;
									if(id1 == (*z).rid)	msg.delivered = true;//this is the destination of msg
									else msg.delivered = false;
									nodes[id1-config->s_nid].buffer_list.push_back(msg);	// copy the message to the end of the id2's buffer!
									if(id1 != (*z).rid)	(*z).cost++;	//since we have transmitted this message from id1 to id2
									(*z).maxRank = r_id1;	//this is the last node with the maximum similarity to dest!
								}
								//let's check if ID1 is the msg.rcvr
								if(id1 == (*z).rid && (*z).gen_t <= stime){
									(*z).cost++;	//we have to count the last step delivery
									(*z).delivered = true;	//stop sending this message anymore
									//Go to the corresponding entry of traffic profile and updates the attributes
									//id1 has a video for id2
									if (DEBUG) log << "node " << id1 << " is receiver" << endl;
									if (DEBUG) log << "message " << (*z).id << " generated at " << (*z).gen_t << " delivered at " << stime << " by " << id2 << endl;
									double delay = (double)(stime - (*z).gen_t)/(double)3600.0;
									
									if (DEBUG) log << "delivery delay: " << delay << endl;
									it1 = z;	//let's save the iterator to remove this buffer from id1's buffer
	
									//let's update the traffic profile table for delivered videos
									for(k = traf_list[(*z).sid-config->s_nid].begin(); k != traf_list[(*z).sid-config->s_nid].end(); ++k){ 
										if((*k).sndr_id == (*z).sid && (*k).rcvr_id == id1 && (*k).gen_t == (*z).gen_t){
											//successful delivery
											(*k).success = 1;	
											(*k).hops = (*z).no_hops + 1;	//one hop transmission	
											(*k).delay = stime - (*z).gen_t;//delay in sec
											(*k).rcv_t = stime;	//received time in sec
										}
											
										if (DEBUG) log << "2updating profile for node: " << (*k).sndr_id << " to node: " << (*k).rcvr_id << " in traffic profile table!" << endl; // print member
										//cout << endl;
									}
								}
							}//else since ID1 is the msg.rcvr --> NOP
						}
					}	
				}				


			}//end for to go throug all returned value from mysql
			//let's virtually clean up all the timeput messages from nodes buffers
			int i;
			int current_time = ts + 2*TTL + 1; //update for solving the bug of msgs with uniform gen times
			for (i = config->s_nid; i <= config->e_nid; i++){
				//let's go through id1's buffer to find expired message, we should update the traffic profile and remove those message from the buffer
				timeout(i, current_time, log);	//let's go through id1 and id2 buffers and remove all timeout messages
			}


			//let's store delay in delayDelg.dat file: RsnkSec Scheme
			ofstream delayD( "/home/kazem/Desktop/traces/logs/delayDelg.dat", ios::app );
				
			// exit program if unable to create file
				
			ofstream costD( "/home/kazem/Desktop/traces/logs/costDelg.dat", ios::app );
			
			// exit program if unable to create file
			
			if ( !delayD || !costD ) // overloaded ! operator
			{
			cerr << "File delayDelg.dat or costDelg.dat could not be opened" << endl;
			return -1;
			} // end if
			
			write_profile(delayD, costD, time, 10);
			delayD.close();
			costD.close();
			log << "------------------------------------end end end-----------------------------------" << endl;
			log.close();
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

/*
New Delegation which is similar to SocialGreedy III
quality metric: rank
arguments: time = scheme no, wLen= window length for counting unique encouners of each node, and uTime= update duration for calling uRank function, 
Note, there is a chance that we visit the same node twice. So, this is not exactly the same as secretary problem.
the last arg is routing metric: 0 --> centrality, 1 --> total contact no (which we assume that we know in advance)
*/
int routing::nDelegation(int time, int wLen, int uTime, int metric, double thresh){
	list<traffic>::iterator k;	// an iterator for the link list
	message msg;
	double util_id1 = 0;	//id1's utility
	double util_id2 = 0;	//id2's utility
	int r_id1 = 0;		//id1's rank
	int r_id2 = 0;		//id2's rank
	bool l_id1D = false;	//is true if id1 has the same label (group) as the msg's dest
	bool l_id2D = false;	//is true if id2 has the same label (group) as the msg's dest
	bool rMode = false;	//routing mode which is based on the conference schedule
	int lUpdate = 0;	//this is for ranking updates
	int mode = 0;	//0:centrality and 1:similarity

	if(metric){
		//we have to collect the total no of contacts in advance
		distr dist(config);
		for (int i=21; i <=99; i++){
			char id1[ 10 ];
			bzero(id1, 10);
			sprintf(id1,"%d", i); 
			//now we are observing the no of contacts of every node from ts to ts+TTL
			int no_cont = dist.no_of_contacts(id1,ts, ts + TTL, 0);
			nodes[i-config->s_nid].contact_no = no_cont;	//save it
		}
	}

	//let's store all interaction of networks in log.dat file
	ofstream log( "/home/kazem/Desktop/traces/logs/log.dat", ios::app );
	
	// exit program if unable to create file
	if ( !log ) // overloaded ! operator
	{
		cerr << "File log.dat could not be opened" << endl;
		return -1;
	}


	// Connect to database 
	mysqlpp::Connection conn(false);

	/* Connect to database */
	if (conn.connect(config->db, "localhost", "root", "mysql")) {
		// Retrieve a subset of the sample stock table set up by resetdb
		// and display it
		cout << "Connected to db!" << endl;
		// and display it
		char _query [500];
		bzero(_query, 500);
		//we should count all contacts: not only long ones!!!
		sprintf (_query, "select user1, user2, starttime, endtime from %s where user1 <= %d and user1 >= %d and user2 <= %d and user2 >= %d and starttime > %d and starttime < %d order by starttime", config->table, config->e_nid, config->s_nid, config->e_nid, config->s_nid, ts, ts + TTL);

//		cout << "query: " << _query << endl;

		mysqlpp::Query query = conn.query(_query);
		if (mysqlpp::StoreQueryResult res = query.store()){
			//BUBBLE SCHEME
			cout << "Communication using nDelegation scheme for routing:" << endl;
			for (size_t i = 0; i < res.num_rows(); ++i){
				//going through db and finding contact duration time for each contact and store them inside map table
				int stime = res[i][2];	//starting time
				int etime = res[i][3];	//end time
				int id1 = res[i][0];	//user1
				int id2 = res[i][1];	//user2
				int current_time = stime;	//emulation clock is updated
				//let's check if user1 or user2 has video for the other one at a time <= stime, therefore we have to check the buffers of two users
				list<message>::iterator y, z, it1, it2, it3,it4;	// an iterator for the buffer link list

				if(metric == 1){
					//there is a contact between id1 and id2
					//metric = total no of contacts
					util_id1 = (double) nodes[id1-config->s_nid].contact_no;
					util_id2 = (double) nodes[id2-config->s_nid].contact_no;
					//cout << id1 << " " << r_id1 << ", " << id2 << " " << r_id2 << endl;
				}else{
					//we are intersted in contact rates per unit time (total no of contacts for the last wlen)
					if((lUpdate == 0 || (current_time - lUpdate) > uTime) && ((current_time - ts) <= TTL)){
						//we have to update the rank list for all users
						cg.updateRanks(ts, current_time, wLen, 0);
						lUpdate = current_time;		//update the last update pointer for rank lists
					}
					//there is a contact between id1 and id2
					r_id1 = cg.rank(id1);
					r_id2 = cg.rank(id2);
				}
								
				//schedule-aware routing, TTL=9 hours
			
				//let's go through id1's buffer to find expired messages, we should update the traffic profile and remove those message from the buffer
				timeout(id1, id2, current_time, log);	//let's go through id1 and id2 buffers and remove all timeout messages

				
				//now let's compare T1 and T2 to see what should we flood
				//read id1's and id2's buffers to see which messages should be copied to the ther one's buffer
				int flag1 = 0;	// to see if we should erase anything from the buffer of id1  
				//let's go through id1.buffer to see if there is any msg to be copied from ID1--> ID2
				//We are looking for T12 = T1-T2
				for(z =  nodes[id1-config->s_nid].buffer_list.begin(); z != nodes[id1-config->s_nid].buffer_list.end(); ++z){
					flag1 = 0;
	
					for(y = nodes[id2-config->s_nid].buffer_list.begin(); y != nodes[id2-config->s_nid].buffer_list.end(); ++y){		
						if ((*z).id == (*y).id){
							flag1 = 1;	//this message has already been found in id2's buffer
							break;
						}
					}
					if(flag1 == 0 && (*z).gen_t <= current_time){
						//cout << "mesage id: " << (*z).id << " does not exist in " << id2 << "'s buffer" << endl;
						//T12 != null
						//we should copy the message from id1's buffer to id2's buffer
						//A message M only forwards once
						if((current_time - (*z).gen_t <= TTL) && ((*z).delivered == false) && ((*z).cost==0)){//check if msg has been expired
							//message has not been expired
							if(id1 != (*z).rid){//check if ID1 is the receiver of the message, if so then we dont need to copy the msg from ID1 to ID2!
							//core of routing scheme decision: Flooding
				if(metric == 2){
					//let's find utilities for id1 and id2 to contact the target. this utility incorporates both similarity and centrality of  nodes
					util_id1 = 1.0 - pow(1.0 - contP[id1-config->s_nid][(*z).rid-config->s_nid], (double) r_id1);
					util_id2 = 1.0 - pow(1.0 - contP[id2-config->s_nid][(*z).rid-config->s_nid], (double) r_id2);
/*				if(closeness[id1-config->s_nid][(*z).rid-config->s_nid] >= thresh)	mode = 1;//similarity mode
				else mode = 0;	//centrality mode
				if(mode == 0){
					//centrality mode
					util_id1 = (double) r_id1;
					util_id2 = (double) r_id2;
				}else{
					//similarity mode
					util_id1 = closeness[id1-config->s_nid][(*z).rid-config->s_nid];
					util_id2 = closeness[id2-config->s_nid][(*z).rid-config->s_nid];
				}*/
				}else if (metric == 0){
					util_id1 = (double) r_id1;
					util_id2 = (double) r_id2;
				}
							//node id1 hands off the message

								if((util_id2 > util_id1)){
									if (DEBUG) log << "message id " << (*z).id << " copied from " << id1 << " to " << id2 << " 's buffer at " << current_time << " rank(s) " << util_id1 << " relay's rank " << util_id2 << endl << endl;
								
									//greedy: copy of the relay is closer to desination
									msg.id = (*z).id;
									msg.sid = (*z).sid;;
									msg.rid = (*z).rid;
									msg.gen_t = (*z).gen_t;
									msg.no_hops = (*z).no_hops + 1;
									if(id2 == (*z).rid)	msg.delivered = true;//this is the destination of msg
									else msg.delivered = false;
									nodes[id2-config->s_nid].buffer_list.push_back(msg);	// copy the message to the end of the id2's buffer!
									if(id2 != (*z).rid)	(*z).cost++;	//since we have transmitted this message from id1 to id2
								}
								//let's check if ID2 is the msg.rcvr
								if(id2 == (*z).rid && (*z).gen_t <= stime){
									(*z).cost++;	//we have to count the last step delivery
									(*z).delivered = true;	//stop sending this data anymore
									//Go to the corresponding entry of traffic profile and updates the attributes
									//id1 has a video for id2
									if (DEBUG) log << "node: " << id2 << " is receiver" << endl;
									if (DEBUG) log << "message " << (*z).id << " generated at " << (*z).gen_t << " delivered at " << stime << " by " << id1 << endl;
									
									double delay = (double)(stime - (*z).gen_t)/(double)3600.0;
									if (DEBUG) log << "delay of delivery: " << delay << endl;
									it1 = z;	//let's save the iterator to remove this buffer from id1's buffer???
	
									//let's update the traffic profile table for delivered videos
									for(k = traf_list[(*z).sid-config->s_nid].begin(); k != traf_list[(*z).sid-config->s_nid].end(); ++k){ 
										if((*k).sndr_id == (*z).sid && (*k).rcvr_id == id2 && (*k).gen_t == (*z).gen_t){
											//successful delivery
											(*k).success = 1;	
											(*k).hops = (*z).no_hops + 1;	//one hop transmission	
											(*k).delay = stime - (*z).gen_t;//delay in sec
											(*k).rcv_t = stime;	//received time in sec
										}
											
										if (DEBUG) log << "1updating profile for node: " << (*k).sndr_id << " to node: " << (*k).rcvr_id << " in traffic profile table!" << endl; // print member
										//cout << endl;
									}
								}
							}//else since ID1 is the msg.rcvr --> NOP
						}
					}	
				}
	
	
				//read id2's and id1's buffers to see which messages should be copied to the ther one's buffer
				flag1 = 0;	// to see if we should erase anything from the buffer of id2  
				//let's go through id2.buffer to see if there is any msg to be copied from ID2--> ID1
				//We are looking for T21 = T2-T1
				for(z = nodes[id2-config->s_nid].buffer_list.begin(); z != nodes[id2-config->s_nid].buffer_list.end(); ++z){
					flag1 = 0;
					for(y = nodes[id1-config->s_nid].buffer_list.begin(); y != nodes[id1-config->s_nid].buffer_list.end(); ++y){		
						if ((*z).id == (*y).id){
							flag1 = 1;	//this message has already been copied to id2's buffer
							break;		//we should break since this message (*z).id is in id2's buffer
						}
					}
					if(flag1 == 0 && (*z).gen_t <= current_time){
						//cout << "mesage id: " << (*z).id << " does not exist in " << id1 << "'s buffer" << endl;
						//T12 != null
						//we should copy the message from id1's buffer to id2's buffer
						//Message M is only copied once
						if((current_time - (*z).gen_t <= TTL) && ((*z).delivered == false) && ((*z).cost==0)){//check if msg has been expired
							//message has not been expired
							if(id2 != (*z).rid){//check if ID2 is the receiver of the message, if so then we dont need o copy the msg from ID1 to ID2!
							//core of routing scheme decision: Flooding
				if(metric == 2){
					//let's find utilities for id1 and id2 to contact the target. this utility incorporates both similarity and centrality of  nodes
					util_id1 = 1.0 - pow(1.0 - contP[id1-config->s_nid][(*z).rid-config->s_nid], (double) r_id1);
					util_id2 = 1.0 - pow(1.0 - contP[id2-config->s_nid][(*z).rid-config->s_nid], (double) r_id2);
/*				if(closeness[id2-config->s_nid][(*z).rid-config->s_nid] >= thresh)	mode = 1;//similarity mode
				else mode = 0;	//centrality mode
				if(mode == 0){
					//centrality mode
					util_id1 = (double) r_id1;
					util_id2 = (double) r_id2;
				}else{
					//similarity mode
					util_id1 = closeness[id1-config->s_nid][(*z).rid-config->s_nid];
					util_id2 = closeness[id2-config->s_nid][(*z).rid-config->s_nid];
				}*/
				}else if (metric == 0){
					util_id1 = (double) r_id1;
					util_id2 = (double) r_id2;
				}
	

							if((util_id1 > util_id2)){
								if (DEBUG) log << "message id " << (*z).id << " copied from " << id2 << " to " << id1 << " 's buffer at " << current_time << " rank(s) " << util_id2 << " relay's rank " << util_id1 << endl << endl;

								//if relay is closer to destination, copy the file
								msg.id = (*z).id;
								msg.sid = (*z).sid;;
								msg.rid = (*z).rid;
								msg.gen_t = (*z).gen_t;
								msg.no_hops = (*z).no_hops + 1;
								if(id1 == (*z).rid)	msg.delivered = true;//this is the destination of msg
								else msg.delivered = false;
								nodes[id1-config->s_nid].buffer_list.push_back(msg);	// copy the message to the end of the id2's buffer!
								if(id1 != (*z).rid)	(*z).cost++;	//since we have transmitted this message from id1 to id2
								}
								//let's check if ID1 is the msg.rcvr
								if(id1 == (*z).rid && (*z).gen_t <= stime){
									(*z).cost++;	//we have to count the last step delivery
									(*z).delivered = true;
									//Go to the corresponding entry of traffic profile and updates the attributes
									//id1 has a video for id2
									if (DEBUG) log << "node " << id1 << " is receiver" << endl;
									if (DEBUG) log << "message " << (*z).id << " generated at " << (*z).gen_t << " delivered at " << stime << " by " << id2 << endl;
									double delay = (double)(stime - (*z).gen_t)/(double)3600.0;
									
									if (DEBUG) log << "delivery delay: " << delay << endl;
									it1 = z;	//let's save the iterator to remove this buffer from id1's buffer
	
									//let's update the traffic profile table for delivered videos
									for(k = traf_list[(*z).sid-config->s_nid].begin(); k != traf_list[(*z).sid-config->s_nid].end(); ++k){ 
										if((*k).sndr_id == (*z).sid && (*k).rcvr_id == id1 && (*k).gen_t == (*z).gen_t){
											//successful delivery
											(*k).success = 1;	
											(*k).hops = (*z).no_hops + 1;	//one hop transmission	
											(*k).delay = stime - (*z).gen_t;//delay in sec
											(*k).rcv_t = stime;	//received time in sec
										}
											
										if (DEBUG) log << "2updating profile for node: " << (*k).sndr_id << " to node: " << (*k).rcvr_id << " in traffic profile table!" << endl; // print member
										//cout << endl;
									}
								}
							}//else since ID1 is the msg.rcvr --> NOP
						}
					}	
				}

			}//end for to go throug all returned value from mysql
			//let's virtually clean up all the timeput messages from nodes buffers
			int i;
			int current_time = ts + 2*TTL + 1; //update for solving the bug of msgs with uniform gen times
			for (i = config->s_nid; i <= config->e_nid; i++){
				//let's go through id1's buffer to find expired message, we should update the traffic profile and remove those message from the buffer
				timeout(i, current_time, log);	//let's go through id1 and id2 buffers and remove all timeout messages
			}


			//let's store delay in delayNDelg.dat file: RsnkSec Scheme
			ofstream delayND( "/home/kazem/Desktop/traces/logs/delayNDelg.dat", ios::app );
				
			// exit program if unable to create file
				
			ofstream costND( "/home/kazem/Desktop/traces/logs/costNDelg.dat", ios::app );
			
			// exit program if unable to create file
			
			if ( !delayND || !costND ) // overloaded ! operator
			{
			cerr << "File delayNDelg.dat or costNDelg.dat could not be opened" << endl;
			return -1;
			} // end if
			
			write_profile(delayND, costND, time, 11);
			delayND.close();
			costND.close();
			log << "-----------------------------------------------------------------------" << endl;
			log.close();
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

/*Greedy forewarding method
this is the extended version of GreedyA where the degree also has been taken into account!
arguments: time = scheme no, wLen= window length for counting unique encouners of each node, and uTime= update duration for calling uRank function, waitingT= waiting time for starting to deliver the message, alpha: A sends a message to B if Rank(B) > (1+alpha)*Rank(A) to minimizae the cost, we should tune this parameter st we get the max SDR by delivering the minimum no of messages. Stretegy: 0 if we follow a Secretery kind of strategy to monoitor the first waitingT to make decisions, 1: if we keep updating the rank during the working time to find the most relevant degree accroding to the environment
*/
int routing::GreedyD(int time, int wLen, int uTime, int waitingT, double alpha, int strategy){
	list<traffic>::iterator k;	// an iterator for the link list
	
	message msg; 
	int r_id1 = 0;	//the rank of node id1
	int r_id2 = 0;	//rank of node id2
	int lUpdate = 0;	//this is for ranking updates
	bool cond_1 = false, cond_2 = false;

	//let's store all interaction of networks in log.dat file
	ofstream log( "/home/kazem/Desktop/traces/logs/log.dat", ios::app );
	
	// exit program if unable to create file
	
	if ( !log ) // overloaded ! operator
	{
		cerr << "File log.dat could not be opened" << endl;
		return -1;
	}

	// Connect to database 
	mysqlpp::Connection conn(false);

	/* Connect to database */
	if (conn.connect(config->db, "localhost", "root", "mysql")) {
		// Retrieve a subset of the sample stock table set up by resetdb
		// and display it
		cout << "Connected to db!" << endl;
		
		char _query [500];
		bzero(_query, 500);
		//we should count all contacts: not only long ones!!!
		sprintf (_query, "select user1, user2, starttime, endtime from %s where user1 <= %d and user1 >= %d and user2 <= %d and user2 >= %d and starttime > %d and starttime < %d order by starttime", config->table, config->e_nid, config->s_nid, config->e_nid, config->s_nid, ts, ts + TTL);

		mysqlpp::Query query = conn.query(_query);
		if (mysqlpp::StoreQueryResult res = query.store()){
			//greedy scheme
			cout << "Communication using Social-Greedy IV scheme for routing:" << endl;
			
			double thresh = 0.1;	//threshod for routing	
			for (size_t i = 0; i < res.num_rows(); ++i) {

				//going through db and finding contact duration time for each contact and store them inside map table
				int stime = res[i][2];	//starting time
				int etime = res[i][3];	//end time
				int id1 = res[i][0];	//user1
				int id2 = res[i][1];	//user2
				int current_time = stime;	//emulation clock is updated
				//let's check if user1 or user2 has video for the other one at a time <= stime, therefore we have to check the buffers of two users
				list<message>::iterator y, z, it1, it2, it3,it4;	// an iterator for the buffer link list

				if((lUpdate == 0 || (current_time - lUpdate) > uTime) && ((current_time - ts) <= TTL)){
					//we have to update the rank list for all users
					cg.updateRanks(ts, current_time, wLen, 0);
					lUpdate = current_time;		//update the last update pointer for rank lists
				}

				//there is a contact between id1 and id2
				r_id1 = cg.rank(id1);
				if(r_id1 == 0)	r_id1++;
				r_id2 = cg.rank(id2);
				if(r_id2 == 0)	r_id2++; 
				//if(r_id1 == 1 && r_id2 == 1)	thresh = 0.1;
				//id1 and id2 have met each other, so let's increment the no of contacted nodes
				//let's calculate the most popular node which has been met so far
//				if(nodes[id1-config->s_nid].metNodes < nodes[id1-config->s_nid].threshold)
//this is similar to Secretery problem in which we monitor for a while to make decisions
				if(current_time - ts < waitingT){
					if (r_id2 > nodes[id1-config->s_nid].maxRank)	nodes[id1-config->s_nid].maxRank = r_id2;
					if (r_id1 > nodes[id2-config->s_nid].maxRank)	nodes[id2-config->s_nid].maxRank = r_id1;
				}								
				//let's go through id1's buffer to find expired message, we should update the traffic profile and remove those message from the buffer
				timeout(id1, id2, current_time, log);	//let's go through id1 and id2 buffers and remove all timeout messages
	
				
				//now let's compare T1 and T2 to see what should we flood
				//read id1's and id2's buffers to see which messages should be copied to the ther one's buffer
				int flag1 = 0;	// to see if we should erase anything from the buffer of id1  
				//let's go through id1.buffer to see if there is any msg to be copied from ID1--> ID2
				//We are looking for T12 = T1-T2
				for(z = nodes[id1-config->s_nid].buffer_list.begin(); z != nodes[id1-config->s_nid].buffer_list.end(); ++z){
					flag1 = 0;
	
					for(y = nodes[id2-config->s_nid].buffer_list.begin(); y != nodes[id2-config->s_nid].buffer_list.end(); ++y){		
						if ((*z).id == (*y).id){
							flag1 = 1;	//this message has already been found in id2's buffer
							break;
						}
					}
					if(flag1 == 0 && (*z).gen_t <= current_time){
						//cout << "mesage id: " << (*z).id << " does not exist in " << id2 << "'s buffer" << endl;
						//T12 != null
						//we should copy the message from id1's buffer to id2's buffer
						if(current_time - (*z).gen_t <= TTL){//check if msg has been expired
							//message has not been expired
							if(id1 != (*z).rid){//check if ID1 is the receiver of the message, if so then we dont need to copy the msg from ID1 to ID2!
							//core of routing scheme decision: Flooding
							//core of routing scheme decision: Flooding
							//let's check if id2 has higher rank the best node which id1 has met so far
							//threshold
//							if(((1-pow((1-closeness[id1-config->s_nid][(*z).rid-config->s_nid]), (1+alpha)*r_id1)) < ((1-pow(1-closeness[id2-config->s_nid][(*z).rid-config->s_nid], r_id2)) - thresh)) && ((*z).delivered == false)){
				if(current_time - ts < waitingT)
					cond_1 = (closeness[id1-config->s_nid][(*z).rid-config->s_nid] < closeness[id2-config->s_nid][(*z).rid-config->s_nid] - thresh) && ((*z).delivered == false);
				else
					cond_1 = (closeness[id1-config->s_nid][(*z).rid-config->s_nid] < closeness[id2-config->s_nid][(*z).rid-config->s_nid] - thresh) && ((*z).delivered == false) || ((r_id2 > nodes[id1-config->s_nid].maxRank) && (r_id2 > (1+alpha)*r_id1));

							if(cond_1){	
									if (DEBUG) log << "1message id: " << (*z).id << " is copied from: " << id1 << " to: " << id2 << "'s buffer at: " << current_time << ", closeness (sender, dst): " << closeness[id1-config->s_nid][(*z).rid-config->s_nid] << ", closeness (receiver, dst): " << closeness[id2-config->s_nid][(*z).rid-config->s_nid] << ", rank(s): " << r_id1 << ", relay's rank: " << r_id2  << endl << endl;
								
								
									//greedy: copy of the relay is closer to desination
									msg.id = (*z).id;
									msg.sid = (*z).sid;;
									msg.rid = (*z).rid;
									msg.gen_t = (*z).gen_t;
									msg.no_hops = (*z).no_hops + 1;
									if(id2 == (*z).rid)	msg.delivered = true;//this is the destination of msg
									else msg.delivered = false;
									nodes[id2-config->s_nid].buffer_list.push_back(msg);	// copy the message to the end of the id2's buffer!
									(*z).cost++;	//since we have transmitted this message from id1 to id2
								}
								//let's check if ID2 is the msg.rcvr
								if(id2 == (*z).rid && (*z).gen_t <= stime){
									(*z).delivered = true;	//stop sending this data anymore
									//Go to the corresponding entry of traffic profile and updates the attributes
									//id1 has a video for id2
									if (DEBUG) log << "1node: " << id2 << " is the receiver" << endl;
									if (DEBUG) log << "message: " << (*z).id << " generated at: " << (*z).gen_t << ", delivered at: " << stime << "by: " << id1 << endl;
									
									double delay = (double)(stime - (*z).gen_t)/(double)3600.0;
									if (DEBUG) log << "delay of delivery: " << delay << endl;
									it1 = z;	//let's save the iterator to remove this buffer from id1's buffer???
	
									//let's update the traffic profile table for delivered videos
									for(k = traf_list[(*z).sid-config->s_nid].begin(); k != traf_list[(*z).sid-config->s_nid].end(); ++k){ 
										if((*k).sndr_id == (*z).sid && (*k).rcvr_id == id2 && (*k).gen_t == (*z).gen_t && (*k).id == (*z).id){
											//successful delivery
											(*k).success = 1;	
											(*k).hops = (*z).no_hops + 1;	//one hop transmission	
											(*k).delay = stime - (*z).gen_t;//delay in sec
											(*k).rcv_t = stime;	//received time in sec
										}
											
										if (DEBUG) log << "1updating profile for node: " << (*k).sndr_id << " to node: " << (*k).rcvr_id << " in traffic profile table!" << endl; // print member
										//cout << endl;
									}
								}
							}//else since ID1 is the msg.rcvr --> NOP
						}
					}	
				}
	
	
				//read id2's and id1's buffers to see which messages should be copied to the ther one's buffer
				flag1 = 0;	// to see if we should erase anything from the buffer of id2  
				//let's go through id2.buffer to see if there is any msg to be copied from ID2--> ID1
				//We are looking for T21 = T2-T1
				for(z = nodes[id2-config->s_nid].buffer_list.begin(); z != nodes[id2-config->s_nid].buffer_list.end(); ++z){
					flag1 = 0;
					for(y = nodes[id1-config->s_nid].buffer_list.begin(); y != nodes[id1-config->s_nid].buffer_list.end(); ++y){		
						if ((*z).id == (*y).id){
							flag1 = 1;	//this message has already been copied to id2's buffer
							break;		//we should break since this message (*z).id is in id2's buffer
						}
					}
					if(flag1 == 0 && (*z).gen_t <= current_time){
						//cout << "mesage id: " << (*z).id << " does not exist in " << id1 << "'s buffer" << endl;
						//T12 != null
						//we should copy the message from id1's buffer to id2's buffer
						if(current_time - (*z).gen_t <= TTL){//check if msg has been expired
							//message has not been expired
							if(id2 != (*z).rid){//check if ID2 is the receiver of the message, if so then we dont need o copy the msg from ID1 to ID2!
							//core of routing scheme decision: Flooding
							//threshold
//							if(((1-pow((1-closeness[id2-config->s_nid][(*z).rid-config->s_nid]), (1+alpha)*r_id2)) < ((1-pow(1-closeness[id1-config->s_nid][(*z).rid-config->s_nid] + thresh, r_id1))) - thresh) && ((*z).delivered == false)){			
				if(current_time - ts < waitingT)
					cond_2 = (closeness[id2-config->s_nid][(*z).rid-config->s_nid] < closeness[id1-config->s_nid][(*z).rid-config->s_nid] - thresh) && ((*z).delivered == false);
				else
					cond_2 = (closeness[id2-config->s_nid][(*z).rid-config->s_nid] < closeness[id1-config->s_nid][(*z).rid-config->s_nid] - thresh) && ((*z).delivered == false) || ((r_id1 > nodes[id2-config->s_nid].maxRank) && (r_id1 > (1+alpha)*r_id2));

							if(cond_2){
									if (DEBUG) log << "2message id: " << (*z).id << " is copied from: " << id2 << " to: " << id1 << "'s buffer at: " << current_time << ", closeness (sender, dst): " << closeness[id2-config->s_nid][(*z).rid-config->s_nid] << ", closeness (receiver, dst): " << closeness[id1-config->s_nid][(*z).rid-config->s_nid] << ", rank(s): " << r_id2 << ", relay's rank: " << r_id1 << endl << endl;
								
									//if relay is closer to destination, copy the file
									msg.id = (*z).id;
									msg.sid = (*z).sid;;
									msg.rid = (*z).rid;
									msg.gen_t = (*z).gen_t;
									msg.no_hops = (*z).no_hops + 1;
									if(id1 == (*z).rid)	msg.delivered = true;//this is the destination of msg
									else msg.delivered = false;
									nodes[id1-config->s_nid].buffer_list.push_back(msg);	// copy the message to the end of the id2's buffer!
									(*z).cost++;	//since we have transmitted this message from id1 to id2
								}
								//let's check if ID2 is the msg.rcvr
								if(id1 == (*z).rid && (*z).gen_t <= stime){
									(*z).delivered = true;
									//Go to the corresponding entry of traffic profile and updates the attributes
									//id1 has a video for id2
									if (DEBUG) log << "2node: " << id1 << " is the receiver" << endl;
									if (DEBUG) log << "2message: " << (*z).id << " generated at: " << (*z).gen_t << ", delivered at: " << stime << "by: " << id2 << endl;
									double delay = (double)(stime - (*z).gen_t)/(double)3600.0;
									
									if (DEBUG) log << "delivery delay: " << delay << endl;
									it1 = z;	//let's save the iterator to remove this buffer from id1's buffer
	
									//let's update the traffic profile table for delivered videos
									for(k = traf_list[(*z).sid-config->s_nid].begin(); k != traf_list[(*z).sid-config->s_nid].end(); ++k){ 
										if((*k).sndr_id == (*z).sid && (*k).rcvr_id == id1 && (*k).gen_t == (*z).gen_t && (*k).id == (*z).id){
											//successful delivery
											(*k).success = 1;	
											(*k).hops = (*z).no_hops + 1;	//one hop transmission	
											(*k).delay = stime - (*z).gen_t;//delay in sec
											(*k).rcv_t = stime;	//received time in sec
										}
											
										if (DEBUG) log << "2updating profile for node: " << (*k).sndr_id << " to node: " << (*k).rcvr_id << " in traffic profile table!" << endl; // print member
										//cout << endl;
									}
								}
							}//else since ID1 is the msg.rcvr --> NOP
						}
					}	
				}
			}//end for
			
			

			//let's store delay in delayG.dat file: Greedy IV Scheme
			ofstream delayGD( "/home/kazem/Desktop/traces/logs/delayGD.dat", ios::app );
				
			// exit program if unable to create file
				
			ofstream costGD( "/home/kazem/Desktop/traces/logs/costGD.dat", ios::app );
			
			// exit program if unable to create file
		
			if ( !delayGD || !costGD ) // overloaded ! operator
			{
				cerr << "File delayGD.dat or costGD.dat could not be opened" << endl;
				return -1;
			} // end if
			//write delay and cost
			write_profile(delayGD, costGD, time, 12);
			delayGD.close();
			costGD.close();
		        log << "-----------------------------------------------------------------------" << endl;
			log.close();
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

//this routing scheme choose a random number r between 1 and 79 and pass the message to the rth node
//passP: is the probability that one node pass a message to another node when it reaches it threshold for the seq no
int routing::RandomSec(int time, int wLen, int uTime, double passP){//, int waitingT, double alpha, int strategy){
	list<traffic>::iterator k;	// an iterator for the link list
	map<int, int>::iterator ii;
	message msg;
	int r_id1 = 0;	//the rank of node id1
	int r_id2 = 0;	//rank of node id2
	bool l_id1D = false;	//is true if id1 has the same label (group) as the msg's dest
	bool l_id2D = false;	//is true if id2 has the same label (group) as the msg's dest
	int lUpdate = 0;	//this is for ranking updates
	mcp = 1;	//no of copies per message

	//let's find the seq no of each node when the routing should happen
	double rand = get_rand();
	for (int i = config->s_nid; i <= config->e_nid; i++)	nodes[i-config->s_nid].randSeq = floor(rand*config->node);
	

	//let's store all interaction of networks in log.dat file
	ofstream log( "/home/kazem/Desktop/traces/logs/log.dat", ios::app );
	
	// exit program if unable to create file
	if ( !log ) // overloaded ! operator
	{
		cerr << "File log.dat could not be opened" << endl;
		return -1;
	}

	// Connect to database 
	mysqlpp::Connection conn(false);

	/* Connect to database */
	if (conn.connect(config->db, "localhost", "root", "mysql")) {
		// Retrieve a subset of the sample stock table set up by resetdb
		// and display it
		cout << "Connected to db!" << endl;
		// and display it
		char _query [500];
		bzero(_query, 500);
		//we should count all contacts: not only long ones!!!
		sprintf (_query, "select user1, user2, starttime, endtime from %s where user1 <= %d and user1 >= %d and user2 <= %d and user2 >= %d and starttime > %d and starttime < %d order by starttime", config->table, config->e_nid, config->s_nid, config->e_nid, config->s_nid, ts, ts + TTL);

		mysqlpp::Query query = conn.query(_query);
		if (mysqlpp::StoreQueryResult res = query.store()){
			//BUBBLE SCHEME
			cout << "Communication using Random Secretary scheme for routing:" << endl;
			for (size_t i = 0; i < res.num_rows(); ++i){
				//going through db and finding contact duration time for each contact and store them inside map table
				int stime = res[i][2];	//starting time
				int etime = res[i][3];	//end time
				int id1 = res[i][0];	//user1
				int id2 = res[i][1];	//user2
				int current_time = stime;	//emulation clock is updated
				//let's check if user1 or user2 has video for the other one at a time <= stime, therefore we have to check the buffers of two users
				list<message>::iterator y, z, it1, it2, it3,it4;	// an iterator for the buffer link list

				if((lUpdate == 0 || (current_time - lUpdate) > uTime) && ((current_time - ts) <= TTL)){
					//we have to update the rank list for all users
					cg.updateRanks(ts, current_time, wLen, 0);
					lUpdate = current_time;		//update the last update pointer for rank lists
				}

				//there is a contact between id1 and id2
				r_id1 = cg.rank(id1);
				r_id2 = cg.rank(id2);
				
				//we keep updating the list of observed nodes to calculate the actual max of the list and compare it with the candidate we have chosen for passing over the message
				if(1){
					//we need to log the actual max of the observed node
					if (r_id1 > nodes[id2-config->s_nid].maxSeq)	nodes[id2-config->s_nid].maxSeq = r_id1;
					nodes[id2-config->s_nid].seqNo++;	//update the seq no
					//node id2 is in observation state
					ii = nodes[id2-config->s_nid].nodesMet.find(id1);
	
					if((*ii).first != id1 && r_id1 != 0){
//						cout << '\t' << "met node: " << id1 << endl;
						nodes[id2-config->s_nid].nodesMet.insert(std::make_pair(id1, r_id1));
						//let's calculate the most popular node which has been met so far if the time is less than optimal stopping time
//						if ((nodes[id2-config->s_nid].nodesMet.size() < stopT) && (r_id1 > nodes[id2-config->s_nid].maxRank))	nodes[id2-config->s_nid].maxRank = r_id1;
					}
				}
				//keep storing the qualities of all met nodes
				if(1){
					//we need to log the actual max of the observed node
					if (r_id2 > nodes[id1-config->s_nid].maxSeq)	nodes[id1-config->s_nid].maxSeq = r_id2;
					nodes[id1-config->s_nid].seqNo++;	//update the seq no
					//node id1 is in observation state
					ii = nodes[id1-config->s_nid].nodesMet.find(id2);
	
					if(((*ii).first != id2) &&(r_id2 != 0)){
//						cout << '\t' << "met node: " << id2 << endl;
						nodes[id1-config->s_nid].nodesMet.insert(std::make_pair(id2, r_id2));
						//we only update the MAX threshold if we are below stopT
//						if ((nodes[id1-config->s_nid].nodesMet.size() < stopT) && (r_id2 > nodes[id1-config->s_nid].maxRank))	nodes[id1-config->s_nid].maxRank = r_id2;
					}

				}
				//let's log all nodes that 50 met after 1:00 pm on Tuesday
/*				if (id1 == 50 && stime > 57600+4*3600 && (nodes[id1-config->s_nid].nodesMet.size() < stopT)){
					cout << "no of observation till " << (double)stime/3600.0 << ": " << nodes[id1-config->s_nid].nodesMet.size() << " & max: " << nodes[id1-config->s_nid].maxRank << endl;
					
					for ( ii = nodes[id1-config->s_nid].nodesMet.begin() ; ii != nodes[id1-config->s_nid].nodesMet.end(); ii++ )
    						cout << (*ii).second << " ";
					cout << endl;
				}*/

				//let's go through id1's buffer to find expired messages, we should update the traffic profile and remove those message from the buffer
				timeout(id1, id2, current_time, log);	//let's go through id1 and id2 buffers and remove all timeout messages


				//now let's compare T1 and T2 to see what should we copy
				//read id1's and id2's buffers to see which messages should be copied to the ther one's buffer
				int flag1 = 0;	// to see if we should erase anything from the buffer of id1  
				//let's go through id1.buffer to see if there is any msg to be copied from ID1--> ID2
				//We are looking for T12 = T1-T2
							//let's check if id2 has higher rank the best node which id1 has met so far
				for(z = nodes[id1-config->s_nid].buffer_list.begin(); z != nodes[id1-config->s_nid].buffer_list.end(); ++z){//#for #1 
					flag1 = 0;

					for(y = nodes[id2-config->s_nid].buffer_list.begin(); y != nodes[id2-config->s_nid].buffer_list.end(); ++y){		
						if ((*z).id == (*y).id){
							flag1 = 1;	//this message has already been found in id2's buffer
							break;
						}
					}
					if(flag1 == 0 && (*z).gen_t <= current_time){//if #1
						//cout << "mesage id: " << (*z).id << " does not exist in " << id2 << "'s buffer" << endl;
						//T12 != null
						//we should copy the message from id1's buffer to id2's buffer
						if((current_time - (*z).gen_t <= TTL)  && ((*z).delivered == false)){//if #2

							//check if msg has been expired
							//message has not been expired
							if(id1 != (*z).rid){//(if #3)check if ID1 is the receiver of the message, if so then we dont need o copy the msg from ID1 to ID2!
				//we start routing after reaching the random seq no for each node
				//we have a passP to adjust the cost with RankSec
				rand = get_rand();	//we need to toss a coin to pass the msg
				if(((nodes[id1-config->s_nid].seqNo == nodes[id1-config->s_nid].randSeq) && ((*z).mcp > 0) && (rand <= passP)) || (id2 == (*z).rid && (*z).gen_t <= stime)){

//let's log the chosen candidate and compare it with max of observed nodes to see how eefective it is
if((nodes[id1-config->s_nid].seqNo == nodes[id1-config->s_nid].randSeq) && ((*z).mcp > 0) && (rand <= passP)){
	nodes[id1-config->s_nid].candN = r_id2;
}
								if (DEBUG) log << "MSG(" << (*z).id << ") copied " << id1 << "-->" << id2 << " @ " << current_time << ", rank(s)=" << r_id1 << ", max Rank(s)=" << nodes[id1-config->s_nid].maxRank << ", relay's rank=" << r_id2 << endl << endl;
								(*z).mcp--;	//no copies anymore
								//greedy: copy of the relay is closer to desination
								msg.id = (*z).id;
								msg.sid = (*z).sid;;
								msg.rid = (*z).rid;
								msg.gen_t = (*z).gen_t;
								msg.no_hops = (*z).no_hops + 1;
								if(id2 == (*z).rid)	msg.delivered = true;//this is the destination of msg
								else msg.delivered = false;
								(*z).cost++;	//since we have transmitted this message from id1 to id2

								msg.mcp = (*z).mcp;
								nodes[id2-config->s_nid].buffer_list.push_back(msg);	// copy the message to the end of the id2's buffer!				
								}
								//let's check if ID2 is the msg.rcvr
								if(id2 == (*z).rid && (*z).gen_t <= stime){//if #4
									(*z).delivered = true;
									//Go to the corresponding entry of traffic profile and updates the attributes
									//id1 has a video for id2
									if (DEBUG) log << "Node " << id2 << " is Rcvr" << endl;
									if (DEBUG) log << "MSG: " << (*z).id << ", gen @: " << (*z).gen_t << ", delivered  @: " << stime << ", by: " << id1 << endl;

									//let's see if the msg has been delivered by a relay node
									if(id1 != (*z).sid)	relayed_msgs++;
									
									double delay = (double)(stime - (*z).gen_t)/(double)3600.0;
									if (DEBUG) log << "delivery delay: " << delay << endl;
									it1 = z;	//let's save the iterator to remove this buffer from id1's buffer???
	
									//let's update the traffic profile table for delivered videos
									for(k = traf_list[(*z).sid-config->s_nid].begin(); k != traf_list[(*z).sid-config->s_nid].end(); ++k){ 
										if((*k).sndr_id == (*z).sid && (*k).rcvr_id == id2 && (*k).gen_t == (*z).gen_t){
											//successful delivery
											(*k).success = 1;	
											(*k).hops = (*z).no_hops + 1;	//one hop transmission	
											(*k).delay = stime - (*z).gen_t;//delay in sec
											(*k).rcv_t = stime;	//received time in sec
										}
											
										if (DEBUG) log << "updating prof of node: " << (*k).sndr_id << " to node: " << (*k).rcvr_id << " in traffic profile table!" << endl; // print member
										//cout << endl;
									}//end for #2
								}//end if #4
							}//if #3 since ID1 is the msg.rcvr --> NOP//end if #3
						}//end if #2
					}//end if #1	
				}//for #1

				//read id2's and id1's buffers to see which messages should be copied to the ther one's buffer
				flag1 = 0;	// to see if we should erase anything from the buffer of id2  
				//let's go through id2.buffer to see if there is any msg to be copied from ID2--> ID1
				//We are looking for T21 = T2-T1
				for(z = nodes[id2-config->s_nid].buffer_list.begin(); z != nodes[id2-config->s_nid].buffer_list.end(); ++z){//for #1
					flag1 = 0;
					for(y = nodes[id1-config->s_nid].buffer_list.begin(); y != nodes[id1-config->s_nid].buffer_list.end(); ++y){		
						if ((*z).id == (*y).id){
							flag1 = 1;	//this message has already been copied to id2's buffer
							break;		//we should break since this message (*z).id is in id2's buffer
						}
					}

					if(flag1 == 0 && (*z).gen_t <= current_time){//if #1
						//cout << "mesage id: " << (*z).id << " does not exist in " << id1 << "'s buffer" << endl;
						//T12 != null
						//we should copy the message from id2's buffer to id1's buffer
						if((current_time - (*z).gen_t <= TTL) && ((*z).delivered == false)){//if #2
							//check if msg has been expired
							//message has not been expired
							if(id2 != (*z).rid){//if # 3: check if ID2 is the receiver of the message, if so then we dont need o copy the msg from ID1 to ID2!
							//core of routing scheme decision: Flooding
							//Bubble: forward if the id2 has the same label as the destination or higher rank!
								//if the current node has the same label as dest, we will forward the message only if id1 has the same label as the dest and higher local rank than id2 as well
							//let's check if id1 has a higher rank than the best node which id2 has met so far
				//we start routing after reaching the random seq no for each node
				//we have a passP to adjust the cost with RankSec
				rand = get_rand();	//we need to toss a coin to pass the msg
				if(((nodes[id2-config->s_nid].seqNo == nodes[id2-config->s_nid].randSeq) && ((*z).mcp > 0) && (rand <= passP)) || (id1 == (*z).rid && (*z).gen_t <= stime)){

//let's log the chosen candidate and compare it with max of observed nodes to see how eefective it is
if((nodes[id2-config->s_nid].seqNo == nodes[id2-config->s_nid].randSeq) && ((*z).mcp > 0) && (rand <= passP)){
	nodes[id2-config->s_nid].candN = r_id1;
}
									if (DEBUG) log << "MSG (" << (*z).id << ") copied " << id2 << "-->" << id1 << " @ " << current_time << ", rank(s)=" << r_id2 << ", max Rank(s)=" << nodes[id2-config->s_nid].maxRank << ", relay's rank=" << r_id1 << endl << endl;
									(*z).mcp--;
									//greedy: copy of the relay is closer to desination
									msg.id = (*z).id;
									msg.sid = (*z).sid;;
									msg.rid = (*z).rid;
									msg.gen_t = (*z).gen_t;
									msg.no_hops = (*z).no_hops + 1;
									msg.mcp = (*z).mcp;
									(*z).cost++;	//since we have transmitted this message from id2 to id1
									if(id1 == (*z).rid)	msg.delivered = true;//this is the destination of msg
									else msg.delivered = false;
									
									nodes[id1-config->s_nid].buffer_list.push_back(msg);	// copy the message to the end of the id1's buffer!
								}
								//let's check if ID1 is the msg.rcvr
								if(id1 == (*z).rid && (*z).gen_t <= stime){//if #4
									(*z).delivered = true;	//stop sending this msg anymore
									//Go to the corresponding entry of traffic profile and updates the attributes
									//id1 has a video for id2
									if (DEBUG) log << "Node: " << id1 << " is Rcvr" << endl;
									if (DEBUG) log << "MSG: " << (*z).id << " gen @ " << (*z).gen_t << ", delivered  @: " << stime << ", by: " << id2 << endl;

									//let's see if the msg has been delivered by a relay node
									if(id2 != (*z).sid)	relayed_msgs++;

									double delay = (double)(stime - (*z).gen_t)/(double)3600.0;
									
									if (DEBUG) log << "delivery delay: " << delay << endl;
									it1 = z;	//let's save the iterator to remove this buffer from id1's buffer
	
									//let's update the traffic profile table for delivered videos
									for(k = traf_list[(*z).sid-config->s_nid].begin(); k != traf_list[(*z).sid-config->s_nid].end(); ++k){ 
										if((*k).sndr_id == (*z).sid && (*k).rcvr_id == id1 && (*k).gen_t == (*z).gen_t){
											//successful delivery
											(*k).success = 1;	
											(*k).hops = (*z).no_hops + 1;	//one hop transmission	
											(*k).delay = stime - (*z).gen_t;//delay in sec
											(*k).rcv_t = stime;	//received time in sec
										}
											
										if (DEBUG) log << "updating prof of node: " << (*k).sndr_id << " to node: " << (*k).rcvr_id << " in traffic profile table!" << endl; // print member
										//cout << endl;
									}//end for #2
								}//end if #4
							}//end if #3 since ID1 is the msg.rcvr --> NOP
						}//end if #2
					}//end if#1
				}//end for #1
//				}//if for copy the message

			}//end for to go throug all returned value from mysql
			//let's virtually clean up all the timeput messages from nodes buffers
			int i;
			int current_time = ts + 2*TTL + 1; //update for solving the bug of msgs with uniform gen times
			for (i = config->s_nid; i <= config->e_nid; i++){
				//let's go through id1's buffer to find expired message, we should update the traffic profile and remove those message from the buffer
				timeout(i, current_time, log);	//let's go through id1 and id2 buffers and remove all timeout messages
			}

			//let's print all the chosen candidates and compare them with the actual max of the list
			for (i = config->s_nid; i <= config->e_nid; i++)
				if(nodes[i-config->s_nid].candN > 0)	cout << nodes[i-config->s_nid].candN << " " << nodes[i-config->s_nid].maxSeq << ";";

			//we have to clean up the whole observed nodes map for all nodes
			for (i = config->s_nid; i <= config->e_nid; i++){	
				nodes[i - config->s_nid].nodesMet.clear();
				nodes[i - config->s_nid].metNodes = 0;
				nodes[i - config->s_nid].maxRank = 0;
				nodes[i - config->s_nid].maxSeq = 0;
				nodes[i - config->s_nid].threshold = 29;
				nodes[i - config->s_nid].candN = -1;
				nodes[i - config->s_nid].seqNo = 0;
				nodes[i - config->s_nid].randSeq = 0;
			}

			//let's store delay in delayRS.dat file: RsnkSec Scheme
			ofstream delayRS( "/home/kazem/Desktop/traces/logs/delayRS.dat", ios::app );
				
			// exit program if unable to create file
				
			ofstream costRS( "/home/kazem/Desktop/traces/logs/costRS.dat", ios::app );
			
			// exit program if unable to create file
			
			if ( !delayRS || !costRS ) // overloaded ! operator
			{
			cerr << "File delayRS.dat or costRS.dat could not be opened" << endl;
			return -1;
			} // end if

			cout << "no of messgae routed by relay nodes: " << relayed_msgs << endl;

			write_profile(delayRS, costRS, time, 13);
			delayRS.close();
			costRS.close();
			log << "-----------------------------------------------------------------------" << endl;
			log.close();
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

/*
TwoHop forwarding
quality metric: rank or nothing --> if metric = 0, then centrality is the metric for routing otherwise we pass the message to the first relay node we encounter 
arguments: time = scheme no, wLen= window length for counting unique encouners of each node, and uTime= update duration for calling uRank function, 
*/
int routing::TwoHop(int time, int wLen, int uTime, int metric){
	int r_id1 = 0;	//the rank of node id1
	int r_id2 = 0;	//rank of node id2
	int lUpdate = 0;	//this is for ranking updates

	int mcp_val = 1;
//	mcp = mcp_val;	//no of copies upper bound: for MCP forwarding scheme

	list<traffic>::iterator k;	// an iterator for the link list
	message msg; 
	//let's store all interaction of networks in log.dat file
	ofstream log( "/home/kazem/Desktop/traces/logs/log.dat", ios::app );
	
	// exit program if unable to create file
	if ( !log ) // overloaded ! operator
	{
		cerr << "File log.dat could not be opened" << endl;
		return -1;
	}

	// Connect to database 
	mysqlpp::Connection conn(false);

	/* Connect to database */
	if (conn.connect(config->db, "localhost", "root", "mysql")) {
		// Retrieve a subset of the sample stock table set up by resetdb
		// and display it
		cout << "Connected to db!" << endl;
		
		char _query [500];
		bzero(_query, 500);
		//we should count all contacts: not only long ones!!!
		sprintf (_query, "select user1, user2, starttime, endtime from %s where user1 <= %d and user1 >= %d and user2 <= %d and user2 >= %d and starttime > %d and starttime < %d order by starttime", config->table, config->e_nid, config->s_nid, config->e_nid, config->s_nid, ts, ts + TTL);

		mysqlpp::Query query = conn.query(_query);
		if (mysqlpp::StoreQueryResult res = query.store()){
			//greedy scheme
		//mcp scheme
		cout << "Communication using Two Hop scheme for routing:" << endl;
			for (size_t i = 0; i < res.num_rows(); ++i) {
				//going through db and finding contact duration time for each contact and store them inside map table
				int stime = res[i][2];	//starting time
				int etime = res[i][3];	//end time
				int id1 = res[i][0];	//user1
				int id2 = res[i][1];	//user2
				int current_time = stime;	//emulation clock is updated
				//let's check if user1 or user2 has video for the other one at a time <= stime, therefore we have to check the buffers of two users
				list<message>::iterator y, z, it1, it2, it3,it4;	// an iterator for the buffer link list

				if(metric == 0){
					//we are intersted in contact rates per unit time (total no of contacts for the last wlen)
					if((lUpdate == 0 || (current_time - lUpdate) > uTime) && ((current_time - ts) <= TTL)){
						//we have to update the rank list for all users
						cg.updateRanks(ts, current_time, wLen, 0);
						lUpdate = current_time;		//update the last update pointer for rank lists
					}
					//there is a contact between id1 and id2
					r_id1 = cg.rank(id1);
					r_id2 = cg.rank(id2);
				}
				
				//let's go through id1's buffer to find expired message, we should update the traffic profile and remove those message from the buffer
				timeout(id1, id2, current_time, log);	//let's go through id1 and id2 buffers and remove all timeout messages

				
				//now let's compare T1 and T2 to see what should we flood
				//read id1's and id2's buffers to see which messages should be copied to the ther one's buffer
				int flag1 = 0;	// to see if we should erase anything from the buffer of id1  
				//let's go through id1.buffer to see if there is any msg to be copied from ID1--> ID2
				//We are looking for T12 = T1-T2
				for(z = nodes[id1-config->s_nid].buffer_list.begin(); z != nodes[id1-config->s_nid].buffer_list.end(); ++z){
					flag1 = 0;
					for(y = nodes[id2-config->s_nid].buffer_list.begin(); y != nodes[id2-config->s_nid].buffer_list.end(); ++y){		
						if ((*z).id == (*y).id){
							flag1 = 1;	//this message has already been found in id2's buffer
							break;		//we dont need to copy this message
						}
					}
					//let's uniformly transmit the messages
					
					if(flag1 == 0 && (*z).gen_t <= current_time){
						//number of flooding is bounded by cg array
						//cout << "mesage id: " << (*z).id << " does not exist in " << id2 << "'s buffer" << endl;
						//T12 != null
						//we should copy the message from id1's buffer to id2's buffer
						if((current_time - (*z).gen_t <= TTL) && ((*z).delivered == false)){//check if msg has been expired
							//message has not been expired
							if(id1 != (*z).rid){//check if ID1 is the receiver of the message, if so then we dont need o copy the msg from ID1 to ID2!
							//core of routing scheme decision: copy
							if(((*z).mcp > 0) && ((r_id1 < r_id2) || metric)){
							//we should copy to the relay node
								(*z).mcp--;//update the number of flooding for this message
								if (DEBUG) log << "mesage id: " << (*z).id << " is copied from: " << id1 << " to: " << id2 << "'s buffer at: " << current_time << ", rank(s): " << r_id1 << ", rank(r): " << r_id2  << "two hop" << (*z).mcp << endl;
								msg.id = (*z).id;
								msg.sid = (*z).sid;;
								msg.rid = (*z).rid;
								msg.gen_t = (*z).gen_t;
								msg.no_hops = (*z).no_hops + 1;
								msg.mcp = 0;	//we only relay the message once
								msg.hopttl = (*z).hopttl - 1;	//hop count decrements
								if(id2 == (*z).rid)	msg.delivered = true;//this is the destination of msg
								else msg.delivered = false;
								nodes[id2-config->s_nid].buffer_list.push_back(msg);	// copy the message to the end of the id2's buffer!
								if(id2 != (*z).rid)	(*z).cost++;	//since we have transmitted this message from id1 to id2
							}
								//let's check if ID2 is the msg.rcvr
								if(id2 == (*z).rid && (*z).gen_t <= stime){
									(*z).cost++;	//we have to count the last step delivery
									(*z).delivered = true;	//top transmission of this msg
									//Go to the corresponding entry of traffic profile and updates the attributes
									//id1 has a video for id2
									if (DEBUG) log << "1node: " << id2 << " is the receiver" << endl;
									if (DEBUG) log << "message: " << (*z).id << " generated at: " << (*z).gen_t << ", delivered at: " << stime << endl;
									
									double delay = (double)(stime - (*z).gen_t)/(double)3600.0;
									if (DEBUG) log << "delay of delivery: " << delay << endl;
									it1 = z;	//let's save the iterator to remove this buffer from id1's buffer???
	
									//let's update the traffic profile table for delivered videos
									for(k = traf_list[(*z).sid-config->s_nid].begin(); k != traf_list[(*z).sid-config->s_nid].end(); ++k){ 
										if((*k).sndr_id == (*z).sid && (*k).rcvr_id == id2 && (*k).gen_t == (*z).gen_t){
											//successful delivery
											(*k).success = 1;	
											(*k).hops = (*z).no_hops + 1;	//one hop transmission	
											(*k).delay = stime - (*z).gen_t;//delay in sec
											(*k).rcv_t = stime;	//received time in sec
										}
											
										if (DEBUG) log << "1updating profile for node: " << (*k).sndr_id << " to node: " << (*k).rcvr_id << " in traffic profile table!" << endl; // print member
										//cout << endl;
									}
								}
							}//else since ID1 is the msg.rcvr --> NOP
						}
					}	
				}


				//read id2's and id1's buffers to see which messages should be copied to the ther one's buffer
				flag1 = 0;	// to see if we should erase anything from the buffer of id2  
				//let's go through id2.buffer to see if there is any msg to be copied from ID2--> ID1
				//We are looking for T21 = T2-T1
				for(z = nodes[id2-config->s_nid].buffer_list.begin(); z != nodes[id2-config->s_nid].buffer_list.end(); ++z){
					flag1 = 0;
					for(y = nodes[id1-config->s_nid].buffer_list.begin(); y != nodes[id1-config->s_nid].buffer_list.end(); ++y){		
						if ((*z).id == (*y).id){
							flag1 = 1;	//this message has already been copied to id2's buffer
							break;		//stop processing more
						}
					}

					if(flag1 == 0 && (*z).gen_t <= current_time){
						//cout << "mesage id: " << (*z).id << " does not exist in " << id1 << "'s buffer" << endl;
						//T12 != null
						//we should copy the message from id2's buffer to id1's buffer
						if((current_time - (*z).gen_t <= TTL) && ((*z).delivered == false)){//check if msg has been expired
							//message has not been expired
							if(id2 != (*z).rid){//check if ID2 is the receiver of the message, if so then we dont need o copy the msg from ID1 to ID2!
							//core of routing scheme decision: check if we should copy the message
							if(((*z).mcp > 0) && ((r_id2 < r_id1) || metric)){
								//we should copy to the relay node
								(*z).mcp--;	//update the number of flooding for this message
								if (DEBUG) log << "mesage id: " << (*z).id << " is copied from: " << id2 << " to: " << id1 << "'s buffer at: " << current_time << ", rank(s): " << r_id2 << ", rank(r): " << r_id1  << "two-hop" << (*z).mcp << endl;

								msg.id = (*z).id;
								msg.sid = (*z).sid;;
								msg.rid = (*z).rid;
								msg.gen_t = (*z).gen_t;
								msg.no_hops = (*z).no_hops + 1;
								msg.hopttl = (*z).hopttl - 1;	//decremet hop ttl by one
 								msg.mcp = 0;	//the msg only is relayed once
								if(id1 == (*z).rid)	msg.delivered = true;//this is the destination of msg
								else msg.delivered = false;
								nodes[id1-config->s_nid].buffer_list.push_back(msg);	// copy the message to the end of the id2's buffer!
								if(id1 != (*z).rid)	(*z).cost++;	//since we have transmitted this message from id1 to id2
}
								//let's check if ID2 is the msg.rcvr
								if(id1 == (*z).rid && (*z).gen_t <= stime){
									(*z).cost++;	//we have to count the last step delivery
									(*z).delivered = true;	//stop sending this message
									//Go to the corresponding entry of traffic profile and updates the attributes
									//id1 has a video for id2
									if (DEBUG) log << "2node: " << id1 << " is the receiver" << endl;
									if (DEBUG) log << "2message: " << (*z).id << " generated at: " << (*z).gen_t << ", delivered at: " << stime << endl;
									double delay = (double)(stime - (*z).gen_t)/(double)3600.0;
									
									if (DEBUG) log << "delivery delay: " << delay << endl;
									it1 = z;	//let's save the iterator to remove this buffer from id1's buffer
	
									//let's update the traffic profile table for delivered videos
									for(k = traf_list[(*z).sid-config->s_nid].begin(); k != traf_list[(*z).sid-config->s_nid].end(); ++k){ 
										if((*k).sndr_id == (*z).sid && (*k).rcvr_id == id1 && (*k).gen_t == (*z).gen_t){
											//successful delivery
											(*k).success = 1;	
											(*k).hops = (*z).no_hops + 1;	//one hop transmission	
											(*k).delay = stime - (*z).gen_t;//delay in sec
											(*k).rcv_t = stime;	//received time in sec
										}
											
										if (DEBUG) log << "2updating profile for node: " << (*k).sndr_id << " to node: " << (*k).rcvr_id << " in traffic profile table!" << endl; // print member
										//cout << endl;
									}
								}
							}//else since ID1 is the msg.rcvr --> NOP
						}
					}	
				}
			}//end for
			int i;
			int current_time = ts + 2*TTL + 1; //update for solving the bug of msgs with uniform gen times
			for (i = config->s_nid; i <= config->e_nid; i++){
				//let's go through id1's buffer to find expired message, we should update the traffic profile and remove those message from the buffer
				timeout(i, current_time, log);	//let's go through id1 and id2 buffers and remove all timeout messages
			}

			//let's store delay in delayM.dat file: Waiting Scheme
			ofstream delayTH( "/home/kazem/Desktop/traces/logs/delayTH.dat", ios::app );
				
			// exit program if unable to create file
				
			ofstream costTH( "/home/kazem/Desktop/traces/logs/costTH.dat", ios::app );
			
			// exit program if unable to create file
			
			if ( !delayTH || !costTH ) // overloaded ! operator
			{
			cerr << "File delayTH.dat or costTH could not be opened" << endl;
			return -1;
			} // end if
			
			write_profile(delayTH, costTH, time, 14);
			delayTH.close();
			costTH.close();
		        log << "-----------------------------------------------------------------------" << endl;
			log.close();
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

int routing::enumShortPaths(int time){

	int paths[79];	//let's store paths in this array: the no of times node i appears in a short delay path
	for (int i = 0; i< 79; i++) paths[i] = 0;	//initialize the paths array

	list<traffic>::iterator k;	// an iterator for the link list
	list< multiple >::iterator it;	

	message msg; 

	int dUpdate = ts;	//this is for deleteing expired messages

	//let's store all paths in path.dat file
	ofstream path( "/home/kazem/Desktop/traces/logs/path.dat", ios::app );
	
	// exit program if unable to create file
	
	if ( !path ) // overloaded ! operator
	{
		cerr << "File path.dat could not be opened" << endl;
		return -1;
	}

// Connect to database 
	mysqlpp::Connection conn(false);

	/* Connect to database */
	if (conn.connect(config->db, "localhost", "root", "mysql")) {
		// Retrieve a subset of the sample stock table set up by resetdb
		// and display it
		cout << "Connected to db!" << endl;
		int maxD = 480;	//max delay for paths
		char _query [500];
		bzero(_query, 500);
		//we should count all contacts: not only long ones!!!
		sprintf (_query, "select user1, user2, starttime, endtime from %s where user1 <= %d and user1 >= %d and user2 <= %d and user2 >= %d and starttime > %d and starttime < %d order by starttime", config->table, config->e_nid, config->s_nid, config->e_nid, config->s_nid, ts, ts + TTL);
		mysqlpp::Query query = conn.query(_query);

		if (mysqlpp::StoreQueryResult res = query.store()){
		
//flooding scheme
		cout << "Enumerting near optimal paths" << endl;
		for (size_t i = 0; i < res.num_rows(); ++i) {
			//going through db and finding contact duration time for each contact and store them inside map table
			int stime = res[i][2];	//starting time
			int etime = res[i][3];	//end time
			int id1 = res[i][0];	//user1
			int id2 = res[i][1];	//user2
			int current_time = stime;	//emulation clock is updated

			//let's check if id1 is an internal node or not
			list<int>::iterator it;	//map list iterator 
			int T = 0;

			//let's check if ID1 is an external node or not
			if(cg.extM.find(id1) != cg.extM.end())	T = 1;
			//we only add edges for which ID1 is an internal node
			if(!T){	

			list<message>::iterator y, z, it1, it2, it3,it4;	// an iterator for the buffer link list
			list<int> delvMsgs;	//the list of delivered msgs id
			list<int>::iterator it5;
			delvMsgs.clear();	//we keep the list of delivered msgs is to remove from buffers later
			//first let's remove all expired msgs

			list<message> del_list;
			int delCnt = 0;
			if(((current_time - dUpdate) > maxD) && ((current_time - ts) <= TTL)){
//std::cout << "time: " << current_time << std::endl;
				for(int id = config->s_nid; id <= config->e_nid; id++){
				for(it1 = nodes[id-config->s_nid].buffer_list.begin(); it1 != nodes[id-config->s_nid].buffer_list.end(); ++it1){
				//delete all msgs which satify (ts - gen_t) > maxD
				if((stime - (*it1).gen_t) > maxD)
					del_list.push_back((*it1));	//to be deleted
				}
				delCnt += del_list.size();
				for(it2 = del_list.begin(); it2 != del_list.end(); ++it2)
				nodes[id-config->s_nid].buffer_list.remove((*it2));	//remove the expired msgs
				del_list.clear();	//clear the removed list
				}
				dUpdate = current_time;		//update the last update
	std::cout << "time: " << stime << ", deleted " << delCnt << " msgs!" << std::endl;
			}
			
			//now let's compare T1 and T2 to see what should we flood
			//read id1's and id2's buffers to see which messages should be copied to the ther one's buffer
			int flag1 = 0;	// to see if we should erase anything from the buffer of id1  
			//let's go through id1.buffer to see if there is any msg to be copied from ID1--> ID2
			//We are looking for T12 = T1-T2
			int copy = 0;	//we only copy one msg with the same src and dest
			for(z = nodes[id1-config->s_nid].buffer_list.begin(); z != nodes[id1-config->s_nid].buffer_list.end(); ++z){
//				if(copy == 1)	break;
				flag1 = 0;
				for(y = nodes[id2-config->s_nid].buffer_list.begin(); y != nodes[id2-config->s_nid].buffer_list.end(); ++y){		
					if ((*z).id == (*y).id){
						flag1 = 1;	//this message has already been found in id2's buffer
						break;		//we dont need to copy this message
					}
				}					
				if(flag1 == 0 && (*z).gen_t <= current_time){

					//T12 != null
					//we should copy the message from id1's buffer to id2's buffer
					if((current_time - (*z).gen_t <= TTL) && ((*z).delivered == false) && (current_time - (*z).gen_t <= maxD)){//check if msg has been expired
						//message has not been expired
						if(id1 != (*z).rid){//check if ID1 is the receiver of the message, if so then we dont need o copy the msg from ID1 to ID2!
						//core of routing scheme decision: Flooding
							//ID1 --> ID2: relay=ID2
							copy = 1;	//we did the copy --> jump to the next contact
							msg.id = (*z).id;
							msg.sid = (*z).sid;;
							msg.rid = (*z).rid;
							msg.gen_t = (*z).gen_t;
							msg.no_hops = (*z).no_hops + 1;

//let's read the path of (*z) and copy to a msg for the new relay node
   list<int>::iterator it_r;

   for ( it_r = (*z).relay_list.begin() ; it_r != (*z).relay_list.end(); it_r++ )
     msg.relay_list.push_back(*it_r);
     msg.relay_list.push_back(id2);
//log the time profile of relay time
   for ( it_r = (*z).time_list.begin() ; it_r != (*z).time_list.end(); it_r++ )
     msg.time_list.push_back(*it_r);
     msg.time_list.push_back(current_time);

if(id2 == (*z).rid)	msg.delivered = true;//this is the destination of msg, so we have found the shortest path for a triple of (src,dst,t)
else msg.delivered = false;

nodes[id2-config->s_nid].buffer_list.push_back(msg);	// copy the message to the end of the id2's buffer!

//if(id2 != (*z).rid)	msg.delivered = false;

//if(id2 != (*z).rid) nodes[id2-config->s_nid].buffer_list.push_back(msg);	// copy the message to the end of the id2's buffer!

							//let's check if ID2 is the msg.rcvr
							if(id2 == (*z).rid && (*z).gen_t <= stime){
delvMsgs.push_back((*z).id);	//this message has been delivered
(*z).delivered = true;	//stop delivering this msg to any other nodes
//we have reached the destination: the full path report
  list<int>::iterator it_r;
int no_relays = -1;
  path << "message " << (*z).id << " generated at " << (*z).gen_t << " delivered at " << stime << " by " << id1 << endl;
  path << "path:";
  for ( it_r = msg.relay_list.begin() ; it_r != msg.relay_list.end(); it_r++ ){
    paths[*it_r-config->s_nid]++;	//let's count the relay node in its corresponding place
    path << " " << *it_r;
    no_relays++;
  }
path << endl << "time: ";
  for ( it_r = msg.time_list.begin() ; it_r != msg.time_list.end(); it_r++ ){
    path << " " << *it_r;
  }

multiple trip;
trip.src = (*z).sid;
trip.dst = (*z).rid;
trip.time = *msg.time_list.begin();
trip.id = (*z).id;	//path id = msg id
trip.score =  (stime - *msg.time_list.begin() + 1)*no_relays;
//trip.score =  stime - *msg.time_list.begin();
//trip.score = no_relays;

triple three;
three.src = (*z).sid;
three.dst = (*z).rid;
three.time = *msg.time_list.begin();

if(mPaths.find(three) == mPaths.end()){
enumPaths.push_back(trip);
mPaths.insert( pair<triple,int>(three,1) );
}

path << endl << "relay no: " << no_relays << std::endl;
path << "delay: " << stime - *msg.time_list.begin();
path << endl << "D*Nhop: " << no_relays*(stime - *msg.time_list.begin());
path << endl << "--------------------------" << endl;
//}



							}
						msg.relay_list.clear();	//remove the path
						msg.time_list.clear();
						}//else since ID1 is the msg.rcvr --> NOP
					}
				}	
			}//end for


//since this msg is delivered, let's delete all msg with the same id from all nodes' buffer
for(int id=config->s_nid; id <= config->e_nid; id++){
//std::cout << "del msg: " << (*z).id << std::endl;
it2 = nodes[id-config->s_nid].buffer_list.end();
for(it1 = nodes[id-config->s_nid].buffer_list.begin(); it1 != nodes[id-config->s_nid].buffer_list.end(); ++it1){
for(it5 = delvMsgs.begin(); it5 != delvMsgs.end(); ++it5)
//delete all msgs with the same id as (*z).id
if((*it1).id == (*it5)){
	it2 = it1;	//to be deleted
	break;
}
}
if(it2 != nodes[id-config->s_nid].buffer_list.end()){
//std::cout << "time: " << stime << ", del 1!, msg:" << (*it2).id << "(" << (*it2).sid << "," << (*it2).rid << ") from " << id << "'s buffer" << std::endl;
nodes[id-config->s_nid].buffer_list.erase(it2);
}
//std::cout << "del 2!" << std::endl;
}

delvMsgs.clear();
			//read id2's and id1's buffers to see which messages should be copied to the ther one's buffer
			flag1 = 0;	// to see if we should erase anything from the buffer of id2  
			//let's go through id2.buffer to see if there is any msg to be copied from ID2--> ID1
			//We are looking for T21 = T2-T1
			copy = 0;	//we only copy one msg with the same src and dest
			for(z = nodes[id2-config->s_nid].buffer_list.begin(); z != nodes[id2-config->s_nid].buffer_list.end(); ++z){
//				if(copy == 1)	break;
				flag1 = 0;
				for(y = nodes[id1-config->s_nid].buffer_list.begin(); y != nodes[id1-config->s_nid].buffer_list.end(); ++y){		
					if ((*z).id == (*y).id){
						flag1 = 1;	//this message has already been copied to id2's buffer
						break;		//stop processing more
					}
				}

				if(flag1 == 0 && (*z).gen_t <= current_time){

					//T12 != null
					//we should copy the message from id1's buffer to id2's buffer
					if((current_time - (*z).gen_t <= TTL) && ((*z).delivered == false) && (current_time - (*z).gen_t <= maxD)){//check if msg has been expired
						//message has not been expired
						if(id2 != (*z).rid){//check if ID2 is the receiver of the message, if so then we dont need o copy the msg from ID1 to ID2!
						//core of routing scheme decision: Flooding
							
							copy = 1;
							msg.id = (*z).id;
							msg.sid = (*z).sid;;
							msg.rid = (*z).rid;
							msg.gen_t = (*z).gen_t;
							msg.no_hops = (*z).no_hops + 1;
							//ID2 --> ID1: relay = ID1
//let's read the path of (*z) and copy to a msg for the new relay node							
  list<int>::iterator it_r;

  for ( it_r = (*z).relay_list.begin() ; it_r != (*z).relay_list.end(); it_r++ )
    msg.relay_list.push_back(*it_r);
    //add id1 to the end o relay nodes list
    msg.relay_list.push_back(id1);

  for ( it_r = (*z).time_list.begin() ; it_r != (*z).time_list.end(); it_r++ )
    msg.time_list.push_back(*it_r);
    //add current time to the end of the list
    msg.time_list.push_back(current_time);

if(id1 == (*z).rid)	msg.delivered = true;//this is the destination of msg, so we have found the shortest path for triple (src,dst,t)
else msg.delivered = false;
// 
nodes[id1-config->s_nid].buffer_list.push_back(msg);	// copy the message to the end of the id1's buffer!

//if(id1 != (*z).rid)	msg.delivered = false;

//if(id1 != (*z).rid)	nodes[id1-config->s_nid].buffer_list.push_back(msg);	// copy the message to the end of the id1's buffer!

							//let's check if ID1 is the msg.rcvr
							if(id1 == (*z).rid && (*z).gen_t <= stime){
delvMsgs.push_back((*z).id);	//this message has been delivered
(*z).delivered = true;	//stop forwarding this msg to any other node
//we have reached the destination: the full path report
  list<int>::iterator it_r;
int no_relays = -1;
path << "message " << (*z).id << " generated at " << (*z).gen_t << " delivered at " << stime << " by " << id2 << endl;
  path << "path:";
  for ( it_r = msg.relay_list.begin() ; it_r != msg.relay_list.end(); it_r++ ){
    //we are incrementing the no every node appears in optimal path
    paths[*it_r-config->s_nid]++;	//let's count the relay node in its corresponding place
    path << " " << *it_r;
    no_relays++;
  }

path << endl << "time: ";
  for ( it_r = msg.time_list.begin() ; it_r != msg.time_list.end(); it_r++ ){
    path << " " << *it_r;
  }

multiple trip;
trip.src = (*z).sid;
trip.dst = (*z).rid;
trip.time = *msg.time_list.begin();
trip.id = (*z).id;	//this is msg id which is equivalent to path id: unique
trip.score =  (stime - *msg.time_list.begin() + 1)*no_relays;
//trip.score =  stime - *msg.time_list.begin();
//trip.score =  no_relays;

triple three;
three.src = (*z).sid;
three.dst = (*z).rid;
three.time = *msg.time_list.begin();

if(mPaths.find(three) == mPaths.end()){
enumPaths.push_back(trip);
mPaths.insert( pair<triple,int>(three,1) );
}

path << endl << "relay no: " << no_relays << std::endl;
path << "delay: " << stime - *msg.time_list.begin();
path << endl << "D*Nhop: " << no_relays*(stime - *msg.time_list.begin());
path << endl << "--------------------------" << endl;
//}


							}
						msg.relay_list.clear();	//remove the path
						msg.time_list.clear();
						}//else since ID1 is the msg.rcvr --> NOP
					}
				}	
			}//end for
//since this msg is delivered, let's delete all msg with the same id from all nodes' buffer
for(int id=config->s_nid; id <= config->e_nid; id++){
//std::cout << "del msg: " << (*z).id << std::endl;
it2 = nodes[id-config->s_nid].buffer_list.end();
for(it1 = nodes[id-config->s_nid].buffer_list.begin(); it1 != nodes[id-config->s_nid].buffer_list.end(); ++it1){
for(it5 = delvMsgs.begin(); it5 != delvMsgs.end(); ++it5)
//delete all msgs with the same id as (*z).id
if((*it1).id == (*it5)){
	it2 = it1;	//to be deleted
	break;
}
}
if(it2 != nodes[id-config->s_nid].buffer_list.end()){
//std::cout << "time: " << stime << ", del 1!, msg:" << (*it2).id << "(" << (*it2).sid << "," << (*it2).rid << ") from " << id << "'s buffer" << std::endl;
nodes[id-config->s_nid].buffer_list.erase(it2);
}
//std::cout << "del 2!" << std::endl;
}

delvMsgs.clear();

		}//end if
		}//end for

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

//sort the total list
enumPaths.sort();

path << "sorted version:" << std::endl;
for ( it = enumPaths.begin() ; it != enumPaths.end(); it++ )
	path << "(" << (*it).src << "," << (*it).dst << ")=(" << (*it).id << "," << (*it).time << "," << (*it).score << ")" << std::endl;

//Evaluation of our prediction method
int num = 1018;	//no of predictions
int i = 0;
int matched = 0;
path << "Evaluating the merged list" << std::endl;
for ( it = enumPaths.begin() ; it != enumPaths.end(); it++ ){
if(cg.IsContact((*it).src, (*it).dst, (*it).time - 120,  (*it).time + 120)){
	path << "(" << (*it).src << "," << (*it).dst << ")=(" << (*it).id << "," << (*it).time << "," << (*it).score << ") is matched!" << std::endl;
	matched++;
	}
	i++;
	if(i == num) break;
}

path << "prediction --> no of matched contacts: " << matched << std::endl;

path.close();
	return 0;
}
