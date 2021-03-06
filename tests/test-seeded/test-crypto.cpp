#include "gtest/gtest.h"
#include <string>
#include <iostream>
#include "lib-seeded.hpp"
#include "../lib-seeded/convert.hpp"


const std::string orderedTestKey = "A1tB2rC3bD4lE5tF6bG1tH1tI1tJ1tK1tL1tM1tN1tO1tP1tR1tS1tT1tU1tV1tW1tX1tY1tZ1t";
std::string defaultTestPublicDerivationOptionsJson = R"KGO({
	"type": "UnsealingKey",
	"additionalSalt": "1"
})KGO";
std::string defaultTestSymmetricDerivationOptionsJson = R"KGO({
	"type": "SymmetricKey",
	"additionalSalt": "1"
})KGO";
std::string defaultTestSigningDerivationOptionsJson = R"KGO({
	"type": "SigningKey",
	"additionalSalt": "1"
})KGO";


TEST(Secret, FidoUseCase) {
	std::string kdo = R"KDO({
	"type": "Secret",
	"hashFunction": "Argon2id",
	"lengthInBytes": 96
})KDO";
	Secret seed(
		orderedTestKey,
		kdo
	);
	const std::string seedAsHex = toHexStr(seed.secretBytes.toVector());
	ASSERT_EQ(
		seedAsHex,
		"fe3bfee2ff3c284e68b9c93af89b42725bb9d758c6883aa216e5c181d328d99adf4d2bf86f88e9d11b31db815a4d7ef602fb14bb59761d7045640682601d2ee7db9846028739d7f2b807e263635f497d2e7e60318415b19e314830184ef1a56a"
	);
}

const std::string fastSeedJsonDerivationOptions = R"KDO({
	"type": "Secret",
	"hashFunction": "SHA256",
	"lengthInBytes": 96
})KDO";
TEST(Secret, ConvertsToJsonAndBack) {
	Secret seed(orderedTestKey, fastSeedJsonDerivationOptions);
	
	const auto serialized = seed.toJson(1, '\t');
	const auto replica = Secret::fromJson(serialized);
	ASSERT_EQ(replica.derivationOptionsJson, seed.derivationOptionsJson);
	ASSERT_STREQ(replica.secretBytes.toHexString().c_str(), seed.secretBytes.toHexString().c_str());
}

TEST(Secret, ConvertsToSerializedFormAndBack) {
	Secret seed(orderedTestKey, fastSeedJsonDerivationOptions);
	
	const auto serialized = seed.toSerializedBinaryForm();
	const auto replica = Secret::fromSerializedBinaryForm(serialized);
	ASSERT_EQ(replica.derivationOptionsJson, seed.derivationOptionsJson);
	ASSERT_STREQ(replica.secretBytes.toHexString().c_str(), seed.secretBytes.toHexString().c_str());
}

TEST(Secret, fromJsonWithoutDerivationOptions) {
	Secret seed = Secret::fromJson(R"JSON({
	"secretBytes": "0xffFE"
})JSON");

	ASSERT_EQ(seed.secretBytes.length, 2);
	ASSERT_EQ(seed.secretBytes.data[0], 0xff);
	ASSERT_EQ(seed.secretBytes.data[1], 0xfe);
	ASSERT_EQ(seed.derivationOptionsJson.length(), 0);
}


TEST(UnsealingInstructions, ThowsOnInvalidJson) {
	ASSERT_ANY_THROW(
		UnsealingInstructions("badjson")
	);
}

// TEST(UnsealingInstructions, Handles0LengthJsonObject) {
// 	ASSERT_STREQ(
// 		UnsealingInstructions("").userMustAcknowledgeThisMessage.c_str(),
// 		""
// 	);
// }

// TEST(UnsealingInstructions, HandlesEmptyJsonObject) {
// 	ASSERT_STREQ(
// 		UnsealingInstructions("{}").userMustAcknowledgeThisMessage.c_str(),
// 		""
// 	);
// }

