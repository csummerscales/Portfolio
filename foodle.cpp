/*
  Author: Charlie Summerscales
  Course: COMP 597 Data Mining
  Purpose: K-NN recipe classifier
  Date: 03/27/2015
*/

#include <iostream>
#include <sstream>
#include <fstream>
#include <vector>
#include <string>
#include <math.h>
#include <stdlib.h>
#include <algorithm>
#include <cstring>
#include <unordered_map>

using std::string;
using std::vector;
using std::cout;
using std::endl;

struct Neighbor {
  int classLabel;
  double distance;

  Neighbor(int c, double d) : classLabel(c), distance(d) {}
  Neighbor(const Neighbor& n) : classLabel(n.classLabel),distance(n.distance) {}
};

inline unsigned int hasher (string str, int magicNumber) {
  unsigned long hashValue = 0;
  char hWord[str.length()];
  strcpy(hWord, str.c_str());

  for (int i = 0; i < str.length(); i++) 
    {
      hashValue = hashValue * 31 + (unsigned long)hWord[i];
    }
  return (unsigned int)(hashValue%(unsigned long)magicNumber);
}

void featureVectorize(string &str, int n, vector<double>& features, std::unordered_map<string,int>& weightWords, int cl){
  int pos = 0;
  do
    {
      pos = str.find(' '); 
      string word;
      if (pos < 0) word = str;
      else 
	{
	  word = str.substr(0, pos);
	  pos = word.length();
	  str = str.substr(pos+1);
	}
      int hashedWord = hasher(word.c_str(),n);
      if (weightWords.find(word) == weightWords.end()) features[hashedWord]++;
      else 
	{
	  if (cl == 0) features[hashedWord] += 2;
	  else if (cl == 1 && word == "cream") features[hashedWord] +=2;
	  else if (cl == 2 && word == "leaf") features[hashedWord] +=2;
	  else if (cl == 3 && word == "coriander") features[hashedWord] +=2;
	  else if (cl == 4 && word == "sesame") features[hashedWord] +=2;
	  else if (cl == 5 && word == "lime") features[hashedWord] +=2;
	  else if (cl == 6 && word == "feta") features[hashedWord] +=2;
	  else if (cl == 7 && word == "tortilla") features[hashedWord] +=2;
	  else features[hashedWord] += 1;
	}
    }while(pos > 0);
}

double distanceMetric(vector<double>& a, vector<double>& b) {
  double sum = 0;
  double sumA = 0;
  double sumB = 0;
  for (int i = 0; i < a.size(); i++)
    {
      sum += a[i]*b[i];
      sumA += pow(a[i], 2.0);
      sumB += pow(b[i], 2.0);
    }
  double cosineD;
  if(sum == sumA && sum == sumB) cosineD = 1;
  else cosineD = sum/(sqrt(sumA)*sqrt(sumB)); 
  return 1.0-cosineD;
}
 
bool ncomp (const Neighbor& nbr1, const Neighbor& nbr2) {
  return nbr1.distance < nbr2.distance;
}
int main () {
  //Using Declarations
  using std::getline;
  using std::ifstream;
  using std::cin;

  vector<string> trnRows;
  vector<string> tstRows;
  vector<int> trueValues(21667);  
  vector<int> predValues(21667);
  string tstRow;
  
  std::unordered_map<std::string, int> badWords ({
      {"salt",0},
	{"fat",0},
	  {"purpose",0},
	    {"low",0},
	      {"no",0},
		{"all",0}});
  std::unordered_map<std::string, int> weights ({
      {"cream",1},
	{"sesame",4},
	  {"coriander",3},
	    {"lime",5},
	      {"feta",6},
		{"tortilla",7},
		  {"leaf",2}});

  //read in test data
  int testSize = 0;
  cin.sync_with_stdio(false);
  while(!cin.eof())
    {
      getline(cin, tstRow);      
      int oldPos = 0, newPos = 0;
      string word;
      string str = tstRow;
      tstRow = "";
      int pos = 0;
      do
	{
	  pos = str.find(' ');
	  if (pos < 0) //end of the line
	    {
	      word = str;
	      if (badWords.find(word) == badWords.end()) tstRow += word;
	    }
	  else //not end of the line
	    {
	      word = str.substr(0, pos);	      
	      pos = word.length(); //move position up
	      str = str.substr(pos+1);	    
	      if (badWords.find(word) == badWords.end()) tstRow += word + " ";
	    }
	}while(pos > 0);
      tstRows.push_back(tstRow);
      testSize++;
    }
    testSize--;

  ifstream file;
  string row;
  int nbor = 13, hashNumber = 128;
  int correct = 0;
  file.open("training-data.txt");
  
  //read in all the recipes from training data
  for (int i = 0; !file.eof(); i++)
    {
      getline(file, row);
      int oldPos = 0, newPos = 0;
      string word;
      string str = row;
      row = "";
      int pos = 0;
      bool classV = true;
      char cVal[2];
      do
	{
	  pos = str.find(' ');
	  if (pos < 0) //end of the line
	    {
	      word = str;
	      if (badWords.find(word) == badWords.end()) row += word;
	    }
	  else //not end of the line
	    {
	      word = str.substr(0, pos);
	      if (classV) //remove class value and store it
		{
		  classV = false; //set flag to wait until next line
		  strcpy(cVal, word.c_str());
		  trueValues[i] = (atoi(cVal));
		}
	      pos = word.length(); //move position up
	      str = str.substr(pos+1);	    
	      if (badWords.find(word) == badWords.end()) row += word + " ";
	    }
	}while(pos > 0);
      trnRows.push_back(row);
    }

  //drop junk line
  trnRows.erase(trnRows.end());  
  int original = trnRows.size();

  //calculate distance between training and test data
#pragma omp parallel for
  for (int i = 0; i < testSize; i++)
    {      
      vector<Neighbor> neighbors;
      neighbors.reserve(nbor);
      //for test subsets only
      string test = tstRows[i];		
      vector<double> testFeatures(hashNumber);
      featureVectorize(test, hashNumber, testFeatures, weights, 0);
      
      for (int j = 0; j < original; j++)
	{	    
	  string train = trnRows[j];
	  vector<double> trainFeatures(hashNumber);
	  featureVectorize(train, hashNumber, trainFeatures, weights, trueValues[j]);

	  //calculate distance
	  double dist = distanceMetric(testFeatures,trainFeatures);

	  //sorts current distances in max descending
	  sort(neighbors.begin(), neighbors.end(), ncomp);

	  //store distances
	  if (neighbors.size() < nbor) //fill up neighbors vector first
	    {
	      Neighbor newNearest(trueValues[j], dist);
	      neighbors.push_back(newNearest);
	    }
	  else if (dist < neighbors[nbor-1].distance) //if distance < max neighbor distance, replace
	    {
	      Neighbor newNearest(trueValues[j], dist);
	      neighbors[nbor-1] = newNearest;
	    }	    
	}

      //determine classification
      vector<double> classes(7);
      for (int k = 0; k < neighbors.size(); k++)
	{
	  double weight;
	  if (neighbors[k].distance == 0) weight = 100;
	  else weight = ((neighbors[neighbors.size()-1].distance-neighbors[k].distance)/(neighbors[neighbors.size()-1].distance-neighbors[0].distance)); 
	  classes[(neighbors[k].classLabel)-1] += weight;
	}

      //store winning class
      int classifiedVal = std::distance(classes.begin(),std::max_element(classes.begin(), classes.end()));
      predValues[i] = (classifiedVal+1);
      
    }
  for (int i = 0; i < testSize; i++) cout << predValues[i] << endl;
}
