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
  QueryCrypto(std::string curPair, int id, std::string queryType, int delay) : _curPair(curPair), _id(id), _queryType(queryType), _http_delay(delay) {};

  // Making http request
  void getData(); 
  void getAllData(); 
  void runSingleQueries(); 
  void runWalletQuery(); 
  void getRequest(std::string *readBuffer, long *http_code, std::map<std::string, std::string> *hDictionary, bool wallet); 

  // Data processing
  void saveCSVData(std::string &readBuffer); 
  void saveAllCoinsCSVData(std::string &readBuffer); 
  void saveData(std::vector<std::vector<std::string>> &chunkData, std::vector<std::vector<std::vector<std::string>>> *allPlotData); 
  std::map<std::string, std::string> parseHeaderData(std::string &readBuffer); 
  void formatData(std::string &readBuffer, std::map<std::string, double> *coinToQuantity); 

  // setters
  void setCoinsToPlot(std::vector<std::string> coinsToPlot); 
  void setAllPlotData(std::vector<std::vector<std::vector<std::string>>> &plotData) { _plotData = plotData; }
  std::vector<std::vector<std::vector<std::string>>> getAllPlotData() {return _plotData; }
  std::vector<std::string> getCoinsToPlot(); 
  void addRequestWeight(std::string requestWeight); 
  void setWindowRange(int &windowRange);
  void setWalletStatus(bool wallet); 
  std::map<std::string, double> getCoinToQuantity(); 
  std::map<std::string, double> getCoinToPrice(); 
  void setCoinToPrice(std::string file_name, std::string coin); 
  void updateCoinToPrice(std::string price, std::string coin); 

  // getters
  int getID() { return _id; }
  int getCurrentWeightRequest(); 
  int getWindowRange(); 
  std::string getCoinPair(){ return _curPair; }
  std::string getCoinToPlot();
  std::string getQueryType() { return _queryType; }
  void parsePlotData(std::string file_name, std::vector<std::vector<std::vector<std::string>>> *plotData, ptrdiff_t pos); 
  bool isWalletEnabled(); 

private:
  std::string _curPair;                                         // crypto currency pair i.e., BTC/USD
  std::string _currentWeight;                                   // Get info about current interval weight
  std::vector<std::vector<std::vector<std::string>>> _plotData; // historic data for each coin to
  std::string _cointToPlot;                                     // Coin to be plotted
  std::vector<std::string> _coinsToPlot;                        // Vector containing all coins to plot
  int _id; 
  int _windowRange;                                              // Window of data to fetch from each coin
  std::string _queryType; 
  int _http_delay;                                              // HTTP request cylce in seconds
  std::mutex _mutex;                                            // Lock certain shared resources
  static std::vector<int> _requestWeight;                       // Vector containing current request weight for given interval
  bool _wallet; 
  static std::map<std::string, double> _coinToQuantity;
  static std::map<std::string, double> _coinToPrice;  
};

#endif