#pragma once
#pragma warning(disable : 4996)

#include <fstream>
#include "Wallet.h"
#include <iostream>
#include <sstream>
#include <iomanip>
#include <filesystem>
#include "ImGui/imgui.h"


static void hexToBytes(const char* hex, unsigned char* bytes, size_t byteCount) {
	for (size_t i = 0; i < byteCount; ++i) {
		sscanf(hex + (i * 2), "%2hhx", &bytes[i]);
	}
}

static void bytesToHex(const unsigned char* bytes, size_t length, char* hex, size_t hexSize) {
	for (size_t i = 0; i < length; ++i) {
		sprintf_s(hex + (i * 2), hexSize - (i * 2), "%02X", bytes[i]); // hexSize - (i*2)?
	}
	hex[length * 2] = '\0';
}

// Constructors 

Wallet::Wallet(){}

Wallet::Wallet(const char* _password, std::string _walletName){ // Wallet creation
	generateKeysAndAddress();

    std::cout << "Private key hex: " << private_key_hex << std::endl;
    std::cout << "Public key hex: " << public_key_hex << std::endl;
    std::cout << "_walletName " << _walletName << std::endl;
    std::cout << "Wallet address: " << getWalletAddressString() << std::endl;

    m_walletName = _walletName;
    
    //OnImGuiRenderKeysAndAddress();

    //std::cout << _password << " and " << _walletName << std::endl;

	saveKeysToFile(_password, private_key_hex, _walletName);
}

Wallet::Wallet(const char* _Password, std::string _WalletAddress, std::string _FilePath){
    // Key Decryption
    bool correctPassword = decryptKeysFromFile(_Password, _WalletAddress, _FilePath);

    if (correctPassword) {
        accountLoggedIn = true;
    }
}

Wallet::~Wallet(){
    // Correctly deallocate memory pointers
}



void Wallet::OnImGuiRenderKeysAndAddress(){
    if (ImGui::Begin("Account Credentials")) {
        ImGui::TextColored(ImVec4(1.0f, 0.0f, 0.0f, 1.0f), "THESE ARE YOUR ACCOUNT CREDENTIALS");
        ImGui::TextColored(ImVec4(1.0f, 0.0f, 0.0f, 1.0f), "BE SURE TO SAVE THESE AND DON'T DISCLOSE THE PRIVATE KEY");

        ImGui::Text("private key: %s", private_key_hex);
        ImGui::Text("public key: %s", public_key_hex);
        ImGui::Text("wallet address: %S", wallet_address);

        if (ImGui::Button("Close")) {
            ImGui::CloseCurrentPopup();
        }
    }ImGui::End();    
}

std::pair<uint8_t, std::pair<std::array<uint8_t, 32>, std::array<uint8_t, 32>>> Wallet::signTransaction(const Transaction& tx){
    // Create EC_KEY structure and load private key
    EC_KEY* key = EC_KEY_new_by_curve_name(NID_secp256k1);
    EC_KEY_set_private_key(key, private_key_bn);

    // Hash the transaction data
    unsigned char hash[SHA256_DIGEST_LENGTH];
    SHA256_CTX sha256;
    SHA256_Init(&sha256);
    
    std::array<uint8_t, 32> transactionHash = tx.computeTXHash();

    // Update hash with the prepared data
    SHA256_Update(&sha256, transactionHash.data(), transactionHash.size());
    SHA256_Final(hash, &sha256);

    // Create a signature structure
    ECDSA_SIG* signature = ECDSA_do_sign(hash, sizeof(hash), key);
    const BIGNUM* r_bn, * s_bn;
    ECDSA_SIG_get0(signature, &r_bn, &s_bn);

    // Convert BIGNUMs to uint8_t arrays
    uint8_t r[32], s[32];
    BN_bn2bin(r_bn, r);
    BN_bn2bin(s_bn, s);
    
    const EC_GROUP* group = EC_KEY_get0_group(key_pair);
    BIGNUM* x = BN_new();
    BIGNUM* y = BN_new();
    uint8_t v;

    if (EC_POINT_get_affine_coordinates_GFp(group, public_key_point, x, y, NULL)) {
        // Convert BIGNUM to unsigned long (assuming y is small enough to fit)
        unsigned long y_value = BN_get_word(y);
        if (y_value != 0xFFFFFFFFFFFFFFFF) { // Check if BN_get_word failed
            v = 27 + (y_value % 2);
        }
        else {
            std::cout << "Conversion of BIGNUM of transaction signing failed \n";
        }
    }
    else {
        std::cout << "Coordinate retrieval failed \n";
    }

    BN_free(x);
    BN_free(y);

    // Convert r and s to std::array<uint8_t, 32>
    std::array<uint8_t, 32> r_array, s_array;
    memcpy(r_array.data(), r, r_array.size());
    memcpy(s_array.data(), s, s_array.size());

    return std::make_pair(v, std::make_pair(r_array, s_array));
}

