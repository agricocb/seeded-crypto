#include <exception>
#include "symmetric-key.hpp"
#include "generate-seed.hpp"
#include "packaged-sealed-message.hpp"

void _crypto_secretbox_nonce_salted(
  unsigned char *nonce,
  const unsigned char *secret_key,
  const unsigned char *message,
  const size_t message_length,
  const char* salt,
  const size_t salt_length
) {
    crypto_generichash_state st;
    crypto_generichash_init(&st, secret_key, crypto_box_SECRETKEYBYTES, crypto_box_NONCEBYTES);
//    crypto_generichash_update(&st, secret_key, crypto_box_SECRETKEYBYTES);
    if (salt_length > 0) {
      crypto_generichash_update(&st, (const unsigned char*) salt, salt_length);
    }
    crypto_generichash_update(&st, message, message_length);
    crypto_generichash_final(&st, nonce, crypto_box_NONCEBYTES);
}

SymmetricKey::SymmetricKey(
  const SodiumBuffer& _keyBytes,
  const std::string _keyDerivationOptionsJson
) : keyBytes(_keyBytes), keyDerivationOptionsJson(_keyDerivationOptionsJson) {
  if (keyBytes.length != crypto_secretbox_KEYBYTES) {
    throw std::invalid_argument("Invalid key length");
  }
}

SymmetricKey::SymmetricKey(
  const SymmetricKey &other
) : SymmetricKey(other.keyBytes, other.keyDerivationOptionsJson) {}

SymmetricKey::SymmetricKey(
  const std::string& seedString,
  const std::string& _keyDerivationOptionsJson
) : SymmetricKey(
  generateSeed(
    seedString,
    _keyDerivationOptionsJson,
    KeyDerivationOptionsJson::KeyType::Symmetric,crypto_secretbox_KEYBYTES
  ),
  _keyDerivationOptionsJson
) {}

const std::vector<unsigned char> SymmetricKey::sealToCiphertextOnly(
  const unsigned char* message,
  const size_t messageLength,
  const std::string& postDecryptionInstructionsJson
) const {
  if (messageLength <= 0) {
    throw std::invalid_argument("Invalid message length");
  }
  const size_t ciphertextLength =
    crypto_secretbox_NONCEBYTES + messageLength + crypto_secretbox_MACBYTES;
  std::vector<unsigned char> ciphertext(ciphertextLength);
  unsigned char* noncePtr = ciphertext.data();
  unsigned char* secretBoxStartPtr = noncePtr + crypto_secretbox_NONCEBYTES;

  // Write a nonce derived from the message and symmeetric key
  _crypto_secretbox_nonce_salted(
    noncePtr, keyBytes.data, message, messageLength,
    postDecryptionInstructionsJson.c_str(), postDecryptionInstructionsJson.length());
  
  // Create the ciphertext as a secret box
  crypto_secretbox_easy(
    secretBoxStartPtr,
    message,
    messageLength,
    noncePtr,
    keyBytes.data
  );

  return ciphertext;
}

const std::vector<unsigned char> SymmetricKey::sealToCiphertextOnly(
  const SodiumBuffer &message,
  const std::string& postDecryptionInstructionsJson
) const {
  return sealToCiphertextOnly(message.data, message.length, postDecryptionInstructionsJson);
}

const PackagedSealedMessage SymmetricKey::seal(
  const SodiumBuffer& message,
  const std::string& postDecryptionInstructionsJson
) const {
  return PackagedSealedMessage(
    sealToCiphertextOnly(message, postDecryptionInstructionsJson),
    keyDerivationOptionsJson,
    postDecryptionInstructionsJson
  );
}

const PackagedSealedMessage SymmetricKey::seal(
  const std::vector<unsigned char>& message,
  const std::string& postDecryptionInstructionsJson
) const {
    return PackagedSealedMessage( 
      sealToCiphertextOnly(message.data(), message.size(), postDecryptionInstructionsJson),
      keyDerivationOptionsJson,
      postDecryptionInstructionsJson
  );
}


const PackagedSealedMessage SymmetricKey::seal(
  const unsigned char* message,
  const size_t messageLength,
  const std::string& postDecryptionInstructionsJson
) const {
    return PackagedSealedMessage( 
      sealToCiphertextOnly(message, messageLength, postDecryptionInstructionsJson),
      keyDerivationOptionsJson,
      postDecryptionInstructionsJson
  );
}

