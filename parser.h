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

#ifndef PARSER_H
#define PARSER_H

#include <mysql++.h>
#include "configuration.h"

class parser{
	int x, y;
  public:
	configuration *config;	//an configuration object
	int GetX() { return x; }
	int GetY() { return y; }
	void set_values (int a, int b){x=a;y=b;};
	int parse_contact_data(const char *);	//parse data from the "contacts.Exp6.dat" file into db
	int parse_synth_contact_data(const char *);	//to parse synthetic contact data into mysql
	mysqlpp::Connection connect_to_db();	//connect to db
	int parse_interest_data();		//parse data from interest files into 
	int generate_traces(int, int, unsigned char, int);			//this method generate the trcaes files for no of contact and contac duration as well as intereset intersection for all possible pairs. then we can use this information to calculate different things,
};
#endif