TEST(SealingKey, GetsSealingKey) {
	const UnsealingKey testUnsealingKey(orderedTestKey, defaultTestPublicDerivationOptionsJson);
	const SealingKey testSealingKey = testUnsealingKey.getSealingKey();

	ASSERT_EQ(testSealingKey.getSealingKeyBytes().size(), 32);
}

TEST(SealingKey, GetsSealingKeyFromEmptyOptions) {
	const UnsealingKey testUnsealingKey(orderedTestKey, "{}");
	const SealingKey testSealingKey = testUnsealingKey.getSealingKey();

	ASSERT_EQ(toHexStr(testSealingKey.getSealingKeyBytes()).length(), 64);
}


TEST(UnsealingKey, ConvertsToJsonAndBack) {
	const UnsealingKey testUnsealingKey(orderedTestKey, defaultTestPublicDerivationOptionsJson);

	const std::string json = testUnsealingKey.toJson(1, '\t');
	const UnsealingKey replica = UnsealingKey::fromJson(json);
	ASSERT_EQ(replica.derivationOptionsJson, defaultTestPublicDerivationOptionsJson);
	ASSERT_EQ(toHexStr(replica.sealingKeyBytes), toHexStr(testUnsealingKey.sealingKeyBytes));
	ASSERT_EQ(replica.unsealingKeyBytes.toHexString(), testUnsealingKey.unsealingKeyBytes.toHexString());
}


TEST(UnsealingKey, ConvertsToSerializedFormAndBack) {
	const UnsealingKey testUnsealingKey(orderedTestKey, defaultTestPublicDerivationOptionsJson);

	auto serialized = testUnsealingKey.toSerializedBinaryForm();
	const UnsealingKey replica = UnsealingKey::fromSerializedBinaryForm(serialized);
	ASSERT_EQ(replica.derivationOptionsJson, defaultTestPublicDerivationOptionsJson);
	ASSERT_EQ(toHexStr(replica.sealingKeyBytes), toHexStr(testUnsealingKey.sealingKeyBytes));
	ASSERT_EQ(replica.unsealingKeyBytes.toHexString(), testUnsealingKey.unsealingKeyBytes.toHexString());
}



TEST(SealingKey, ConvertsToJsonAndBack) {
	const UnsealingKey testUnsealingKey(orderedTestKey, defaultTestPublicDerivationOptionsJson);
	const SealingKey testSealingKey = testUnsealingKey.getSealingKey();
	
	const std::string json = testSealingKey.toJson(1, '\t');
	const SealingKey replica = SealingKey::fromJson(json);
	ASSERT_EQ(replica.getDerivationOptionsJson(), defaultTestPublicDerivationOptionsJson);
	ASSERT_EQ(toHexStr(replica.getSealingKeyBytes()), toHexStr(testSealingKey.getSealingKeyBytes()));
}


TEST(SealingKey, ConvertsToSerializedFormAndBack) {
	const UnsealingKey testUnsealingKey(orderedTestKey, defaultTestPublicDerivationOptionsJson);
	const SealingKey testSealingKey = testUnsealingKey.getSealingKey();
	
	const auto serialized = testSealingKey.toSerializedBinaryForm();
	const SealingKey replica = SealingKey::fromSerializedBinaryForm(serialized);
	ASSERT_EQ(replica.getDerivationOptionsJson(), defaultTestPublicDerivationOptionsJson);
	ASSERT_EQ(toHexStr(replica.getSealingKeyBytes()), toHexStr(testSealingKey.getSealingKeyBytes()));
}


TEST(SealingKey, EncryptsAndDecrypts) {
	const UnsealingKey testUnsealingKey(orderedTestKey, defaultTestPublicDerivationOptionsJson);
	const SealingKey testSealingKey = testUnsealingKey.getSealingKey();

	const std::vector<unsigned char> messageVector = { 'y', 'o', 't', 'o' };
	const std::string unsealingInstructions = "{}";
	SodiumBuffer messageBuffer(messageVector);
	const auto sealedMessage = testSealingKey.sealToCiphertextOnly(messageBuffer, unsealingInstructions);
	const auto unsealedMessage = testUnsealingKey.unseal(sealedMessage, unsealingInstructions);
	const auto unsealedPlaintext = unsealedMessage.toVector();
	ASSERT_EQ(messageVector, unsealedPlaintext);
}

