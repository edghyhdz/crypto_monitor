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
private:
  std::vector<std::shared_ptr<QueryCrypto>> _cryptos;
  std::vector<std::thread> _threads;
  std::vector<std::thread> _threadCryptos;
};

#endif