std::string Wallet::getWalletAddressString(){
    std::ostringstream wallet_address_str;
    wallet_address_str << std::hex << std::setfill('0');
    for (const auto& byte : wallet_address) {
        wallet_address_str << std::setw(2) << static_cast<int>(byte);
    }

    return wallet_address_str.str();
}

std::string Wallet::getShortenedAddress(){
    std::string addressHex = getWalletAddressString();
    std::string shortenedAddress = addressHex.substr(0, 4) + " ... " + addressHex.substr(addressHex.length() - 4);
    return shortenedAddress;
}

void Wallet::incrementNonce(){
    nonce++;
    std::cout << "Current nonce: " << nonce << std::endl;
}



// -- Class Methods --

void Wallet::computeWalletAddress(const unsigned char* public_key_bytes) {
    // Perform SHA256 hash
    unsigned char hash[SHA256_DIGEST_LENGTH];
    SHA256(public_key_bytes + 1, 64, hash);

    // Extract the address bytes and save them in wallet_address
    for (size_t i = 0; i < wallet_address.size(); ++i) {
        wallet_address[i] = hash[i + 12];
    }
}

void Wallet::generateKeysAndAddress(){
    OpenSSL_add_all_algorithms();  // Initialize OpenSSL's algorithms

    EC_KEY* key_pair = EC_KEY_new_by_curve_name(NID_secp256k1);
    EC_KEY_generate_key(key_pair);

    // Get private and public keys
    const BIGNUM* private_key_bn = EC_KEY_get0_private_key(key_pair);
    const EC_POINT* public_key_point = EC_KEY_get0_public_key(key_pair);    

    // Private key hex
    private_key_hex = BN_bn2hex(private_key_bn);

    // Public key hex
    public_key_hex = BN_bn2hex(EC_POINT_point2bn(EC_KEY_get0_group(key_pair), public_key_point, POINT_CONVERSION_UNCOMPRESSED, NULL, NULL));

    // Generate address
    unsigned char public_key_bytes[65];
    EC_POINT_point2oct(EC_KEY_get0_group(key_pair), public_key_point, POINT_CONVERSION_UNCOMPRESSED, public_key_bytes, sizeof(public_key_bytes), NULL);

    computeWalletAddress(public_key_bytes);;

    EC_KEY_free(key_pair);
    EVP_cleanup();  // Clean up OpenSSL
}

