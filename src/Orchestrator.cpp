#include "Orchestrator.h"
#include "QueryCrypto.h"
#include <algorithm>
#include <chrono>
#include <future>
#include <random>
#include <thread>


/*
Orchestrator member attribute declarations

g++ -g src/QueryCrypto.cpp src/QueryCrypto.h src/Binance.h src/Orchestrator.cpp src/Orchestrator.h -o run.TEST -lcurl -pthread

*/

Orchestrator::Orchestrator(bool wallet) : _wallet(wallet){
  // TODO: Put this into a txt file
  // std::vector<std::string> currencies{"REEFUSDT", "CHZUSDT", "MATICUSDT", "VETUSDT", "FETUSDT", "ZILUSDT", "RLCUSDT", "OCEANUSDT", "ATOMUSDT", "LUNAUSDT"};
  std::vector<std::string> currencies{"BTCUSDT"};

  int id_currency = 0;

  for (std::string coin : currencies) {
      std::cout << "Coin: " << coin << std::endl; 
    std::shared_ptr<QueryCrypto> crypto =
        std::make_shared<QueryCrypto>(coin, ++id_currency, "single", 10);
    _cryptos.emplace_back(crypto);
  }

  // For querying all data from all coins on binance
  // TODO: Uncomment once finishing command window dashboard
  _allCryptos = std::make_shared<QueryCrypto>("all", ALL_COINS_ID, "all", 2);

  // Query only in the beginning current porftolio
  // and only if wallet is enabled
  if (this->_wallet) {
    _walletCryptos = std::make_shared<QueryCrypto>("wallet", ALL_COINS_ID + 1, "wallet", 5000);
  }
}

Orchestrator::~Orchestrator() {
  // set up thread barrier before this object is destroyed

  std::cout << "Calling destructor" << std::endl;
//   _threads.front().join();
//   std::for_each(_threads.begin(), _threads.end(),
//                 [](std::thread &t) { t.join(); });

  _threadCryptos.front().join();
  std::for_each(_threadCryptos.begin(), _threadCryptos.end(),
                [](std::thread &t) { t.join(); });
}

void Orchestrator::runQuery() {
  for (auto &c : _cryptos) {
    _threadCryptos.emplace_back(std::thread(&QueryCrypto::runSingleQueries, c));
  }
  // TODO: Uncomment once finishing command window dashboard
  _threadCryptos.emplace_back(std::thread(&QueryCrypto::runSingleQueries, _allCryptos));
  if (this->_wallet) {
    _threadCryptos.emplace_back(std::thread(&QueryCrypto::runWalletQuery, _walletCryptos));
  }
}

void Orchestrator::setCoinsToPlot(std::vector<std::string> coinsToPlot){
  _allCryptos->setCoinsToPlot(coinsToPlot); 
}

std::vector<std::vector<std::vector<std::string>>> Orchestrator::getAllPlotData() {
  return this->_allCryptos->getAllPlotData(); 
}

std::vector<std::string>Orchestrator::getCoinsToPlot(){
  return _allCryptos->getCoinsToPlot(); 
}

int Orchestrator::getCurrentWeightRequest(){
  return _allCryptos->getCurrentWeightRequest();
}

void Orchestrator::setWindowRange(int &windowRange){
  _allCryptos->setWindowRange(windowRange); 
}

void Orchestrator::setWalletStatus(bool wallet){
  _allCryptos->setWalletStatus(wallet); 
}

bool Orchestrator::isWalletEnabled(){ 
  return _allCryptos->isWalletEnabled(); 
}

void Orchestrator::runWalletQuery() {
  if (this->_wallet) {
  }
}

std::map<std::string, double> Orchestrator::getCoinToQuantity() { 
  return this->_walletCryptos->getCoinToQuantity(); 
}

std::map<std::string, double> Orchestrator::getCoinToPrice() { 
  return this->_walletCryptos->getCoinToPrice(); 
}