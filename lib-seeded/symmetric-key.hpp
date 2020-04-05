#pragma once

#include <string>
#include "sodium-buffer.hpp"

class SymmetricKey {

  public:
  const SodiumBuffer keyBytes;
  const std::string keyDerivationOptionsJson;

  SymmetricKey(
    const SodiumBuffer& _key,
    std::string keyDerivationOptionsJson
  );

  SymmetricKey(
    const SymmetricKey &other
  );

  SymmetricKey(
    const std::string& symmetricKeyAsJson
  );

  SymmetricKey(
    const std::string& seed,
    const std::string &keyDerivationOptionsJson
  );

  /**
   *
   * LibSodium's crypto_secretbox_easy construct isn't THAT easy.
   * The caller must store both the ciphertext AND a 24-byte nonce
   * (crypto_secretbox_NONCEBYTES = 24).
   * 
   * This seal operation outputs a _composite_ ciphertext containing the
   * nonce followed by the "secret box" ciphertext generated by LibSodium.
   * Since the "secret box" is 16 bytes longer than the message size
   * (crypto_secretbox_MACBYTES = 16),
   * the composite ciphertext is is 40 bytes longer than the message length
   * (24 for then nonce, plus the 16 added to create the secret box)
   */
  const std::vector<unsigned char> seal(
    const unsigned char* message,
    const size_t messageLength,
    const std::string &postDecryptionInstructionsJson = ""
  ) const;
  
  const std::vector<unsigned char> seal(
    const SodiumBuffer& message,
    const std::string& postDecryptionInstructionsJson = ""
  ) const;

  const SodiumBuffer unseal(
    const unsigned char* ciphertext,
    const size_t ciphertextLength,
    const std::string &postDecryptionInstructionsJson = ""
  ) const;

  const SodiumBuffer unseal(
    const std::vector<unsigned char> &ciphertext,
    const std::string& postDecryptionInstructionsJson = ""
  ) const;

  const std::string toJson(
    int indent = -1,
    const char indent_char = ' '
  ) const;

protected:

  static SymmetricKey fromJson(
    const std::string& symmetricKeyAsJson
  );
  
  const SodiumBuffer unsealMessageContents(
    const unsigned char* ciphertext,
    const size_t ciphertextLength,
    const std::string &postDecryptionInstructionsJson = ""
  ) const;

};