void Wallet::saveKeysToFile(const char* password, const char* private_key_hex, std::string _walletName){  
    unsigned char salt[8];
    RAND_bytes(salt, sizeof(salt));

    unsigned char key[EVP_MAX_KEY_LENGTH], iv[EVP_MAX_IV_LENGTH];
    if (!EVP_BytesToKey(
            EVP_aes_256_cbc(),
            EVP_sha256(),
            salt,
            reinterpret_cast<const unsigned char*>(password),
            strlen(password),
            1000,       // Number of iterations
            key, iv)) {
        std::cout << "Error: Problem with key derivation" << std::endl;
        return;
    }

    // Convert the private key hex to bytes
    unsigned char private_key_data[32];
    hexToBytes(private_key_hex, private_key_data, sizeof(private_key_data));

    // Encrypt the private key using AES-256-CBC
    unsigned char encrypted_private_key[64]; 
    int encrypted_len = 0;
    EVP_CIPHER_CTX* ctx = EVP_CIPHER_CTX_new();
    EVP_EncryptInit_ex(ctx, EVP_aes_256_cbc(), nullptr, key, iv);
    EVP_EncryptUpdate(ctx, encrypted_private_key, &encrypted_len, private_key_data, sizeof(private_key_data));
    int final_len = 0;
    EVP_EncryptFinal_ex(ctx, encrypted_private_key + encrypted_len, &final_len);
    encrypted_len += final_len;

    // Encrypt the wallet name
    unsigned char encrypted_wallet_name[32]; // Adjust size as needed
    int wallet_name_len = 0;
    EVP_EncryptInit_ex(ctx, EVP_aes_256_cbc(), nullptr, key, iv);
    EVP_EncryptUpdate(ctx, encrypted_wallet_name, &wallet_name_len, reinterpret_cast<const unsigned char*>(_walletName.c_str()), _walletName.size());
    EVP_EncryptFinal_ex(ctx, encrypted_wallet_name + wallet_name_len, &final_len);
    wallet_name_len += final_len;
    EVP_CIPHER_CTX_free(ctx);

    std::cout << "Salt size: " << sizeof(salt) << "\n";
    std::cout << "Encrypted private key size: " << encrypted_len << "\n";
    std::cout << "Encrypted wallet name size: " << wallet_name_len << "\n";

    std::string outputDirectory = "wallets/";
    std::stringstream outputFileName;
    outputFileName << outputDirectory << getWalletAddressString() << ".bin";

    // Create the directory if it doesn't exist
    std::filesystem::create_directories(outputDirectory);
   
    std::ofstream outFile(outputFileName.str(), std::ios::binary);
    if (outFile) {
        outFile.write(reinterpret_cast<const char*>(salt), sizeof(salt));
        outFile.write(reinterpret_cast<const char*>(encrypted_private_key), encrypted_len);
        outFile.write(reinterpret_cast<const char*>(encrypted_wallet_name), wallet_name_len);
        outFile.close();
    } else {
        std::cout << "Error: Error with saving encrypted private key to file" << std::endl;
    }
}

void Wallet::retrievalFromPrivateKey(const char* _private_key_hex){ // ar private_key_hex izveido public key and address. also sets all 3 as class variables.
    OpenSSL_add_all_algorithms();
    private_key_hex = const_cast<char*>(_private_key_hex);
    // Create EC_KEY structure using the secp256k1 curve
    key_pair = EC_KEY_new_by_curve_name(NID_secp256k1);
    private_key_bn = BN_new();

    // Convert private_key_hex to BIGNUM
    BN_hex2bn(&private_key_bn, _private_key_hex);

    // Set private key
    EC_KEY_set_private_key(key_pair, private_key_bn);

    // Generate public key from private key
    public_key_point = EC_POINT_new(EC_KEY_get0_group(key_pair));
    EC_POINT_mul(EC_KEY_get0_group(key_pair), public_key_point, private_key_bn, nullptr, nullptr, nullptr);

    // Public Key Hex conversion
    char* public_key_BNtoHex = BN_bn2hex(EC_POINT_point2bn(EC_KEY_get0_group(key_pair), public_key_point, POINT_CONVERSION_UNCOMPRESSED, NULL, NULL));
    public_key_hex = public_key_BNtoHex;

    // Generate address from public key
    unsigned char public_key_bytes[65];
    size_t public_key_len = EC_POINT_point2oct(EC_KEY_get0_group(key_pair), public_key_point, POINT_CONVERSION_UNCOMPRESSED, public_key_bytes, sizeof(public_key_bytes), nullptr);

    computeWalletAddress(public_key_bytes);
    
    OPENSSL_free(public_key_BNtoHex);
    EVP_cleanup();
}

