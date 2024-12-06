#include <bits/stdc++.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <cstring>
#include <string>

using namespace std;

unsigned int DNS_port=8080;

class DNSServer {
private:
    int serverSocket;
    struct sockaddr_in serverAddress;
    map<string,string> dnsRecords;

public:
    DNSServer(int port) {
        // Create UDP socket
        serverSocket = socket(AF_INET, SOCK_DGRAM, 0);
        if (serverSocket < 0) {
            throw runtime_error("Socket creation failed");
        }

        // Configure server address
        memset(&serverAddress, 0, sizeof(serverAddress));
        serverAddress.sin_family = AF_INET;
        serverAddress.sin_addr.s_addr = INADDR_ANY;
        serverAddress.sin_port = htons(port);

        // Bind socket to address
        if (bind(serverSocket, (struct sockaddr*)&serverAddress, sizeof(serverAddress)) < 0) {
            throw runtime_error("Binding failed");
        }

        // Populate some initial DNS records
        dnsRecords["localhost"] = "127.0.0.1";
        dnsRecords["test.com"] = "10.0.0.1";
    }

    void addRecord(const string& domain, const string& ip) {
        dnsRecords[domain] = ip;
    }

    void start() {
        char buffer[1024];
        struct sockaddr_in clientAddress;
        socklen_t clientLength = sizeof(clientAddress);

        std::cout << "DNS Server up and running on port "<<DNS_port<<endl;

        while (true) {
            // Receive DNS query
            ssize_t receivedBytes = recvfrom(serverSocket, buffer, sizeof(buffer), 0, (struct sockaddr*)&clientAddress, &clientLength);
            if (receivedBytes < 0) {
                cerr << "Receive error" << endl;
                continue;
            }

            buffer[receivedBytes] = '\0';
            string domain(buffer);

            // Look up IP for domain
            string response;
            auto it = dnsRecords.find(domain);
            if (it != dnsRecords.end()) {
                response = it->second;
            } else {
                response = "NOT_FOUND";
            }

            // Send response back to client
            sendto(serverSocket, response.c_str(), response.length(), 0, (struct sockaddr*)&clientAddress, clientLength);
        }
    }
    ~DNSServer() {
        close(serverSocket);
    }
};

int main() {
    try {
        DNSServer server(DNS_port);
        server.addRecord("test2.com", "192.16.0.19");
        server.start();
    } catch (const exception& e) {
        cerr << "Error: " << e.what() << endl;
        return 1;
    }
    return 0;
}


// PROBLEM --->  echo -n "example.com" | nc -u localhost 8080 prints something weird like "i??"  ,its not the ip_address or NOT_FOUND.

//SOLUTION --->
// This typically happens because of how the UDP socket handles data transmission.UDP is connectionless, so there’s no guarantee of delivery, and the data can arrive 
//in an unexpected format, especially if the receiving application (terminal in this case) doesn't handle the input properly.
// Key changes:
// 1.Added cleanDomain() method to sanitize input
// 2.Uses memset() to clear buffer before each receive
// 3.Explicitly handles buffer size to prevent unexpected characters

// Recommended testing method:
// # Use printf to ensure clean input
// printf "example.com" | nc -u localhost 8080

//echo automatically appends a newline (\n) to the end of its output, unless you explicitly disable it with the -n option.
//printf, on the other hand, does not append a newline by default. It requires you to specify a newline explicitly if needed.

//echo can introduce unintended newlines or escape sequences in its output, especially when piped or redirected.
//printf gives you more precise control over the output, so when you're sending data to something like nc or a server, it’s often better to use printf because it won't introduce any unexpected characters or formatting.