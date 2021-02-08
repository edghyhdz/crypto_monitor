#include <algorithm>
#include <boost/algorithm/string.hpp>
#include <curl/curl.h>
#include <iostream>
#include <map>
#include <sstream>
#include <string>
#include <vector>
#include <fstream>


/// g++ response.cpp -lcurl

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

void saveCSVData(std::string &readBuffer) {
  std::ofstream outfile;
  outfile.open("test_file.csv", std::ios_base::app); // append instead of overwrite

  std::string::size_type index;
  std::istringstream resp(readBuffer);

  size_t pos = 0;
  std::string token, row, columns, last_char, data;
  std::string delimiter = ",";
  bool lastColumn = false;
  int c_loop = 0; // Used to count the colums of the data to query

  while (std::getline(resp, data)) {
    index = data.find('[', 0);
    if (index != std::string::npos) {
      std::replace(data.begin(), data.end(), '[', ' ');
      // Replace <[> with <,>, so that find method goes until last item
      std::replace(data.begin(), data.end(), ']', ',');
      // While delimiter is found
      while ((pos = data.find(delimiter)) != std::string::npos) {

        token = data.substr(0, pos);
        // Check if we are in the last column
        lastColumn = (token.find('}') != std::string::npos) ? true : false;

        // Replace quotes with empty char
        std::replace(token.begin(), token.end(), '"', ' ');

        // If last column, get rid of curlybrackets
        if (lastColumn == true) {
          std::replace(token.begin(), token.end(), '}', ' ');
        } else {
          std::replace(token.begin(), token.end(), '{', ' ');
        }

        // Split token into key/value 
        std::vector<std::string> splitString = split(token, ":");
        data.erase(0, pos + delimiter.length());

        // Add column names to column string
        if (c_loop == 0) {
          last_char = (lastColumn) ? "" : ";";
          columns += splitString[0] + last_char;
          // if (lastColumn) {
          //   // Pass columns into text only if beginning of file
          //   std::cout << columns << std::endl;
          // }
        }

        // Add new columns to row string
        if (lastColumn) {
          c_loop++;
          row += splitString[1] + "\n";
          outfile << row;
          row = "";
        } else {
          row += splitString[1] + ";";
        }
      }    
      break;
    }
  }
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

    if (header.find(status_response) != std::string::npos) {
          headerDictionary.insert(std::make_pair("response", "200"));
          keys_counter++;
    }

    // Check if we have all keys we are interested on
    if (header.find(used_weight) != std::string::npos) {
      keys_counter++;
    } else if (header.find(used_weight_interval) != std::string::npos) {
      keys_counter++;
    }

    // Break while loop once we have all keys we want
    if (keys_counter == 3) {
      break;
    }
  }

  // for (auto &kv : headerDictionary) {
  //   std::cout << "header data: " << kv.first << ": " << kv.second << std::endl;
  // }

  std::cout << "RESPONSE                : " << headerDictionary.at("response") << std::endl;
  std::cout << "USED WEIGHT             : " << headerDictionary.at("x-mbx-used-weight") << std::endl;
  std::cout << "USED WEIGHT / INTERVAL  : " << headerDictionary.at("x-mbx-used-weight-1m") << std::endl;

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
                   "https://api.binance.com/api/v3/aggTrades?limit=775&symbol=BTCUSDT");
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

  saveCSVData(readBuffer);

  // std::cout << "Data: " << readBuffer << std::endl;
  return 0;
}

/*
TODO: 
- Class to query data
- Threads to search for different cryptos
- Add column names to beginning of file

*/