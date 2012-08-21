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
#include "feature.h"

using namespace std;

feature::feature()   // Constructor
{
	nodes_pair = make_pair (0, 0);	//a apir represents the nodes that we are interested in extracting their features
	first_deg = 0;			//the first node degree in CG
	sec_deg = 0;			//the second node degree in G
	deg_prod = 0;			//degree product
	ncn = 0;
}

feature& feature::operator=(const feature &rhs)
{
   this->nodes_pair.first = rhs.nodes_pair.first;
   this->nodes_pair.second = rhs.nodes_pair.second;
   this->first_deg = rhs.first_deg;
   this->sec_deg = rhs.sec_deg;
   this->deg_prod = rhs.deg_prod;
   this->ncn = rhs.ncn;
   return *this;
}
