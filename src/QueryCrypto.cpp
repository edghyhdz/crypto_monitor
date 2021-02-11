#include "QueryCrypto.h"
#include "Binance.h"
#include <algorithm>
#include <boost/algorithm/string.hpp>
#include <curl/curl.h>
#include <fstream>
#include <sstream>
#include <thread>
#include <unistd.h>

/*
QueryCrypto class definitions
*/


// Run queries in a while loop
void QueryCrypto::runSingleQueries() {
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
  long http_code = 0; 
  CURL *curl = curl_easy_init();
  std::string readBuffer;
  std::map<std::string, std::string> hDictionary; 
  std::string url;

  // Check if all coins need to be queried and assign endpoint accordingly
  if (this->allQueries()){
    url = Binance::TICKER; 
  }
  else {
    url = Binance::BASE_URL + this->getCoinPair(); 
  }

  curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
  // curl_easy_setopt(curl, CURLOPT_PROXYPORT, 8080L);
  // curl_easy_setopt(curl, CURLOPT_VERBOSE, 1L);
  curl_easy_setopt(curl, CURLOPT_HEADER, 1); // Used to write response header data back
  curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, Binance::WriteCallback);
  curl_easy_setopt(curl, CURLOPT_WRITEDATA, &readBuffer);
  curl_easy_perform(curl);
  curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &http_code); 
  curl_easy_cleanup(curl);
  curl_global_cleanup();

  // Parse header dictionary
  // To be used to check current requests weight -> limit / minute
  hDictionary = this->parseHeaderData(readBuffer);
  
  if (http_code == Binance::OK_RESPONSE){
    std::cout << "RESPONSE CODE: " << http_code << std::endl; 
  }
  if (hDictionary.at(Binance::RESPONSE) == Binance::OK_RESPONSE_STR){

    // Save data into csv file
    if (this->allQueries()) {
      this->saveAllCoinsCSVData(readBuffer); 
    } else {
      this->saveCSVData(readBuffer);
    }

  } else {
    std::this_thread::sleep_for(std::chrono::seconds(10));
    std::cout << "Continuing" << std::endl; 
  }
  
  if (std::stoi(hDictionary.at(Binance::USED_WEIGHT)) > Binance::INTERVAL_LIMIT){
    // If limit is about to exceed -> send message to kill all requests
    std::cout << "Should send message to all threads to terminate" << std::endl; 
  }
}


// saves data into csv file from all coins on binance
void QueryCrypto::saveAllCoinsCSVData(std::string &readBuffer){

  // Structure of response
  // [{'symbol': 'LTCBTC', 'price': '0.00396400'}, {...}]
  std::vector<std::vector<std::string>> allData; 
  std::vector<std::vector<std::vector<std::string>>> allDataTest; 

  std::vector<std::vector<int>> workerLoad;  // Index thread should work with
  auto system_clock = std::chrono::system_clock::now();
  long time_stamp = std::chrono::duration_cast<std::chrono::milliseconds>(system_clock.time_since_epoch()).count();
  std::string time_stamp_str = std::to_string(time_stamp); 
  std::string::size_type index;
  std::istringstream resp(readBuffer);

  size_t pos = 0;
  std::string token, row, columns, last_char, data;
  std::string delimiter = "}";
  bool lastColumn = false;
  int index_end = 0; 

  while (std::getline(resp, data)) {
    index = data.find('[', 0);
    
    if (index != std::string::npos) {
      std::replace(data.begin(), data.end(), '[', ' ');
      // Replace <[> with <}>, so that find method goes until last item
      std::replace(data.begin(), data.end(), ']', '}');
      // While delimiter is found
      while ((pos = data.find(delimiter)) != std::string::npos) {

        token = data.substr(0, pos);
        // Check if we are in the last column
        lastColumn = (token.find('}') != std::string::npos) ? true : false;

        // Replace quotes with empty char
        std::replace(token.begin(), token.end(), '"', ' ');
        std::replace(token.begin(), token.end(), '{', ' ');

        // Split token into key/value
        std::vector<std::string> splitString = Binance::split(token, ",");
        data.erase(0, pos + delimiter.length());

        if (splitString.size() > 0) {
          std::vector<std::string> symbol = Binance::split(splitString[0], ":");
          std::vector<std::string> price = Binance::split(splitString[1], ":");
          // Push data into a vector that will later go to be saved into 
          
          if (++index_end < Binance::MAX_WORKER_BATCH_SIZE) {
            allData.push_back({symbol[1], time_stamp_str, price[1]});
          }
          else{
            allData.push_back({symbol[1], time_stamp_str, price[1]});
            allDataTest.push_back(allData); 
            allData.clear(); 
            index_end = 0; 
          }
        }
      }
      allDataTest.push_back(allData); 
      break;
    }
  }

  // Save all the data
  std::chrono::time_point<std::chrono::system_clock> currentTime;
  currentTime = std::chrono::system_clock::now();
  for (int i = 0; i < allDataTest.size() ; i++) {
    // Each of these vectors are used to be saved in a thread
    for (int j = 0; j < allDataTest[i].size(); j++) {
      this->saveData(allDataTest[i]); 
    }
  }
  long timeDifference = std::chrono::duration_cast<std::chrono::milliseconds>(
                            std::chrono::system_clock::now() - currentTime)
                            .count();

  std::cout << "Took: " << timeDifference << " miliseconds to save without threads" << std::endl; 
                    
}

// QueryCrypto::saveAllCoinsCSVData helper function
void QueryCrypto::saveData(std::vector<std::vector<std::string>> &chunkData){

  for (int k = 0; k < chunkData.size(); k++) {
    std::ofstream outfile;
    std::string file_name = Binance::getCurrentDirectory() + "/data/" + chunkData[k][0] + ".csv";
    outfile.open(file_name, std::ios_base::app);
    outfile << chunkData[k][1] + ";" + chunkData[k][2] + "\n";
    outfile.close(); 
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
          std::make_pair(Binance::RESPONSE, Binance::OK_RESPONSE_STR));
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