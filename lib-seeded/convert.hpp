#pragma once

#include <stdexcept>
#include <string>
#include <vector>
#include "sodium-buffer.hpp"


class InvalidHexCharacterException : public std::invalid_argument
{
  public:
  explicit InvalidHexCharacterException(const char* what =
    "Could not parse non-hex character"
  ) :
    std::invalid_argument(what) {}
};

inline unsigned char parseHexChar(char c) {
  if (c >= '0' && c <= '9') {
    return c - '0';
  } else if (c >= 'a' && c <= 'f') {
    return 10 + (c - 'a');
  } else if (c >= 'A' && c <= 'F') {
    return 10 + (c - 'A');
  }
  throw InvalidHexCharacterException();
}

const std::string toHexStr(const std::vector<unsigned char> bytes);
const std::vector<unsigned char> hexStrToByteVector(const std::string hexStr);
