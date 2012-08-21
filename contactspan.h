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
#ifndef CONTACTSPAN_H
#define CONTACTSPAN_H

#include <ssqls.h>

sql_create_6(
  contactspan, 1, 6,
  mysqlpp::sql_int, user1,
  mysqlpp::sql_int, user2,
  mysqlpp::sql_int, starttime,
  mysqlpp::sql_int, endtime,
  mysqlpp::sql_int, enumerator,
  mysqlpp::sql_int, intercontact
);

class Derived : public contactspan
{
public:
  // constructor
  Derived(mysqlpp::sql_int _id1, mysqlpp::sql_int _id2, mysqlpp::sql_int _st, 									mysqlpp::sql_int _et, mysqlpp::sql_int _en, mysqlpp::sql_int _cd) :
  contactspan(_id1, _id2, _st, _et, _en, _cd)
  {
  }

  // functionality added to the SSQLS through inheritance
  bool do_something_interesting(int data);
};
#endif
