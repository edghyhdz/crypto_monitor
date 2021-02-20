#ifndef ORCHESTRATOR_H
#define ORCHESTRATOR_H

#include <memory>
#include <thread>
#include <vector>
#include <map>

/*
Class declaration Orchestrator
This class will deal with all QueryCrypto threads as well
as managing their current request weights to avoid IP bans

TODO: Other methods or so
*/

// Forward declaration to avoid include cycle
class QueryCrypto;

class Orchestrator {
public:
Orchestrator(bool wallet); 
~Orchestrator(); 
// Reference: https://stackoverflow.com/a/23575458/13743493
Orchestrator(Orchestrator &&o) = default;

// std::vector<std::vector<std::string>> getPlotData(); 
// void setCoinToPlot(std::string coinToPlot);
void setCoinsToPlot(std::vector<std::string> coinsToPlot);
std::vector<std::vector<std::vector<std::string>>> getAllPlotData(); 
void setWindowRange(int &windowRange); 
void setWalletStatus(bool wallet); 
void runWalletQuery(); 
std::map<std::string, double> getCoinToQuantity(); 
std::map<std::string, double> getCoinToPrice(); 

// std::string getCoinToPlot();
std::vector<std::string> getCoinsToPlot(); 
int getCurrentWeightRequest(); 
bool isWalletEnabled(); 

void runQuery(); 

private:
  std::vector<std::shared_ptr<QueryCrypto>> _cryptos;
  std::shared_ptr<QueryCrypto> _allCryptos;
  std::shared_ptr<QueryCrypto> _walletCryptos;  
  std::vector<std::thread> _threads;
  std::vector<std::thread> _threadCryptos;
  std::string _currentDirectory; 
  bool _wallet; 
};

#endif