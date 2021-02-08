#include "QueryCrypto.h"
#include "Binance.h"
#include <algorithm>
#include <fstream>
#include <sstream>
#include <boost/algorithm/string.hpp>


// Class definitions
void QueryCrypto::saveCSVData(std::string &readBuffer) {
  std::ofstream outfile;
  outfile.open("test_file.csv",
               std::ios_base::app); // append instead of overwrite

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

  int keys_counter = 0;

  while (std::getline(resp, header) && header != "\r") {

    index = header.find(':', 0);
    if (index != std::string::npos) {
      headerDictionary.insert(std::make_pair(
          boost::algorithm::trim_copy(header.substr(0, index)),
          boost::algorithm::trim_copy(header.substr(index + 1))));
    }

    // If correct response return 200
    if (header.find(Binance::HTTP_RESPONSE) != std::string::npos) {
      headerDictionary.insert(std::make_pair(Binance::RESPONSE, Binance::OK_RESPONSE));
      keys_counter++;
    }

    // Check if we have all keys we are interested on
    if (header.find(Binance::USED_WEIGHT) != std::string::npos) {
      keys_counter++;
    } else if (header.find(Binance::USED_WEIGHT_PER_INTERVAL) != std::string::npos) {
      keys_counter++;
    }

    // Break while loop once we have all keys we want
    if (keys_counter == 3) {
      break;
    }
  }

  std::cout << "RESPONSE                : " << headerDictionary.at(Binance::RESPONSE)
            << std::endl;
  std::cout << "USED WEIGHT             : "
            << headerDictionary.at(Binance::USED_WEIGHT) << std::endl;
  std::cout << "USED WEIGHT / INTERVAL  : "
            << headerDictionary.at(Binance::USED_WEIGHT_PER_INTERVAL) << std::endl;

  return headerDictionary;
}

int main() {
  QueryCrypto crypto = QueryCrypto("test", 1);

  crypto.printCoinPair();
  return 0;
}