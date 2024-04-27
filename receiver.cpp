// File: receiver.cpp
#include <iostream>
#include <boost/asio.hpp>
#include <openssl/sha.h>
#include <sstream>
#include <iomanip>

using boost::asio::ip::tcp;
namespace asio = boost::asio;
using namespace std;

// Convert hash bytes to a hexadecimal string for easy comparison.
string toHexString(const unsigned char* hash, size_t length) {
    stringstream ss;
    for (size_t i = 0; i < length; ++i) {
        ss << hex << setw(2) << setfill('0') << (int)hash[i];
    }
    return ss.str();
}

// Hash the data using SHA-256.
string hashData(const string& data) {
    unsigned char hash[SHA256_DIGEST_LENGTH];
    SHA256_CTX sha256;
    SHA256_Init(&sha256);
    SHA256_Update(&sha256, data.c_str(), data.size());
    SHA256_Final(hash, &sha256);
    return toHexString(hash, SHA256_DIGEST_LENGTH);
}

int main() {
    try {
        asio::io_context io_context;
        tcp::acceptor acceptor(io_context, tcp::endpoint(tcp::v4(), 1234));
        tcp::socket socket(io_context);
        acceptor.accept(socket);

        // Receive data and the transmitted hash from the sender.
        asio::streambuf buffer;
        asio::read_until(socket, buffer, '\n');
        istream is(&buffer);
        string data, received_hash;
        getline(is, data);
        getline(is, received_hash);

        // Recreate and rehash the received data.
        string new_hash = hashData(data);

        // Compare the new hash with the received hash and send back confirmation.
        string confirmation = (new_hash == received_hash) ? "OK" : "FAIL";
        asio::write(socket, asio::buffer(confirmation));

    } catch (exception& e) {
        cerr << "Exception: "
