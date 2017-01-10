/*
Author: Charlie Summerscales
Course: CMPSC 463 - Algorithms
Purpose: Final Assignment - Pandora Listening Enjoyment
Date: 04/19/2016
*/

#include <stdlib.h>
#include <iostream>
#include <fstream>
#include <string>
#include <algorithm>
#include <limits.h>

using std::string;
using std::cout;
using std::endl;

//Ugly Global Variable Section to make life easier
int n = 0;
int memo[140][140][3][100]; //memoized array
int s1[140] = { 0 }; //station 1 song list
int s2[140] = { 0 }; //station 2 song list

int listen(int i, int j, int k, int m) {
	using std::max;

	if (k < 0) k = 0;

	//check for a memoized result
	if (memo[i][j][k][m] != INT_MIN)
		return memo[i][j][k][m];

	int result = 0;

	if (k <= 0 && m == n - 1) //base case 1
		result = max(s1[i], max(s2[j], max(s1[i + 1], s2[j + 1])));
	if (k > 0 && m == n - 1) //base case 2
		result = max(s1[i], s2[j]);
	if (k <= 0 && m < n - 1) //regular case with skip available
		result = max(s1[i] + listen(i + 1, j, k - 1, m + 1), max(s2[j] + listen(i, j + 1, k - 1, m + 1), max(s1[i + 1] + listen(i + 2, j, 2, m + 1), s2[j + 1] + listen(i, j + 2, 2, m + 1))));
	if (k > 0 && m < n - 1) //regular case with no skip available
		result = max(s1[i] + listen(i + 1, j, k - 1, m + 1), s2[j] + listen(i, j + 1, k - 1, m + 1));

	//store result in memo
	memo[i][j][k][m] = result;

	return result;
}

int main() {
	using std::getline;
	using std::ifstream;
	using std::cin;

	string stationlist;
	string fakeN;

	//Initialize the memo array
	for (int i = 0; i < 140; i++)
	{
		for (int j = 0; j < 140; j++)
		{
			for (int k = 0; k < 3; k++)
			{
				for (int m = 0; m < 100; m++)
				{
					memo[i][j][k][m] = INT_MIN;
				}
			}
		}
	}
	//Most of this is simply reading in the file input
	//and assigning it to their respective arrays.
	cin.sync_with_stdio(false);

	getline(cin, fakeN);
	n = atoi(fakeN.c_str());
	getline(cin, stationlist);
	string songs = stationlist;
	string song;
	int position = 0;
	int i = 0;

	//station list 1 songs
	do
	{
		position = songs.find(' ');
		if (position >= 0)
		{
			song = songs.substr(0, position);
			position = song.length();
			songs = songs.substr(position + 1);
			s1[i] = atoi(song.c_str());
			i++;
		}
		else
		{
			song = songs;
			s1[i] = atoi(song.c_str());
		}
	} while (i < 140 && position > 0);

	getline(cin, stationlist);
	songs = stationlist;
	i = 0;
	//station list 2 songs
	do
	{
		position = songs.find(' ');
		if (position >= 0)
		{
			song = songs.substr(0, position);
			position = song.length();
			songs = songs.substr(position + 1);
			s2[i] = atoi(song.c_str());
			i++;
		}
		else
		{
			song = songs;
			s2[i] = atoi(song.c_str());
		}
	} while (i < 140 && position > 0);

	//call the listen function
	int solution = listen(0, 0, 0, 0);

	//output the result
	cout << solution << endl;

	return 0;
}