const SodiumBuffer SymmetricKey::unsealMessageContents(
  const unsigned char* ciphertext,
  const size_t ciphertextLength,
  const std::string& postDecryptionInstructionsJson
) const {
  if (ciphertextLength <= (crypto_secretbox_MACBYTES + crypto_secretbox_NONCEBYTES)) {
    throw std::invalid_argument("Invalid message length");
  }
  SodiumBuffer plaintextBuffer(ciphertextLength - (crypto_secretbox_MACBYTES + crypto_secretbox_NONCEBYTES));
  const unsigned char* noncePtr = ciphertext;
  const unsigned char* secretBoxStartPtr = noncePtr + crypto_secretbox_NONCEBYTES;

  const int result = crypto_secretbox_open_easy(
    plaintextBuffer.data,
    secretBoxStartPtr,
        ciphertextLength - crypto_secretbox_NONCEBYTES,
    noncePtr,
    keyBytes.data
      );
   if (result != 0) {
     throw CryptographicVerificationFailureException("Symmetric key unseal failed: the key or post-decryption instructions must be different from those used to seal the message, or the ciphertext was modified/corrupted.");
   }

  // Recalculate nonce to validate that the provided
  // postDecryptionInstructionsJson is valid 
  unsigned char recalculatedNonce[crypto_secretbox_NONCEBYTES];
  _crypto_secretbox_nonce_salted(
    recalculatedNonce, keyBytes.data, plaintextBuffer.data, plaintextBuffer.length,
    postDecryptionInstructionsJson.c_str(), postDecryptionInstructionsJson.length()
  );
  if (memcmp(recalculatedNonce, noncePtr, crypto_secretbox_NONCEBYTES) != 0) {
     throw CryptographicVerificationFailureException("Symmetric key unseal failed: the key or post-decryption instructions must be different from those used to seal the message, or the ciphertext was modified/corrupted.");
  }

  return plaintextBuffer;
}

const SodiumBuffer SymmetricKey::unseal(
  const unsigned char* ciphertext,
  const size_t ciphertextLength,
  const std::string& postDecryptionInstructionsJson
) const {
  return unsealMessageContents(ciphertext, ciphertextLength, postDecryptionInstructionsJson);
};

const SodiumBuffer SymmetricKey::unseal(
  const std::vector<unsigned char> &ciphertext,
  const std::string& postDecryptionInstructionsJson
) const {
  return unseal(ciphertext.data(), ciphertext.size(), postDecryptionInstructionsJson);
}

const SodiumBuffer SymmetricKey::unseal(
  const PackagedSealedMessage &packagedSealedMessage
) const {
  return unseal(packagedSealedMessage.ciphertext, packagedSealedMessage.postDecryptionInstructionJson);
}

/* static */const SodiumBuffer SymmetricKey::unseal(
  const PackagedSealedMessage& packagedSealedMessage,
  const std::string& seedString
) {
  return SymmetricKey(seedString, packagedSealedMessage.keyDerivationOptionsJson)
    .unseal(packagedSealedMessage.ciphertext, packagedSealedMessage.postDecryptionInstructionJson);
}


namespace SymmetricKeyJsonField {
  const std::string keyBytes = "keyBytes";
  const std::string keyDerivationOptionsJson = "keyDerivationOptionsJson";
}

SymmetricKey SymmetricKey::fromJson(
  const std::string& symmetricKeyAsJson
) {
  try {
    nlohmann::json jsonObject = nlohmann::json::parse(symmetricKeyAsJson);
    return SymmetricKey(
      SodiumBuffer::fromHexString(jsonObject.at(SymmetricKeyJsonField::keyBytes)),
      jsonObject.value(SymmetricKeyJsonField::keyDerivationOptionsJson, "")
    );
  } catch (nlohmann::json::exception e) {
    throw JsonParsingException(e.what());
  }
}

const std::string SymmetricKey::toJson(
  int indent,
  const char indent_char
) const {
  nlohmann::json asJson;
  asJson[SymmetricKeyJsonField::keyBytes] = keyBytes.toHexString();
  if (keyDerivationOptionsJson.size() > 0) {
    asJson[SymmetricKeyJsonField::keyDerivationOptionsJson] = keyDerivationOptionsJson;
  }
  return asJson.dump(indent, indent_char);
};


const SodiumBuffer SymmetricKey::toSerializedBinaryForm() const {
  SodiumBuffer _keyDerivationOptionsJson = SodiumBuffer(keyDerivationOptionsJson);
  return SodiumBuffer::combineFixedLengthList({
    &keyBytes,
    &_keyDerivationOptionsJson
  });
}

SymmetricKey SymmetricKey::fromSerializedBinaryForm(SodiumBuffer serializedBinaryForm) {
  const auto fields = serializedBinaryForm.splitFixedLengthList(2);
  return SymmetricKey(fields[0], fields[1].toUtf8String());
}
