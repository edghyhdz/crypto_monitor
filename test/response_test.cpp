#include <algorithm>
#include <boost/algorithm/string.hpp>
#include <curl/curl.h>
#include <iostream>
#include <map>
#include <sstream>
#include <string>
#include <vector>


/// g++ response.cpp -lcurl

struct BaseObject {};

struct ObjectDouble : public BaseObject
{
  double a;
};

struct ObjectString : public BaseObject
{
  std::string a;
};

std::vector<std::string> split(const std::string& str, const std::string& delim)
{
    std::vector<std::string> tokens;
    size_t prev = 0, pos = 0;
    do
    {
        pos = str.find(delim, prev);
        if (pos == std::string::npos) pos = str.length();
        std::string token = str.substr(prev, pos-prev);
        if (!token.empty()) tokens.push_back(token);
        prev = pos + delim.length();
    }
    while (pos < str.length() && prev < str.length());
    return tokens;
}

std::vector<std::map<std::string, BaseObject>> parseJsonData(std::string &readBuffer) {
  std::vector<std::map<std::string, BaseObject>> dataDictionary;
  std::string::size_type index;
  std::istringstream resp(readBuffer);
  std::string data;

  while (std::getline(resp, data)) {
    index = data.find('[', 0);
    if (index != std::string::npos) {
      std::replace(data.begin(), data.end(), '[', '\0');
      std::replace(data.begin(), data.end(), ']', '\0');

      std::string delimiter = ",";

      size_t pos = 0;
      std::string token;
      std::vector<std::string> allData; 
      std::string row; 
      std::string columns; 
      int c_loop = 0;   // Used to count the colums of the data to query

      while ((pos = data.find(delimiter)) != std::string::npos) {
        token = data.substr(0, pos);
        // std::cout << token << std::endl;
        std::vector<std::string> splitString = split(token, ":"); 
        std::cout << "VALUE?: " << splitString[1] << std::endl; 
        // Split token into key/value pairs


        data.erase(0, pos + delimiter.length());

        if (c_loop == 0) {
            columns += ' '; 
        }


        if (token.find('}') != std::string::npos) {
          c_loop++;
          std::replace(token.begin(), token.end(), '}', '\0');
          row += token;
          allData.push_back(row);
        //   std::cout << row << std::endl;
          row = ""; 
        }
        else {
            std::replace(token.begin(), token.end(), '{', '\0');
            row += token + ";"; 
        }

      }
      std::cout  << data << std::endl;
      break; 
    }
  }

  return dataDictionary; 

}

std::map<std::string, std::string> parseHeaderData(std::string &readBuffer) {

  std::map<std::string, std::string> headerDictionary;
  std::istringstream resp(readBuffer);
  std::string header;
  std::string::size_type index;
  std::string::size_type found_keys;

  std::string used_weight = "x-mbx-used-weight:";
  std::string used_weight_interval = "x-mbx-used-weight-1m:";
  std::string status_response = "HTTP/1.1 200 OK";

  int keys_counter = 0;

  while (std::getline(resp, header) && header != "\r") {

    index = header.find(':', 0);
    if (index != std::string::npos) {
      headerDictionary.insert(std::make_pair(
          boost::algorithm::trim_copy(header.substr(0, index)),
          boost::algorithm::trim_copy(header.substr(index + 1))));
    }

    // Check if we have all keys we are interested on
    if (header.find(used_weight) != std::string::npos) {
      keys_counter++;
    } else if (header.find(used_weight_interval) != std::string::npos) {
      keys_counter++;
    } else if (header.find(status_response) != std::string::npos) {
      keys_counter++;
    }

    // Break while loop once we have all keys we want
    if (keys_counter == 3) {
      break;
    }
  }

  for (auto &kv : headerDictionary) {
    std::cout << "header data: " << kv.first << ": " << kv.second << std::endl;
  }

  return headerDictionary;
}

size_t WriteCallback(char *contents, size_t size, size_t nmemb, void *userp) {
  ((std::string *)userp)->append((char *)contents, size * nmemb);
  return size * nmemb;
}

int main(int argc, char *argv[]) {
  curl_global_init(CURL_GLOBAL_ALL);

  CURL *easyhandle = curl_easy_init();
  std::string readBuffer;
  std::string headerData;

  curl_easy_setopt(easyhandle, CURLOPT_URL,
                   "https://api.binance.com/api/v3/aggTrades?symbol=BTCUSDT");
  // curl_easy_setopt(easyhandle, CURLOPT_VERBOSE, 1L);
  // curl_easy_setopt(easyhandle, CURLOPT_PROXY, "http://my.proxy.net");   //
  // replace with your actual proxy
  curl_easy_setopt(easyhandle, CURLOPT_PROXYPORT, 8080L);
  curl_easy_setopt(easyhandle, CURLOPT_HEADER,
                   1); // Used to write response header data back
  curl_easy_setopt(easyhandle, CURLOPT_WRITEFUNCTION, WriteCallback);

  curl_easy_setopt(easyhandle, CURLOPT_WRITEDATA, &readBuffer);
  curl_easy_perform(easyhandle);

  std::map<std::string, std::string> headerDictionary =
      parseHeaderData(readBuffer);

  parseJsonData(readBuffer);

  //   std::cout << "Data: " << readBuffer << std::endl;
  return 0;
}