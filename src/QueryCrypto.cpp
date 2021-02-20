#include "QueryCrypto.h"
#include "Binance.h"
#include "utils_crypto.h"
#include <algorithm>
#include <boost/algorithm/string.hpp>
#include <curl/curl.h>
#include <fstream>
#include <sstream>
#include <thread>
#include <unistd.h>
#include <chrono>
#include <openssl/hmac.h>
#include <iomanip> 

using std::ifstream; 
using namespace std::chrono;


/*
QueryCrypto class definitions
*/

std::vector<int> QueryCrypto::_requestWeight;
std::map<std::string, double> QueryCrypto::_coinToQuantity; 
std::map<std::string, double> QueryCrypto::_coinToPrice; 

// Run queries in a while loop
void QueryCrypto::runSingleQueries() {
  std::cout << "Started querying: " << this->getCoinPair() << std::endl;
  while (true) {
    this->getData();
    std::this_thread::sleep_for(std::chrono::seconds(this->_http_delay));
    // TODO: Cycle duration instead of sleep
  }
}

// Add pointer to dictionary from Orchestrator
void QueryCrypto::runWalletQuery() {
  long http_code; 
  std::string readBuffer;
  std::map<std::string, std::string> hDictionary; 
  // std::map<std::string, double> *coinToQuantity; 
  // Do http request
  this->getRequest(&readBuffer, &http_code, &hDictionary, true);

  std::lock_guard<std::mutex> lock(_mutex);
  formatData(readBuffer, &this->_coinToQuantity);  

  // Check if response was successfull
  if (http_code == Binance::OK_RESPONSE) {
    // Else stop querying
  }
}

std::map<std::string, double> QueryCrypto::getCoinToQuantity(){
  std::lock_guard<std::mutex> lock(_mutex);
  return _coinToQuantity; 
}

std::map<std::string, double> QueryCrypto::getCoinToPrice(){
  std::lock_guard<std::mutex> lock(_mutex);
  return _coinToPrice; 
}

void QueryCrypto::getRequest(std::string *readBuffer, long *http_code, std::map<std::string, std::string> *hDictionary, bool wallet=false){

  std::string url; 
  // Check if all coins need to be queried and assign endpoint accordingly
  if (this->getQueryType() == "all"){
    url = Binance::TICKER; 
  } else if (this->getQueryType() == "single"){
    url = Binance::BASE_URL + this->getCoinPair(); 
  }
  else {
    url = Binance::ACCOUNT_URL; 
  }

  curl_global_init(CURL_GLOBAL_ALL);
  CURL *curl = curl_easy_init();
  
  // Headers and set up signature
  if (wallet){
    // TODO: Fix all these, make it into a proper function
    const char *API_KEY = std::getenv("BINANCE_API_KEY");
    const char *API_SECRET = std::getenv("BINANCE_API_SECRET");
    std::string api_key(API_KEY);
    std::string key(API_SECRET);
    unsigned char result[EVP_MAX_MD_SIZE];
    unsigned result_len = 0;
    long ms = duration_cast<milliseconds>(system_clock::now().time_since_epoch()).count();

    // Current time stamp
    std::string message = "timestamp=" + std::to_string(ms);

    HMAC(EVP_sha256(), key.data(), key.size(),
         reinterpret_cast<unsigned char const *>(message.data()),
         message.size(), result, &result_len);

    curl_slist *list;
    std::string signature = Utils::binary_to_hex(result, result_len);
    url = url + "?" + message + "&signature=" + signature;

    // Make headers with current timestamp and signature to send to binance api
    // get current's user portfolio
    list = curl_slist_append(list, ("X-MBX-APIKEY: " + api_key + "\r\n").c_str());
    list = curl_slist_append(list, "timeout: 10\r\n");
    list = curl_slist_append(list, "\r\n");
    curl_easy_setopt(curl, CURLOPT_HTTPHEADER, list);
    // curl_easy_setopt(curl, CURLOPT_VERBOSE, 1L);
  }
  curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
  curl_easy_setopt(curl, CURLOPT_HEADER, 1); // Used to write response header data back
  curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, Utils::WriteCallback);
  curl_easy_setopt(curl, CURLOPT_WRITEDATA, readBuffer);
  curl_easy_perform(curl);
  curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, http_code); 
  curl_easy_cleanup(curl);
  curl_global_cleanup();

  *hDictionary = this->parseHeaderData(*readBuffer);
}

