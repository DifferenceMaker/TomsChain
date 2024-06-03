#pragma once
#include <string>
#include <array>
#include <Transaction.h>

#include <openssl/ec.h>
#include <openssl/obj_mac.h>
#include <openssl/evp.h>
#include <openssl/rand.h>
#include <openssl/sha.h>

class Wallet {
public:
	Wallet();
	Wallet(const char* _password, std::string _walletName); // For Wallet Creation
	Wallet(const char* _Password, std::string _WalletAddress, std::string _FilePath); // For Adding an Existing Wallet
	~Wallet();

	std::pair<uint8_t, std::pair<std::array<uint8_t, 32>, std::array<uint8_t, 32>>> signTransaction(const Transaction& tx);
	
	std::array<uint8_t, 20> getWalletAddress() const { return wallet_address; };
	std::string getWalletAddressString();
	std::string getShortenedAddress();

	std::string getWalletName() { return m_walletName; };
	const int getNonce() { return nonce; };
	void incrementNonce();
	const int getBalance() { return balance; };

	void OnImGuiRenderKeysAndAddress();

public:
	// Wallet variable for GUI
	bool accountLoggedIn = false;

private:
	// Creating keys and saving them to file
	void computeWalletAddress(const unsigned char* public_key_bytes);
	void generateKeysAndAddress();
	void saveKeysToFile(const char* password, const char* private_key_hex, std::string _walletName);

	// Retrieving them from file
	void retrievalFromPrivateKey(const char* private_key_hex);
	bool decryptKeysFromFile(const char* _Password, std::string _WalletAddress, std::string _FilePath);

private:
	int nonce;
	int balance;

	std::array<uint8_t, 20> wallet_address;
	std::string m_walletName;  // 24 characters max
	char* private_key_hex = nullptr;
	char* public_key_hex = nullptr;

	/* FOR CRYPTO AND ELLIPTIC CURVE */
	BIGNUM* private_key_bn = nullptr;
	EC_POINT* public_key_point = nullptr;
	EC_KEY* key_pair = nullptr;
};