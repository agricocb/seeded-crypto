#pragma once

#include <string>
#include "sodium-buffer.hpp"
#include "packaged-sealed-message.hpp"

/**
 * @brief A SymmetricKey can be used to seal and unseal messages.
 * This SymmetricKey class can be (re) derived from a seed using
 * set of derivation options specified in @ref derivation_options_format.
 * So, you can use this symmetric-key to seal a message, throw the
 * key away, and re-generate the key when you need to unseal the
 * message so long as you still have the original seed and
 * derivationOptionsJson.
 *  
 * Sealing a message (_plaintext_) creates a _ciphertext which contains
 * the message but from which observers who do not have the UnsealingKey
 * cannot discern the contents of the message.
 * Sealing also provides integrity-protection, which will preven the
 * message from being unsealed if it is modified.
 * We use the verbs seal and unseal, rather than encrypt and decrypt,
 * because the encrypting alone does not confer that the message includes
 * an integrity (message authentication) code to prove that the ciphertext
 * has not been tampered with.
 * 
 * The seal operation is built on LibSodium's crypto_secretbox_easy function,
 * but despite it's name the construct isn't as easy as it should be.
 * The caller must store both the ciphertext AND a 24-byte nonce
 * (crypto_secretbox_NONCEBYTES = 24).
 * Hence, the SymmetricKey seal operation outputs a _composite_ ciphertext
 * containing the nonce followed by the "secret box" ciphertext generated by LibSodium.
 * Since the "secret box" is 16 bytes longer than the message size
 * (crypto_secretbox_MACBYTES = 16),
 * the composite ciphertext is is 40 bytes longer than the message length
 * (24 for then nonce, plus the 16 added to create the secret box)
 * 
 * @ingroup DerivedFromSeeds
 */
class SymmetricKey {

  public:
  /**
   * @brief The binary representation of the symmetric key
   * 
   */
  const SodiumBuffer keyBytes;
  /**
   * @brief A @ref derivation_options_format string used to specify how this key is derived.
   */
  const std::string derivationOptionsJson;

  /**
   * @brief Construct a SymmetricKey from its members
   */
  SymmetricKey(
    const SodiumBuffer& keyBytes,
    std::string derivationOptionsJson
  );

  /**
   * @brief Construct a SymmetricKey by copying another one.
   */
  SymmetricKey(
    const SymmetricKey &other
  );

  // /**
  //  * @brief Construct (reconstitute) a SymmetricKey from its JSON
  //  * representation
  //  * 
  //  * @param symmetricKeyAsJson A SymmetricKey serialized in JSON format
  //  * via a previous call to toJson
  //  */
  // SymmetricKey(
  //   const std::string& symmetricKeyAsJson
  // );

    /**
   * @brief Construct a new SymmetricKey by (re)deriving it from a seed string
   * and a set of derivation options in @ref derivation_options_format.
   * 
   * @param seedString The private seed which is used to generate the key.
   * Anyone who knows (or can guess) this seed can re-generate the key
   * by passing it along with the derivationOptionsJson.
   * @param derivationOptionsJson The derivation options in @ref derivation_options_format.
   * 
   */
  SymmetricKey(
    const std::string& seedString,
    const std::string& derivationOptionsJson
  );

  /**
   * @brief Seal a plaintext message
   * 
   * @param message The plaintxt message to seal 
   * @param messageLength The length of the plaintext message in bytes
   * @param unsealingInstructions If this optional string is
   * passed, the same string must be passed to unseal the message.
   * @return const std::vector<unsigned char> The sealed _ciphertext_
   * without the additional context needed to unseal
   * (the derivationOptionsJson required to re-derive the key and
   * any unsealingInstructions which must match on unsealing.)

   */
  const std::vector<unsigned char> sealToCiphertextOnly(
    const unsigned char* message,
    const size_t messageLength,
    const std::string& unsealingInstructions = {}
  ) const;

  /**
   * @brief Seal a plaintext message
   * 
   * @param message The plaintxt message to seal 
   * @param unsealingInstructions If this optional string is
   * passed, the same string must be passed to unseal the message.
   * It can be used to pair a sealed message with public instructions
   * about what should happen after the message is unsealed.
   * @return const std::vector<unsigned char> The sealed _ciphertext_
   * without the additional context needed to unseal
   * (the derivationOptionsJson required to re-derive the key and
   * any unsealingInstructions which must match on unsealing.)
   */  
  const std::vector<unsigned char> sealToCiphertextOnly(
    const SodiumBuffer& message,
    const std::string& unsealingInstructions = {}
  ) const;

  /**
   * @brief Seal a plaintext message
   * 
   * @param message The plaintxt message to seal 
   * @param unsealingInstructions If this optional string is
   * passed, the same string must be passed to unseal the message.
   * It can be used to pair a sealed message with public instructions
   * about what should happen after the message is unsealed.
   * @return PackagedSealedMessage Everything needed to re-derive
   * the SymmetricKey from the seed (except the seed string iteslf)
   * and unseal the message.
   */  
  const PackagedSealedMessage seal(
    const SodiumBuffer& message,
    const std::string& unsealingInstructions = {}
  ) const;

