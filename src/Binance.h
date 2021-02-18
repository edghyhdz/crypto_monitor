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
const int INTERVAL_LIMIT(1100); 
const int MAX_WORKER_SIZE(60); 
const int MAX_WORKER_BATCH_SIZE(30); 
const int MAX_COIN_TO_PLOT(4); 
const int KEY_COUNTER(3);
const long OK_RESPONSE(200);  

// get current directory
// reference https://www.daniweb.com/programming/software-development/threads/406729/current-directory
std::string getCurrentDirectory(){
  char strDir[129] = {0};
  getcwd(strDir, 128);

  return strDir; 
}

// Split
// reference https://stackoverflow.com/a/14266139/13743493
std::vector<std::string> split(const std::string &str, const std::string &delim) {
  std::vector<std::string> tokens;
  size_t prev = 0, pos = 0;
  do {
    pos = str.find(delim, prev);
    if (pos == std::string::npos)
      pos = str.length();
    std::string token = str.substr(prev, pos - prev);
    if (!token.empty())
      tokens.push_back(token);
    prev = pos + delim.length();
  } while (pos < str.length() && prev < str.length());
  return tokens;
}

// Callback to curl function
// Reference https://curl.se/libcurl/c/CURLOPT_WRITEFUNCTION.html
size_t WriteCallback(char *contents, size_t size, size_t nmemb, void *userp) {
  ((std::string *)userp)->append((char *)contents, size * nmemb);
  return size * nmemb;
}

} // namespace Binance

#endif
