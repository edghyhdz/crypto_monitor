#include "Orchestrator.h"
#include "QueryCrypto.h"
#include <algorithm>
#include <chrono>
#include <future>
#include <random>
#include <thread>

/*
Orchestrator member attribute declarations
g++ -g src/QueryCrypto.cpp src/QueryCrypto.h src/Binance.h src/Orchestrator.cpp src/Orchestrator.h -o run.out -lcurl -pthread


*/

Orchestrator::Orchestrator() {
  // TODO: Put this into a txt file
  std::vector<std::string> currencies{"BTCUSDT", "ETHUSDT"};
  int id_currency = 0;

  for (std::string coin : currencies) {
      std::cout << "Coin: " << coin << std::endl; 
    std::shared_ptr<QueryCrypto> crypto =
        std::make_shared<QueryCrypto>(coin, ++id_currency);
    _cryptos.emplace_back(crypto);
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

  std::cout << "Finished calling destructor" << std::endl;

}

void Orchestrator::runQuery() {
  for (auto &c : _cryptos) {
    _threadCryptos.emplace_back(std::thread(&QueryCrypto::runQueries, c));
  }
}

int main() {
  Orchestrator orchestrator = Orchestrator();
  orchestrator.runQuery();
//   QueryCrypto crypto = QueryCrypto("BTCUSDT", 1);
//   crypto.getData();
  return 0;
}