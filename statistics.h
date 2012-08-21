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
#ifndef STATISTICS_H
#define STATISTICS_H

class statistics
{
public:
	//calculate the mean of col_1 column of the file
	int meanvar(const char*, int);
	double sd(const char*, int);
	double cov(const char*, int, int);
	double corr(const char*, int, int);
	void setmean(double m){meanval = m;}
	double getmean(){return meanval;}
	double getsd(){return sdval;}
	void setsd(double sd){sdval = sd;}
	void setrows(int rown){rows = rown;}
	int getrows(){return rows;}
	int rows_no(const char*);	//reads a file and returns no of rows
private:
	double meanval;	//mean value
	double sdval;	//standard deviation
	int rows;	//no of rows for data: no of samples
};
#endif

