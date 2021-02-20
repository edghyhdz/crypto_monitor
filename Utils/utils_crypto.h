
#include <vector>
#include <string>
#include <unistd.h>

namespace Utils {
// Split
// reference https://stackoverflow.com/a/14266139/13743493
std::vector<std::string> split(const std::string &str,
                               const std::string &delim) {
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

// get current directory
// reference https://www.daniweb.com/programming/software-development/threads/406729/current-directory
std::string getCurrentDirectory(){
  char strDir[129] = {0};
  getcwd(strDir, 128);

  return strDir; 
}

// TODO: 
// Reference https://github.com/Chudleyj/Binance-Trading-Bot.git
auto binary_to_hex_digit(unsigned a) -> char{
    return a + (a < 10 ? '0' : 'a' - 10);
}

// Reference https://github.com/Chudleyj/Binance-Trading-Bot.git
auto binary_to_hex(unsigned char const* binary, unsigned binary_len) -> std::string {
    std::string r(binary_len * 2, '\0');
    for(unsigned i = 0; i < binary_len; ++i) {
        r[i * 2] = binary_to_hex_digit(binary[i] >> 4);
        r[i * 2 + 1] = binary_to_hex_digit(binary[i] & 15);
    }
    return r;
}

} // namespace Utils