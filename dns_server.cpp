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

        // Set socket option to reuse address
        int optval = 1;
        if (setsockopt(serverSocket, SOL_SOCKET, SO_REUSEADDR, &optval, sizeof(optval)) < 0) {
            throw runtime_error("Socket option setting failed");
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
            }else{
                buffer[receivedBytes] = '\0';
                cout<<"Request received : "<<buffer<<endl;
            }

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
            size_t sentBytes =sendto(serverSocket, response.c_str(), response.length(), 0, (struct sockaddr*)&clientAddress, clientLength);
            if (sentBytes < 0) 
            cerr<<"Failed to send response"<<endl;
            else if (static_cast<size_t>(sentBytes) != response.length()) 
            cerr << "Partial send: Only " << sentBytes << " of " << response.length() << " bytes sent" << endl;
            else cout<<"Response sent : "<<response<<endl;
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