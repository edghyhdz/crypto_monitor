#ifndef BINANCE_H
#define BINANCE_H

#include <string>
#include <vector>
#include <unistd.h>

// Binance api end point definitions and utility methods
namespace Binance {
// TODO -> modify limit so that it takes it also as an argument
const std::string BASE_URL{"https://api.binance.com/api/v3/aggTrades?limit=130&symbol="};
const std::string ACCOUNT_URL{"https://api.binance.com/api/v3/account"}; 
const std::string TICKER{"https://api.binance.com/api/v3/ticker/price"}; 
const std::string USED_WEIGHT{"x-mbx-used-weight"};
const std::string USED_WEIGHT_PER_INTERVAL{"x-mbx-used-weight-1m"};
const std::string HTTP_RESPONSE{"HTTP/1.1 200 OK"};
const std::string RESPONSE{"Response"};
const std::string OK_RESPONSE_STR{"200"};
const std::string BAD_RESPONSE{"404"};
const std::string FIRST_KEY_WORD{"makerCommission"}; // Used to parse json data
const std::string LAST_KEY_WORD{"SPOT"}; // Used to parse json data
const int INTERVAL_LIMIT(1100); 
const int MAX_WORKER_SIZE(60); 
const int MAX_WORKER_BATCH_SIZE(30); 
const int MAX_COIN_TO_PLOT(4); 
const int KEY_COUNTER(3);
const long OK_RESPONSE(200);  

} // namespace Binance

#endif
