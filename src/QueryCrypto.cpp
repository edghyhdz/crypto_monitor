#include "QueryCrypto.h"
#include "Binance.h"
#include <algorithm>
#include <boost/algorithm/string.hpp>
#include <curl/curl.h>
#include <fstream>
#include <sstream>
#include <thread>


/*
QueryCrypto class definitions
*/


// Run queries in a while loop
void QueryCrypto::runQueries() {
  std::cout << "Started querying: " << this->getCoinPair() << std::endl;
  while (true) {
    this->getData();
    std::this_thread::sleep_for(std::chrono::seconds(10));
    // TODO: Cycle duration instead of sleep
  }
}

// Queries data to binance endpoint
void QueryCrypto::getData() {
  
  curl_global_init(CURL_GLOBAL_ALL);
  CURL *curl = curl_easy_init();
  std::string readBuffer;
  std::map<std::string, std::string> hDictionary; 
  curl_easy_setopt(curl, CURLOPT_URL, (Binance::BASE_URL + this->getCoinPair()).c_str());
  curl_easy_setopt(curl, CURLOPT_PROXYPORT, 8080L);
  // curl_easy_setopt(curl, CURLOPT_VERBOSE, 1L);  
  curl_easy_setopt(curl, CURLOPT_HEADER, 1); // Used to write response header data back
  curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, Binance::WriteCallback);
  curl_easy_setopt(curl, CURLOPT_WRITEDATA, &readBuffer);
  curl_easy_perform(curl);

  // Parse header dictionary
  // To be used to check current requests weight -> limit / minute
  hDictionary = this->parseHeaderData(readBuffer);
  
  if (hDictionary.at(Binance::RESPONSE) == Binance::OK_RESPONSE){
    // Save data into csv file
    this->saveCSVData(readBuffer);
  }
  
  if (std::stoi(hDictionary.at(Binance::USED_WEIGHT)) > Binance::INTERVAL_LIMIT){
    // If limit is about to exceed -> send message to kill all requests
    std::cout << "Should send message to all threads to terminate" << std::endl; 
  }
}

// Saves queried data into csv file
void QueryCrypto::saveCSVData(std::string &readBuffer) {
  std::ofstream outfile;
  std::string file_name = this->getCoinPair() + ".csv"; 
  outfile.open(file_name, std::ios_base::app);

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
        std::vector<std::string> splitString = Binance::split(token, ":");
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

std::map<std::string, std::string>
QueryCrypto::parseHeaderData(std::string &readBuffer) {
  std::map<std::string, std::string> headerDictionary;
  std::istringstream resp(readBuffer);
  std::string header;
  std::string::size_type index;
  std::string::size_type found_keys;

  headerDictionary.clear();
  
  // Check when we have all keys we need
  int keys_counter = 0;
  bool found_response = false; 
  while (std::getline(resp, header) && header != "\r") {

    index = header.find(':', 0);
    if (index != std::string::npos) {
      headerDictionary.insert(std::make_pair(
          boost::algorithm::trim_copy(header.substr(0, index)),
          boost::algorithm::trim_copy(header.substr(index + 1))));
    }

    // If correct response return 200
    if (header.find(Binance::HTTP_RESPONSE) != std::string::npos) {
      headerDictionary.insert(
          std::make_pair(Binance::RESPONSE, Binance::OK_RESPONSE));
      keys_counter++;
      found_response = true; 
    }

    // Check if we have all keys we are interested on
    if (header.find(Binance::USED_WEIGHT) != std::string::npos) {
      keys_counter++;
    } else if (header.find(Binance::USED_WEIGHT_PER_INTERVAL) !=
               std::string::npos) {
      keys_counter++;
    }

    // Break while loop once we have all keys we want
    if (keys_counter == Binance::KEY_COUNTER) {
      break;
    }

    // TODO: Check headers -> seems to be going on a loop several times
    // for (auto &kv : headerDictionary){
    //   std::cout << "Headers => " << kv.first + ": " << kv.second << std::endl; 
    // }

  }

  try {
    std::cout << "RESPONSE                : "
              << headerDictionary.at(Binance::RESPONSE) << std::endl;
    std::cout << "USED WEIGHT             : "
              << headerDictionary.at(Binance::USED_WEIGHT) << std::endl;
    std::cout << "USED WEIGHT / INTERVAL  : "
              << headerDictionary.at(Binance::USED_WEIGHT_PER_INTERVAL)
              << std::endl;
  } catch (std::out_of_range) {
    std::cout << "Out of range error" << std::endl;
  }

  // If no response was gotten 
  if (!found_response) {
    headerDictionary.insert(std::make_pair(Binance::RESPONSE, Binance::BAD_RESPONSE));
    
    // If not found -> incert another used weight value
    if ( headerDictionary.find(Binance::USED_WEIGHT) == headerDictionary.end() ) {
      headerDictionary.insert(std::make_pair(Binance::USED_WEIGHT, Binance::BAD_RESPONSE));
    } 
    
    std::cout << "Bad Response" << std::endl; 
  }

  return headerDictionary;
}


// Test function
// int main() {
//   QueryCrypto crypto = QueryCrypto("BTCUSDT", 1);
//   crypto.getData();
//   return 0;
// }