// File: sender.cpp
#include <iostream>
#include <boost/asio.hpp>
#include <openssl/sha.h>
#include <sstream>
#include <iomanip>

using boost::asio::ip::tcp;
namespace asio = boost::asio;
using namespace std;

// Convert hash bytes to a hexadecimal string for easy transmission and comparison.
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

        // Setup the TCP connection using Boost.Asio.
        tcp::resolver resolver(io_context);
        auto endpoints = resolver.resolve("127.0.0.1", "1234");
        tcp::socket socket(io_context);
        asio::connect(socket, endpoints);

        // Simulate scanning and hashing the data.
        string data = "Hello, world!";
        string hash = hashData(data);

        // Send data and hash over the TCP connection.
        asio::write(socket, asio::buffer(data + "\n" + hash));

        // Wait for confirmation from the receiver.
        char reply[128];
        size_t reply_length = asio::read(socket, asio::buffer(reply, 128));
        string response(reply, reply_length);
        if (response == "OK") {
            cout << "Match confirmed: Destroy original object" << endl;
            data.clear(); // Simulating destruction of original data.
        } else {
            cout << "No match: Original data retained" << endl;
        }
    } catch (exception& e) {
        cerr << "Exception: " << e.what() << "\n";
    }
    return 0;
}
