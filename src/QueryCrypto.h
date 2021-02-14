#ifndef CRYPTO_H
#define CRYPTO_H
#define ALL_COINS_ID 205  // Generic all coins ID
#include <string>
#include <iostream>
#include <map>
#include <vector>
#include <mutex>


/*
Crypto currency query class declaration
*/
class QueryCrypto {
public:
  // Currency pair
  QueryCrypto(std::string curPair, int id, bool allCoins, int delay) : _curPair(curPair), _id(id), _allCoins(allCoins), _http_delay(delay) {};

  // Making http request
  void getData(); 
  void getAllData(); 
  void runSingleQueries(); 
  // Data processing
  void saveCSVData(std::string &readBuffer); 
  void saveAllCoinsCSVData(std::string &readBuffer); 
  void saveData(std::vector<std::vector<std::string>> &chunkData); 
  std::map<std::string, std::string> parseHeaderData(std::string &readBuffer); 

  // setters
  void setCoinToPlot(std::string coinToPlot);
  void setPlotData(std::vector<std::vector<std::string>> &plotData) { _plotData = plotData; }
  // getters
  int getID() { return _id; }
  std::vector<std::vector<std::string>> getPlotData() { return _plotData; }
  std::string getCoinPair(){ return _curPair; }
  std::string getCoinToPlot();
  bool allQueries(){ return _allCoins; }

private:
  std::string _curPair;                               // crypto currency pair i.e., BTC/USD
  std::string _currentWeight;                         // Get info about current interval weight
  std::vector<std::vector<std::string>> _plotData;    // Data to plot into dashboard
  std::string _cointToPlot;                    // Coin to be plotted
  int _id; 
  bool _allCoins; 
  int _http_delay;                                    // HTTP request cylce in seconds
  std::mutex _mutex; 

};

#endif