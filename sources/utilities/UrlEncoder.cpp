#include <UrlEncoder.hpp>

bool UrlEncoder::isUnreserved(char c) {
  return std::isalnum(c) || c == '-' || c == '_' || c == '.' || c == '~';
}

std::string UrlEncoder::charToHex(char c) {
  std::ostringstream hex;
  hex << std::uppercase << std::hex << static_cast<int>(c);
  return hex.str();
}

char UrlEncoder::hexToChar(const std::string& hex) {
  int c;
  std::istringstream(hex) >> std::hex >> c;
  return static_cast<char>(c);
}

std::string UrlEncoder::encode(const std::string& str) {
  std::string encoded;
  for (char c : str) {
    if (isUnreserved(c)) {  // Does not need to be encoded
      encoded += c;
    } else {
      encoded += '%';
      encoded += charToHex(c);
    }
  }
  return encoded;
}

std::string UrlEncoder::decode(const std::string& str) {
  std::string decoded;
  for (size_t i = 0; i < str.size(); ++i) {
    if (str[i] == '%' && i + 2 < str.size()) {
      decoded += hexToChar(str.substr(i + 1, 2));
      i += 2;
    } else {
      decoded += str[i];
    }
  }
  return decoded;
}