bool Wallet::decryptKeysFromFile(const char* _Password, std::string _WalletAddress, std::string _FilePath){
    std::ifstream inFile(_FilePath, std::ios::binary);
    if (!inFile) {
        std::cout << "Error: Error with OPENING encrypted file" << std::endl;
        return false;
    }

    // Read salt
    unsigned char salt[8];
    inFile.read(reinterpret_cast<char*>(salt), sizeof(salt));

    // Read the encrypted private key
    std::vector<unsigned char> encrypted_private_key(48); // Adjust size if needed
    inFile.read(reinterpret_cast<char*>(encrypted_private_key.data()), encrypted_private_key.size());
    size_t encrypted_private_key_len = inFile.gcount();

    // Read the encrypted wallet name
    std::vector<unsigned char> encrypted_wallet_name(32); // Adjust size if needed
    inFile.read(reinterpret_cast<char*>(encrypted_wallet_name.data()), encrypted_wallet_name.size());
    size_t encrypted_wallet_name_len = inFile.gcount();

    inFile.close();

    std::cout << "Salt size: " << sizeof(salt) << "\n";
    std::cout << "Encrypted private key size: " << encrypted_private_key_len << "\n";
    std::cout << "Encrypted wallet name size: " << encrypted_wallet_name_len << "\n";

    // Derive key and IV using the same salt and password
    unsigned char key[EVP_MAX_KEY_LENGTH], iv[EVP_MAX_IV_LENGTH];
    if (!EVP_BytesToKey(
            EVP_aes_256_cbc(),
            EVP_sha256(),
            salt,
            reinterpret_cast<const unsigned char*>(_Password),
            strlen(_Password),
            1000,       // Number of iterations
            key, iv)) {
        std::cerr << "Key derivation failed." << std::endl;
        return false;
    }

    // Decrypt the private key
    unsigned char decrypted_private_key[32];
    int decrypted_len = 0;
    EVP_CIPHER_CTX* ctx = EVP_CIPHER_CTX_new();
    EVP_DecryptInit_ex(ctx, EVP_aes_256_cbc(), nullptr, key, iv);
    EVP_DecryptUpdate(ctx, decrypted_private_key, &decrypted_len, encrypted_private_key.data(), encrypted_private_key_len);
    int final_len = 0;
    EVP_DecryptFinal_ex(ctx, decrypted_private_key + decrypted_len, &final_len);
    decrypted_len += final_len;

    // Decrypt the wallet name
    unsigned char decrypted_wallet_name[32]; // Adjust size as needed
    int decrypted_wallet_name_len = 0;
    EVP_DecryptInit_ex(ctx, EVP_aes_256_cbc(), nullptr, key, iv);
    EVP_DecryptUpdate(ctx, decrypted_wallet_name, &decrypted_wallet_name_len, encrypted_wallet_name.data(), encrypted_wallet_name_len);
    EVP_DecryptFinal_ex(ctx, decrypted_wallet_name + decrypted_wallet_name_len, &final_len);
    decrypted_wallet_name_len += final_len;
    EVP_CIPHER_CTX_free(ctx);

    // Convert decrypted private key bytes to a hex string
    char decrypted_private_key_hex[65]; // 64 characters + null terminator
    bytesToHex(decrypted_private_key, decrypted_len, decrypted_private_key_hex, 65);

    // Convert decrypted wallet name to a string
    std::string wallet_name(reinterpret_cast<char*>(decrypted_wallet_name), decrypted_wallet_name_len);
    m_walletName = wallet_name;

    std::cout << "Decrypted Wallet Name: " << wallet_name << "\n";

    retrievalFromPrivateKey(decrypted_private_key_hex);

    if (_WalletAddress == getWalletAddressString()) {
        std::cout << "Success: Given _WalletAddress == Derived wallet_address.\n";
        std::cout << "Decrypted Wallet Name: " << wallet_name << "\n";
        std::cout << "Private key hex: " << private_key_hex << std::endl;
        std::cout << "Public key hex: " << public_key_hex << std::endl;
        std::cout << "_walletName " << m_walletName << std::endl;
        std::cout << "Wallet address: " << getWalletAddressString() << std::endl;

        accountLoggedIn = true;

        return true;
    }
    else {
        std::cout << "Error: derived wallet_address doesn't match given address.\n";
        std::cout << "decrypted_private_key_hex: " << decrypted_private_key_hex << std::endl;
        std::cout << "wallet_name = " << wallet_name;
        return false;
    }
}
