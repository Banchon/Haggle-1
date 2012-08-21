/***************************************************************************
 *   Copyright (C) 2011 by Kazem   *
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

#ifndef FEATURE_H
#define FEATURE_H

#include <list>
#include <iostream>
#include <cstdlib>
#include <fstream> // file stream        
#include <list>
#include "contactG.h"
#include <math.h>

using std::ifstream; // input file stream

using namespace std;

class feature
{
public:
	feature();
	~feature(){};

	pair <int,int> nodes_pair;	//a apir represents the nodes that we are interested in extracting their features
	int first_deg;			//the first node degree in CG
	int sec_deg;			//the second node degree in G
	int deg_prod;			//degree product
	int ncn;			//ncn	
	int edge;			//1 if there is an edge otherwise 0
	feature &operator=(const feature &rhs);
private:
};
#endif
