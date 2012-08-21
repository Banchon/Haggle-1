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
#include "triple.h"

triple::triple()   // Constructor
{
	src = 0;
	dst = 0;	
	time = 0;
}

triple& triple::operator=(const triple &rhs)
{
   this->src = rhs.src;
   this->dst = rhs.dst;
   this->time = rhs.time;
   return *this;
}

int triple::operator==(const triple &rhs) const
{

   if( this->src != rhs.src) return 0;
   if( this->dst != rhs.dst) return 0;
   if( this->time != rhs.time) return 0;

   return 1;
}

int triple::operator<(const triple &rhs) const
{
if( this->time < rhs.time ) return 1;
return 0;
}
