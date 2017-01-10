/*
Author: Charlie Summerscales
Purpose: Restaurant
Date: 05/13/2016
*/

#include <stdlib.h>
#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <vector>
#include <algorithm>
#include <limits.h>

using std::string;
using std::cout;
using std::endl;
using std::vector;

int gcd(int a, int b) {
  
  while (a != b) {
    if (a > b)
      a -= b;
    else
      b -= a;
  }
  return b;
}

int main() {
	using std::getline;
	using std::ifstream;
	using std::cin;

	string numOfPoints;
	string temp;
	int newx, newy;

	//Most of this is simply reading in the file input
	//and assigning it to their respective arrays.
	cin.sync_with_stdio(false);
	getline(cin, numOfPoints);
	int n = atoi(numOfPoints.c_str());

	while (getline(cin, temp))
	  {
	    std::stringstream ss(temp); //stringstream to hold the current line of input
	    vector<int> points;
	    int num; //temporary integer before we store it
	    while (ss >> num)
	      {
		points.push_back(num);
	      }    
	    int minDimension = std::min(points[0], points[1]);
	    int goal = points[0] * points[1];
	    int maxSquares = 0;
	    int maxSide = 0;

	    for (int i = 1; i <= minDimension; i++)
	      {
		int testSquare = i*i;
		int checkForSpaceLeft = goal % testSquare;
		if (checkForSpaceLeft == 0)
		  {
		    if (points[0]%i == 0 && points[1]%i == 0)
		    maxSquares = goal / testSquare;
		  }
		//cout << "i: " << i << " testSquare: " << testSquare << endl;
	      }
	    cout << maxSquares << endl;
	  }

	return 0;
}
