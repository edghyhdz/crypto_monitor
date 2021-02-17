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
  // void saveData(std::vector<std::vector<std::string>> &chunkData); 
  void saveData(std::vector<std::vector<std::string>> &chunkData, std::vector<std::vector<std::vector<std::string>>> *allPlotData); 
  std::map<std::string, std::string> parseHeaderData(std::string &readBuffer); 

  // setters
  void setCoinsToPlot(std::vector<std::string> coinsToPlot); 
  void setAllPlotData(std::vector<std::vector<std::vector<std::string>>> &plotData) { _plotData = plotData; }
  std::vector<std::vector<std::vector<std::string>>> getAllPlotData() {return _plotData; }
  std::vector<std::string> getCoinsToPlot(); 

  void addRequestWeight(std::string requestWeight); 
  // getters
  int getID() { return _id; }
  int getCurrentWeightRequest(); 
  std::string getCoinPair(){ return _curPair; }
  std::string getCoinToPlot();
  bool allQueries(){ return _allCoins; }
  void parsePlotData(std::string file_name, std::vector<std::vector<std::vector<std::string>>> *plotData, ptrdiff_t pos); 

private:
  std::string _curPair;                                         // crypto currency pair i.e., BTC/USD
  std::string _currentWeight;                                   // Get info about current interval weight
  std::vector<std::vector<std::vector<std::string>>> _plotData; // historic data for each coin to plot
  std::string _cointToPlot;                                     // Coin to be plotted
  std::vector<std::string> _coinsToPlot;                        // Vector containing all coins to plot
  int _id; 
  bool _allCoins; 
  int _http_delay;                                              // HTTP request cylce in seconds
  std::mutex _mutex;                                            // Lock certain shared resources
  static std::vector<int> _requestWeight;                       // Vector containing current request weight for given interval
};

#endif