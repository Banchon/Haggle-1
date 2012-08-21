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
#include "statistics.h"
#include <gsl/gsl_statistics.h>
#include <fstream> // file stream        
#include <iostream>
#include <cstdlib>
#include <string.h>
#include <vector>

using std::ifstream; // input file stream
using namespace std;
//this method returns the number of rows in the file
int statistics::rows_no(const char* file){
  ifstream inClientFile( file, ios::in );

  // exit program if ifstream could not open file
  if ( !inClientFile )
  {
	cerr << "File could not be opened" << endl;
        return -1;
   } // end if

  char str[256];
  int length = 0;

  // get length of file:
  while( inClientFile.getline( str, 256 ) ) length++;

  setrows(length);
  return 0;	//successful
}

int statistics::meanvar(const char* file, int col){
  ifstream inClientFile( file, ios::in );

  // exit program if ifstream could not open file
  if ( !inClientFile )
  {
	cerr << "File could not be opened" << endl;
        return NULL;
   } // end if


  char *x;
  char str[256];
  double *y, temp;
  int i = 0, length = 0, j = 0;

  // get length of file:
  length = getrows(); 

  // allocate memory:
  y = (double *) malloc(sizeof(double)*length);
  bzero(str, 255);

  //read data from the file line by line and parse only the desired column
  while ( inClientFile.getline(str, 256) ){
	temp = 0.0;
	j = 1;
	x = strtok (str," ");
	//we are interested in data from column col, so we only parse the corresponding column data
        while (x != NULL and j < col)
  	{
    		x = strtok (NULL, " ");
		temp = atof(x);
		j++;
  	}
	//cout << x << endl;
        if (temp != 0){
		y[i] = temp;
//		cout << y[i] << endl;
		i++;
	}
	bzero(str, 255);
  }
  //let's set mean and sd for this variable
  setmean(gsl_stats_mean(y, 1, length));
  setsd(gsl_stats_sd(y, 1, length));
  //cout << gsl_stats_mean(y, 1, i) << endl;
inClientFile.close();
  return 0;
}

double statistics::corr(const char* file, int col1, int col2){
  ifstream inClientFile( file, ios::in );

  // exit program if ifstream could not open file
  if ( !inClientFile )
  {
	cerr << "File could not be opened" << endl;
        return NULL;
   } // end if
  char *x;
  char str[256];
  double *y1, *y2;	//let's read the corresponding columns
  int i = 0, length = 0, j = 0;


  // get length of file:
  length = getrows();

  // allocate memory:
  y1 = (double *) malloc(sizeof(double)*length);
  y2 = (double *) malloc(sizeof(double)*length);
  double temp1, temp2;
  bzero(str, 255);

  //read data from the file line by line and parse only the desired column
  while ( inClientFile.getline(str, 256) ){
	temp1 = 0.0;
	temp2 = 0.0;
	j = 1;
	x = strtok (str," ");
	//we are interested in calculating the correlation between column col1 and col2,
	// so we only parse the corresponding columns data
        while (x != NULL)
  	{
		if(j == col1){
			temp1 = atof(x);
//			cout << "column " << col1 << ": " <<  y1[i]; 
		}
		else if(j == col2){
			temp2 = atof(x);
//			cout << ", column " << col2 << ": " << y2[i] <<endl; 
		}
		x = strtok (NULL, " ");
		j++;
  	}
	//cout << x << endl;
	bzero(str, 255);
	if(temp1 != 0 && temp2 != 0){	
		//we record only if the values are not zero
		y1[i] = temp1;
		y2[i] = temp2;
		i++;
	}
  }
  //let's set mean and sd for this variable
  //setmean(gsl_stats_mean(y, 1, length));
  //setsd(gsl_stats_sd(y, 1, length));
//  cout << gsl_stats_mean(y1, 1, i) << endl;
/*for(int i=0; i < length; i++)
cout << y1[i] << " " << y2[i] << endl;*/
  double corr = 0;
  corr = gsl_stats_correlation(y1, 1, y2, 1, length);

  free(y1);
  free(y2);
  inClientFile.close();

  return corr;
}
