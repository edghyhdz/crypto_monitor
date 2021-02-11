#ifndef CRYPTO_H
#define CRYPTO_H

#include <string>
#include <iostream>
#include <map>
#include <vector>

/*
Crypto currency query class declaration
*/
class QueryCrypto {
public:
  // Currency pair
  QueryCrypto(std::string curPair, int id, bool allCoins) : _curPair(curPair), _id(id), _allCoins(allCoins) {};

  // Making http request
  void getData(); 
  void getAllData(); 
  void runSingleQueries(); 
  // Data processing
  void saveCSVData(std::string &readBuffer); 
  void saveAllCoinsCSVData(std::string &readBuffer); 
  void saveData(std::vector<std::vector<std::string>> &chunkData); 
  std::map<std::string, std::string> parseHeaderData(std::string &readBuffer); 

  // getters
  std::string getCoinPair(){ return _curPair; }
  bool allQueries(){ return _allCoins; }

  void printCoinPair() { std::cout << "Coin: " << _curPair << std::endl; }; 

private:
  std::string _curPair;                 // crypto currency pair i.e., BTC/USD
  std::string _currentWeight;           // Get info about current interval weight
  int _id; 
  bool _allCoins; 

};

#endif