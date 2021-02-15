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

Orchestrator::Orchestrator() {
  // TODO: Put this into a txt file
  // std::vector<std::string> currencies{"REEFUSDT", "CHZUSDT", "MATICUSDT", "VETUSDT", "FETUSDT", "ZILUSDT", "RLCUSDT", "OCEANUSDT", "ATOMUSDT", "LUNAUSDT"};
  std::vector<std::string> currencies{"BTCUSDT"};

  int id_currency = 0;

  for (std::string coin : currencies) {
      std::cout << "Coin: " << coin << std::endl; 
    std::shared_ptr<QueryCrypto> crypto =
        std::make_shared<QueryCrypto>(coin, ++id_currency, false, 10);
    _cryptos.emplace_back(crypto);
  }

  // For querying all data from all coins on binance
  // TODO: Uncomment once finishing command window dashboard
  _allCryptos = std::make_shared<QueryCrypto>("all", ALL_COINS_ID, true, 2); 

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

std::vector<std::vector<std::string>> Orchestrator::getPlotData() {
  return this->_allCryptos->getPlotData(); 
}

void Orchestrator::runQuery() {
  for (auto &c : _cryptos) {
    _threadCryptos.emplace_back(std::thread(&QueryCrypto::runSingleQueries, c));
  }
  // TODO: Uncomment once finishing command window dashboard
  _threadCryptos.emplace_back(std::thread(&QueryCrypto::runSingleQueries, _allCryptos)); 

}

void Orchestrator::setCoinToPlot(std::string coinToPlot) {
  /*Sets coin to plot into shared_ptr allCrypto vector*/

  _allCryptos->setCoinToPlot(coinToPlot); 
}

std::string Orchestrator::getCoinToPlot(){
  return _allCryptos->getCoinToPlot(); 
}

int Orchestrator::getCurrentWeightRequest(){
  return _allCryptos->getCurrentWeightRequest();
}