TEST(SealingKey, EncryptsAndDecryptsPackaged) {
	const UnsealingKey testUnsealingKey(orderedTestKey, defaultTestPublicDerivationOptionsJson);
	const SealingKey testSealingKey = testUnsealingKey.getSealingKey();

	const std::vector<unsigned char> messageVector = { 'y', 'o', 't', 'o' };
	const std::string unsealingInstructions = "{}";
	SodiumBuffer messageBuffer(messageVector);
	const auto sealedMessage = testSealingKey.seal(messageBuffer, unsealingInstructions);
	const auto unsealedMessage = UnsealingKey::unseal(sealedMessage, orderedTestKey);
	const auto unsealedPlaintext = unsealedMessage.toVector();
	ASSERT_EQ(messageVector, unsealedPlaintext);
}


TEST(SigningKey, GetsSigningKey) {
	SigningKey testSigningKey(orderedTestKey, defaultTestSigningDerivationOptionsJson);
	const SignatureVerificationKey testSignatureVerificationKey = testSigningKey.getSignatureVerificationKey();

	ASSERT_EQ(testSignatureVerificationKey.getKeyBytesAsHexDigits().length(), 64);
}

TEST(SigningKey, GetsSigningKeyFromEmptyOptions) {
	SigningKey testSigningKey(orderedTestKey, "{}");
	const SignatureVerificationKey testSignatureVerificationKey = testSigningKey.getSignatureVerificationKey();

	ASSERT_EQ(testSignatureVerificationKey.getKeyBytesAsHexDigits().length(), 64);
}

TEST(SigningKey, ConvertsToJsonAndBack) {
	SigningKey testKey(orderedTestKey, defaultTestSigningDerivationOptionsJson);

	const std::string json = testKey.toJson(true, 1, '\t');
	SigningKey replica = SigningKey::fromJson(json);
	ASSERT_EQ(replica.derivationOptionsJson, defaultTestSigningDerivationOptionsJson);
	ASSERT_STREQ(replica.signingKeyBytes.toHexString().c_str(), testKey.signingKeyBytes.toHexString().c_str());
	ASSERT_STREQ(toHexStr(replica.getSignatureVerificationKeyBytes()).c_str(), toHexStr(testKey.getSignatureVerificationKeyBytes()).c_str());
}


TEST(SigningKey, ConvertsToSerializedFormAndBack) {
	SigningKey testKey(orderedTestKey, defaultTestSigningDerivationOptionsJson);

	auto comactSerializedBinaryForm = testKey.toSerializedBinaryForm(true);
	auto compactCopy = SigningKey::fromSerializedBinaryForm(comactSerializedBinaryForm);
	ASSERT_EQ(compactCopy.derivationOptionsJson, testKey.derivationOptionsJson);
	ASSERT_STREQ(toHexStr(compactCopy.getSignatureVerificationKeyBytes()).c_str(), toHexStr(testKey.getSignatureVerificationKeyBytes()).c_str());
	ASSERT_STREQ(compactCopy.signingKeyBytes.toHexString().c_str(), testKey.signingKeyBytes.toHexString().c_str());

	auto fullSerializedBinaryForm = testKey.toSerializedBinaryForm(false);
	auto fullCopy = SigningKey::fromSerializedBinaryForm(fullSerializedBinaryForm);
	ASSERT_EQ(fullCopy.derivationOptionsJson, testKey.derivationOptionsJson);
	ASSERT_STREQ(toHexStr(fullCopy.getSignatureVerificationKeyBytes()).c_str(), toHexStr(testKey.getSignatureVerificationKeyBytes()).c_str());
	ASSERT_STREQ(fullCopy.signingKeyBytes.toHexString().c_str(), testKey.signingKeyBytes.toHexString().c_str());

}