void QueryCrypto::getData(){
  long http_code; 
  std::string readBuffer;
  std::map<std::string, std::string> hDictionary; 

  // Do http request
  this->getRequest(&readBuffer, &http_code, &hDictionary);

  // Check if response was successfull
  if (http_code == Binance::OK_RESPONSE) {

    // Add request to request vector
    this->addRequestWeight(hDictionary.at(Binance::USED_WEIGHT));

    // Save data into csv file
    if (this->getQueryType() == "all") {
      this->saveAllCoinsCSVData(readBuffer);
    } else if (this->getQueryType() == "single") {
      this->saveCSVData(readBuffer);
    }
  } else {
    std::this_thread::sleep_for(std::chrono::seconds(10));
    std::cout << "Continuing" << std::endl;
  }

  if (std::stoi(hDictionary.at(Binance::USED_WEIGHT)) >
      Binance::INTERVAL_LIMIT) {
    // TODO:
    // If limit is about to exceed -> send message to kill all requests
    std::cout << "Should send message to all threads to terminate" << std::endl;
  }
}

// saves data into csv file from all coins on binance
void QueryCrypto::saveAllCoinsCSVData(std::string &readBuffer){

  // Structure of json response
  // [{'symbol': 'LTCBTC', 'price': '0.00396400'}, {...}]
  std::vector<std::vector<std::string>> allData; 
  std::vector<std::vector<std::vector<std::string>>> allDataTest; 

  auto system_clock = std::chrono::system_clock::now();
  long time_stamp = std::chrono::duration_cast<std::chrono::milliseconds>(system_clock.time_since_epoch()).count();
  std::string time_stamp_str = std::to_string(time_stamp); 
  std::string::size_type index;
  std::istringstream resp(readBuffer);

  size_t pos = 0;
  std::string token, row, columns, last_char, data;
  std::string delimiter = "}";
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

        // Replace quotes with empty char
        std::replace(token.begin(), token.end(), '"', ' ');
        std::replace(token.begin(), token.end(), '{', ' ');

        // Split token into key/value
        std::vector<std::string> splitString = Utils::split(token, ",");
        data.erase(0, pos + delimiter.length());

        if (splitString.size() > 0) {
          std::vector<std::string> symbol = Utils::split(splitString[0], ":");
          std::vector<std::string> price = Utils::split(splitString[1], ":");
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
  std::vector<std::string> allCoins = this->getCoinsToPlot(); 
  std::vector<std::vector<std::vector<std::string>>> allPlotData;

  // Initialize vector with size of allCoins
  // This way, position of coins from gui will remain equal here
  // for Coin 1, Coin 2, etc...
  for (int k = 0; k < allCoins.size(); k++){
    allPlotData.push_back({}); 
  }

  for (int i = 0; i < allDataTest.size() ; i++) {
    // Each of these vectors are used to be saved in a thread
    this->saveData(allDataTest[i], &allPlotData);

    // // If inside a thread
    // for (int j = 0; j < allDataTest[i].size(); j++) {
    //   // here goes thread code
    // }
  }
  // Setting plot data for all currencies
  this->setAllPlotData(allPlotData);
}

// QueryCrypto::saveAllCoinsCSVData helper function
void QueryCrypto::saveData(std::vector<std::vector<std::string>> &chunkData, std::vector<std::vector<std::vector<std::string>>> *allPlotData){
  // Vector to have all plot data from all coins to fetch
  std::vector<std::string> allCoins = this->getCoinsToPlot(); 
  std::vector<std::string> walletCoins; 

  // Only interested in USDT conversion
  for (auto &kv : this->_coinToQuantity) {
      walletCoins.push_back(kv.first + "USDT"); 
  }

  for (int k = 0; k < chunkData.size(); k++) {
    std::ofstream outfile;
    std::string coin_name = chunkData[k][0]; 
    std::remove(coin_name.begin(), coin_name.end(), ' ');
    std::string file_name = Utils::getCurrentDirectory() + "/data_test/" + coin_name + ".csv";

    outfile.open(file_name, std::ios_base::app);
    outfile << chunkData[k][1] + ";" + chunkData[k][2] + "\n";   
    outfile.close();

    if (this->_wallet) {
      // TODO: Fix this redundant coin name
      std::string coin = chunkData[k][0];
      coin.erase(remove(coin.begin(), coin.end(), ' '), coin.end());
  
      if (std::find(walletCoins.begin(), walletCoins.end(), coin) != walletCoins.end()){
        this->setCoinToPrice(file_name, coin);
      }
    }

    // New implementation
    if (allCoins.size()>0){
      // If coin was found
      if (std::find(allCoins.begin(), allCoins.end(), coin_name) != allCoins.end()){
        ptrdiff_t pos = distance(allCoins.begin(), find(allCoins.begin(), allCoins.end(), coin_name)); 
        this->parsePlotData(file_name, allPlotData, pos);
      }
    }
  }
}


void QueryCrypto::formatData(std::string &readBuffer, std::map<std::string, double> *coinToQuantity) {
  std::string::size_type index;
  std::istringstream resp(readBuffer);
  // std::map<std::string, double> coinToQuantity;
  size_t pos = 0;
  std::string token, data;
  std::string delimiter = "}";
  std::string first_word = "makerCommission";
  std::string last_word = "SPOT";

  while (std::getline(resp, data)) {
    index = data.find('[', 0);
    if (index != std::string::npos) {
      std::replace(data.begin(), data.end(), '[', ' ');
      std::replace(data.begin(), data.end(), ']', '}');
      // While delimiter is found
      while ((pos = data.find(delimiter)) != std::string::npos) {

        token = data.substr(0, pos);

        // Replace quotes with empty char
        std::replace(token.begin(), token.end(), '"', ' ');
        std::replace(token.begin(), token.end(), '{', ' ');

        // Split token into key/value
        std::vector<std::string> splitString = Utils::split(token, ",");
        data.erase(0, pos + delimiter.length());
        if (splitString.size() > 0) {
          if (splitString[0].find(Binance::FIRST_KEY_WORD) == std::string::npos &&
              splitString[0].find(Binance::LAST_KEY_WORD) == std::string::npos) {
            std::vector<std::string> price =  Utils::split(splitString[1], ":");

            if (stod(price[1]) > 0) {
              std::vector<std::string> asset_coin =  Utils::split(splitString[0], ":");
              std::string coin_name = asset_coin[1]; 
              coin_name.erase(remove(coin_name.begin(), coin_name.end(), ' '), coin_name.end());
              // std::remove(coin_name.begin(), coin_name.end(), ' ');
              (*coinToQuantity).insert(std::make_pair(coin_name, stod(price[1])));
            }
          }
        }
      }
    }
  }
}

void QueryCrypto::parsePlotData(std::string file_name, std::vector<std::vector<std::vector<std::string>>> *allPlotData, ptrdiff_t pos) {
  /*  Helper function to parse data from txt file
      file_name     - path to file with coin data
  */

  char c;
  int row_counter = 0;
  std::vector<std::vector<std::string>> plotData;
  std::ifstream myFile(file_name, std::ios::ate);
  std::streampos size = myFile.tellg();
  std::string line = "";
  // Read file from bottom up
  // Help reference https://stackoverflow.com/a/27750629/13743493
  for (int i = 1; i <= size; i++) {
    myFile.seekg(-i, std::ios::end);
    myFile.get(c);
    if (c != '\n') {
      line += c;
    } else {
      row_counter++;
      // characters are read from right to left
      std::reverse(line.begin(), line.end());
      std::vector<std::string> time_price = Utils::split(line, ";");
      if (time_price.size() > 1) {
        plotData.push_back(time_price);
      }
      line = "";
    }
    if (row_counter >= this->getWindowRange()) {
      break;
    }
  }
  (*allPlotData)[pos] = plotData;
  myFile.close();
}

// 
void QueryCrypto::setCoinToPrice(std::string file_name, std::string coin){
  char c;
  int row_counter = 0;
  std::ifstream myFile(file_name, std::ios::ate);
  std::streampos size = myFile.tellg();
  std::string line = "";
  // Read file from bottom up
  // Help reference https://stackoverflow.com/a/27750629/13743493
  for (int i = 1; i <= size; i++) {
    myFile.seekg(-i, std::ios::end);
    myFile.get(c);
    if (c != '\n') {
      line += c;
    } else {
      row_counter++;
      // characters are read from right to left
      std::reverse(line.begin(), line.end());
      std::vector<std::string> time_price = Utils::split(line, ";");
      if (time_price.size() > 1) {
        // Add price data
        this->updateCoinToPrice(time_price[1], coin); 
        break;
      }
      line = "";
    }
  }
  myFile.close();
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
        std::vector<std::string> splitString = Utils::split(token, ":");
        data.erase(0, pos + delimiter.length());

        // Add new columns to row string
        if (lastColumn) {
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

std::map<std::string, std::string> QueryCrypto::parseHeaderData(std::string &readBuffer) {
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

void QueryCrypto::addRequestWeight(std::string requestWeight) {
  /*  Saves current request weight into a matrix, this matrix will later be used
     to get the max value of current request weight for a given time interval ->
     following binance api endpoin tlimits
  */
  std::lock_guard<std::mutex> lock(_mutex);
  this->_requestWeight.push_back(stoi(requestWeight));

  // Avoid that vector surpasses a certain vector size
  // pops back last item in the vector
  if (this->_requestWeight.size() > 10) {
    this->_requestWeight.erase(_requestWeight.begin());
  }
}

int QueryCrypto::getCurrentWeightRequest() {
  /*  Get maximum value of weight request, which will correspond
      to the current request weight
  */
  std::lock_guard<std::mutex> lock(_mutex);
  if (_requestWeight.size()==0) return 0;
  return _requestWeight.at(_requestWeight.size() - 1); 
}

void QueryCrypto::setCoinsToPlot(std::vector<std::string> coinsToPlot){
  std::lock_guard<std::mutex> lock(_mutex);
  this->_coinsToPlot.clear(); 
  for (std::string &coin : coinsToPlot){
    coin.erase(remove(coin.begin(), coin.end(), ' '), coin.end());
    coin = coin + " "; 
    this->_coinsToPlot.push_back(coin); 
  }
}

std::vector<std::string> QueryCrypto::getCoinsToPlot() {
  /*
  Gets coin to be plotted into terminal dashboard
  */
  std::lock_guard<std::mutex> lock(_mutex);
  if (this->_coinsToPlot.size()>0) return _coinsToPlot; 
  return {"ZILUSDTT ", "ENJUSDTT "} ;
}

void QueryCrypto::setWindowRange(int &windowRange){
  std::lock_guard<std::mutex> lock(_mutex);
  _windowRange = windowRange; 
}

int QueryCrypto::getWindowRange() { 
  std::lock_guard<std::mutex> lock(_mutex);
  return _windowRange; 
}

void QueryCrypto::setWalletStatus(bool wallet){
  std::lock_guard<std::mutex> lock(_mutex);
  _wallet = wallet; 
}

bool QueryCrypto::isWalletEnabled(){
  std::lock_guard<std::mutex> lock(_mutex);
  return _wallet; 
}

void QueryCrypto::updateCoinToPrice(std::string price, std::string coin){

  std::lock_guard<std::mutex> lock(_mutex);
  double d_price = stod(price); 
  std::map<std::string, double>::iterator it = this->_coinToPrice.find(coin); 
  if (it != _coinToPrice.end())
      it->second = d_price;
  else {
    _coinToPrice.insert(std::make_pair(coin, d_price));
  }
}