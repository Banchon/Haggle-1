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
#ifndef INTERESTS_H
#define INTERESTS_H

#include <ssqls.h>

sql_create_13(
  interests, 1, 13,
  mysqlpp::sql_char, name,
  mysqlpp::sql_char, email,
  mysqlpp::sql_char, nationality,
  mysqlpp::sql_char, studies,
  mysqlpp::sql_char, languages,
  mysqlpp::sql_char, affiliation,
  mysqlpp::sql_char, position,
  mysqlpp::sql_char, city,
  mysqlpp::sql_char, country,
  mysqlpp::sql_char, airports,
  mysqlpp::sql_char, stay,
  mysqlpp::sql_char, member,
  mysqlpp::sql_char, topics
);

class Derived_ : public interests
{
public:
  // constructor
  Derived_(  mysqlpp::sql_char _name, mysqlpp::sql_char _email, mysqlpp::sql_char _nat,
	    mysqlpp::sql_char _stud, mysqlpp::sql_char _lang, mysqlpp::sql_char _affil,
  	    mysqlpp::sql_char _pos,  mysqlpp::sql_char _city, mysqlpp::sql_char _count,
  	    mysqlpp::sql_char _airp, mysqlpp::sql_char _stay, mysqlpp::sql_char _mem,
  	    mysqlpp::sql_char _tops) :
  interests(_name, _email, _nat, _stud, _lang, _affil, _pos, _city, _count, _airp, _stay, _mem, _tops)
  {
  }

  // functionality added to the SSQLS through inheritance
  bool do_something_interesting(int data);
};
#endif