TEST(SignatureVerificationKey, ConvertsToJsonAndBack) {
	SigningKey testSigningKey(orderedTestKey, defaultTestSigningDerivationOptionsJson);
	const SignatureVerificationKey testSignatureVerificationKey = testSigningKey.getSignatureVerificationKey();

	const std::string serialized = testSignatureVerificationKey.toJson(1, '\t');
	const SignatureVerificationKey replica = SignatureVerificationKey::fromJson(serialized);
	ASSERT_EQ(replica.getDerivationOptionsJson(), defaultTestSigningDerivationOptionsJson);
	ASSERT_STREQ(replica.getKeyBytesAsHexDigits().c_str(), testSignatureVerificationKey.getKeyBytesAsHexDigits().c_str());
}

TEST(SignatureVerificationKey, ConvertsToSerializedFormAndBack) {
	SigningKey testSigningKey(orderedTestKey, defaultTestSigningDerivationOptionsJson);
	const SignatureVerificationKey testSignatureVerificationKey = testSigningKey.getSignatureVerificationKey();

	const auto serialized = testSignatureVerificationKey.toSerializedBinaryForm();
	const SignatureVerificationKey replica = SignatureVerificationKey::fromSerializedBinaryForm(serialized);
	ASSERT_EQ(replica.getDerivationOptionsJson(), defaultTestSigningDerivationOptionsJson);
	ASSERT_STREQ(replica.getKeyBytesAsHexDigits().c_str(), testSignatureVerificationKey.getKeyBytesAsHexDigits().c_str());
}

TEST(SigningKey, Verification) {
	SigningKey testSigningKey(orderedTestKey, defaultTestSigningDerivationOptionsJson);
	const SignatureVerificationKey testSignatureVerificationKey = testSigningKey.getSignatureVerificationKey();

	const std::vector<unsigned char> messageVector = { 'y', 'o', 't', 'o' };
	const auto signature = testSigningKey.generateSignature(messageVector);
	const auto shouldVerifyAsTrue = testSignatureVerificationKey.verify(messageVector, signature);
	ASSERT_TRUE(shouldVerifyAsTrue);
	const std::vector<unsigned char> invalidMessageVector = { 'y', 'o', 'l', 'o' };
	const auto shouldVerifyAsFalse = testSignatureVerificationKey.verify(invalidMessageVector, signature);
	ASSERT_FALSE(shouldVerifyAsFalse);
}


TEST(SymmetricKey, EncryptsAndDecryptsWithoutUnsealingInstructions) {
	const SymmetricKey testSymmetricKey(orderedTestKey, defaultTestSymmetricDerivationOptionsJson);

	const std::vector<unsigned char> messageVector = { 'y', 'o', 't', 'o' };
	const std::string unsealingInstructions = {};
	SodiumBuffer messageBuffer(messageVector);
	const auto sealedMessage = testSymmetricKey.sealToCiphertextOnly(messageBuffer);
	const auto unsealedMessage = testSymmetricKey.unseal(sealedMessage);
	const auto unsealedPlaintext = unsealedMessage.toVector();
	ASSERT_EQ(messageVector, unsealedPlaintext);
}



TEST(SymmetricKey, ConvertsToSerializedFormAndBack) {
	const SymmetricKey testKey(orderedTestKey, defaultTestSymmetricDerivationOptionsJson);

	const auto serializedBinaryForm = testKey.toSerializedBinaryForm();
	const auto copy = SymmetricKey::fromSerializedBinaryForm(serializedBinaryForm);
	ASSERT_EQ(copy.derivationOptionsJson, defaultTestSymmetricDerivationOptionsJson);
	ASSERT_STREQ(copy.keyBytes.toHexString().c_str(), testKey.keyBytes.toHexString().c_str());
}

TEST(SymmetricKey, ConvertsToJsonAndBack) {
	const SymmetricKey testKey(orderedTestKey, defaultTestSymmetricDerivationOptionsJson);

	const std::string json = testKey.toJson(1, '\t');
	const SymmetricKey replica = SymmetricKey::fromJson(json);
	ASSERT_EQ(replica.derivationOptionsJson, defaultTestSymmetricDerivationOptionsJson);
	ASSERT_STREQ(replica.keyBytes.toHexString().c_str(), testKey.keyBytes.toHexString().c_str());
}