    /**
   * @brief Seal a plaintext message
   * 
   * @param message The plaintxt message to seal 
   * @param unsealingInstructions If this optional string is
   * passed, the same string must be passed to unseal the message.
   * It can be used to pair a sealed message with public instructions
   * about what should happen after the message is unsealed.
   * @return PackagedSealedMessage Everything needed to re-derive
   * the SymmetricKey from the seed (except the seed string iteslf)
   * and unseal the message.
   */  
  const PackagedSealedMessage seal(
    const std::string& message,
    const std::string& unsealingInstructions = {}
  ) const;

  /**
   * @brief Seal a plaintext message
   * 
   * @param message The plaintxt message to seal 
   * @param unsealingInstructions If this optional string is
   * passed, the same string must be passed to unseal the message.
   * It can be used to pair a sealed message with public instructions
   * about what should happen after the message is unsealed.
   * @return PackagedSealedMessage Everything needed to re-derive
   * the SymmetricKey from the seed (except the seed string iteslf)
   * and unseal the message.
   */  
  const PackagedSealedMessage seal(
    const std::vector<unsigned char>& message,
    const std::string& unsealingInstructions = {}
  ) const;


  /**
   * @brief Seal a plaintext message
   * 
   * @param message The plaintxt message to seal
   * @param messageLength The length of the plaintext message to seal
   * @param unsealingInstructions If this optional string is
   * passed, the same string must be passed to unseal the message.
   * It can be used to pair a sealed message with public instructions
   * about what should happen after the message is unsealed.
   * @return PackagedSealedMessage Everything needed to re-derive
   * the SymmetricKey from the seed (except the seed string iteslf)
   * and unseal the message.
   */  
  const PackagedSealedMessage seal(
    const unsigned char* message,
    const size_t messageLength,
    const std::string& unsealingInstructions = {}
  ) const;

  /**
   * @brief Unseal a message 
   * 
   * @param ciphertext The sealed message to be unsealed
   * @param ciphertextLength The length of the sealed message
   * @param unsealingInstructions If this optional value was
   * set during the SymmetricKey::seal operation, the same value must
   * be provided to unseal the message or the operation will fail.
   * It can be used to pair a secret (sealed) message with public instructions
   * about what should happen after the message is unsealed.
   * @return const SodiumBuffer 
   * 
   * @exception CryptographicVerificationFailureException Thrown if the ciphertext
   * is not valid and cannot be unsealed.
   */
  const SodiumBuffer unseal(
    const unsigned char* ciphertext,
    const size_t ciphertextLength,
    const std::string& unsealingInstructions = {}
  ) const;

  /**
   * @brief Unseal a message 
   * 
   * @param ciphertext The sealed message to be unsealed
   * @param unsealingInstructions If this optional value was
   * set during the SymmetricKey::seal operation, the same value must
   * be provided to unseal the message or the operation will fail.
   * @return const SodiumBuffer 
   * 
   * @exception CryptographicVerificationFailureException Thrown if the ciphertext
   * is not valid and cannot be unsealed.
   */
  const SodiumBuffer unseal(
    const std::vector<unsigned char> &ciphertext,
    const std::string& unsealingInstructions = {}
  ) const;

  /**
   * @brief Unseal a message by re-deriving the SymmetricKey from a seed. 
   * 
   * @param packagedSealedMessage The message to be unsealed
   * @return const SodiumBuffer The plaintesxt message that had been sealed
   */
  const SodiumBuffer unseal(
    const PackagedSealedMessage& packagedSealedMessage
  ) const;

  /**
   * @brief Unseal a message by re-deriving the SymmetricKey from a seed. 
   * 
   * @param seedString The seed string used to generate the SymmetricKey that
   * sealed this message
   * @param packagedSealedMessage The message to be unsealed
   * @return const SodiumBuffer The plaintesxt message that had been sealed
   */
  static const SodiumBuffer unseal(
    const PackagedSealedMessage &packagedSealedMessage,
    const std::string& seedString
  );


  /**
   * @brief Serialize this object to a JSON-formatted string
   * 
   * It can be reconstituted by calling the constructor with this string.
   * 
   * @param indent The number of characters to indent the JSON (optional)
   * @param indent_char The character with which to indent the JSON (optional)
   * @return const std::string A SymmetricKey serialized to JSON format.
   */
  const std::string toJson(
    int indent = -1,
    const char indent_char = ' '
  ) const;


  /**
   * @brief Serialize to byte array as a list of:
   *   (keyBytes, derivationOptionsJson)
   * 
   * Stored in SodiumBuffer's fixed-length list format.
   * Strings are stored as UTF8 byte arrays.
   */
  const SodiumBuffer toSerializedBinaryForm() const;

  /**
   * @brief Deserialize from a byte array stored as a list of:
   *   (keyBytes, derivationOptionsJson)
   * 
   * Stored in SodiumBuffer's fixed-length list format.
   * Strings are stored as UTF8 byte arrays.
   */
  static SymmetricKey fromSerializedBinaryForm(SodiumBuffer serializedBinaryForm);

  /**
   * @brief Internal implementation of JSON parser for the JSON contructor
   */
  static SymmetricKey fromJson(
    const std::string& symmetricKeyAsJson
  );

protected:

  /**
   * @brief Internal implementation of unseal
   */
  const SodiumBuffer unsealMessageContents(
    const unsigned char* ciphertext,
    const size_t ciphertextLength,
    const std::string& unsealingInstructions = {}
  ) const;

};