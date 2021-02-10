#ifndef ORCHESTRATOR_H
#define ORCHESTRATOR_H

#include <memory>
#include <thread>
#include <vector>

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
Orchestrator(); 
~Orchestrator(); 
// Reference: https://stackoverflow.com/a/23575458/13743493
Orchestrator(Orchestrator &&o) = default;

void runQuery(); 

private:
  std::vector<std::shared_ptr<QueryCrypto>> _cryptos;
  std::shared_ptr<QueryCrypto> _allCryptos; 
  std::vector<std::thread> _threads;
  std::vector<std::thread> _threadCryptos;
};

#endif