TEST(SymmetricKey, EncryptsAndDecrypts) {
	const SymmetricKey testSymmetricKey(orderedTestKey, defaultTestSymmetricDerivationOptionsJson);

	const std::vector<unsigned char> messageVector = { 'y', 'o', 't', 'o' };
	const std::string unsealingInstructions = "{\"userMustAcknowledgeThisMessage\": \"yoto mofo\"}";
	SodiumBuffer messageBuffer(messageVector);
	
	const auto sealedMessage = testSymmetricKey.sealToCiphertextOnly(messageBuffer, unsealingInstructions);
	const auto unsealedMessage = testSymmetricKey.unseal(sealedMessage, unsealingInstructions);
	const std::vector<unsigned char> unsealedPlaintext = unsealedMessage.toVector();
	ASSERT_EQ(messageVector, unsealedPlaintext);
}


TEST(SymmetricKey, EncryptsAndDecryptsPackaged) {
	const SymmetricKey testSymmetricKey(orderedTestKey, defaultTestSymmetricDerivationOptionsJson);

	const std::vector<unsigned char> messageVector = { 'y', 'o', 't', 'o' };
	const std::string unsealingInstructions = "{\"userMustAcknowledgeThisMessage\": \"yoto mofo\"}";
	SodiumBuffer messageBuffer(messageVector);

	const auto sealedMessage = testSymmetricKey.seal(messageBuffer, unsealingInstructions);
	const auto unsealedMessage = SymmetricKey::unseal(sealedMessage, orderedTestKey);
	const std::vector<unsigned char> unsealedPlaintext = unsealedMessage.toVector();
	ASSERT_EQ(messageVector, unsealedPlaintext);
}



TEST(SymmetricKey, EncryptsAndDecryptsPackagedAndDecryptsWithoutRederiving) {
	const SymmetricKey testSymmetricKey(orderedTestKey, defaultTestSymmetricDerivationOptionsJson);
	
	const std::vector<unsigned char> messageVector = { 'y', 'o', 't', 'o' };
	const std::string unsealingInstructions = "{\"userMustAcknowledgeThisMessage\": \"yoto mofo\"}";
	SodiumBuffer messageBuffer(messageVector);

	const auto sealedMessage = testSymmetricKey.seal(messageBuffer, unsealingInstructions);
	const auto unsealedMessage = testSymmetricKey.unseal(sealedMessage);

	const std::vector<unsigned char> unsealedPlaintext = unsealedMessage.toVector();
	ASSERT_EQ(messageVector, unsealedPlaintext);
}



TEST(PackagedSealedMessage, ConvertsToSerializedFormAndBack) {
	std::vector<unsigned char> testCiphertext({ 42 });
	PackagedSealedMessage message(testCiphertext, "no", "way");
	auto serialized = message.toSerializedBinaryForm();
	auto replica = PackagedSealedMessage::fromSerializedBinaryForm(serialized);
	
	ASSERT_EQ(replica.ciphertext.size(), 1);
	ASSERT_EQ(replica.ciphertext.data()[0], 42);
	ASSERT_STREQ(replica.derivationOptionsJson.c_str(), message.derivationOptionsJson.c_str());
	ASSERT_STREQ(replica.unsealingInstructions.c_str(), message.unsealingInstructions.c_str());
}

TEST(PackagedSealedMessage, ConvertsToJsonAndBack) {
	std::vector<unsigned char> testCiphertext({ 42 });
	PackagedSealedMessage message(testCiphertext, "no", "way");
	auto serialized = message.toJson();
	auto replica = PackagedSealedMessage::fromJson(serialized);

	ASSERT_EQ(replica.ciphertext.size(), 1);
	ASSERT_EQ(replica.ciphertext.data()[0], 42);
	ASSERT_STREQ(replica.derivationOptionsJson.c_str(), message.derivationOptionsJson.c_str());
	ASSERT_STREQ(replica.unsealingInstructions.c_str(), message.unsealingInstructions.c_str());
}
