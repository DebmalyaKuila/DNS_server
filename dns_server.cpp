#include <bits/stdc++.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <cstring>
#include <string>

using namespace std;

unsigned int DNS_port=8080;

// Ensure packed struct for network communication
#pragma pack(push, 1)
struct DNSHeader {
    uint16_t id;         // Query ID
    uint16_t flags;      // Flags
    uint16_t quesCnt;    // Question Count
    uint16_t ansCnt;    // Answer Count
    uint16_t nsCnt;    // Authority Count
    uint16_t arCnt;    // Additional Count
};

struct DNSQuery {
    uint16_t qType;      // Query Type
    uint16_t qClass;     // Query Class
};
#pragma pack(pop)

class DNSServer {
private:
    int sockfd;
    struct sockaddr_in server_addr;
    unordered_map<string,string> dnsRecords;

    // Convert domain name to DNS wire format
    vector<uint8_t> encodeDomainName(const string& domain) {
    vector<uint8_t> encoded;
    size_t start = 0;
    for (size_t i = 0; i < domain.length(); ++i) {
        if (domain[i] == '.') {
            size_t len = i - start;
            encoded.push_back(static_cast<uint8_t>(len));
            encoded.insert(encoded.end(), domain.begin() + start, domain.begin() + i);
            start = i + 1;
        }
    }
    // Handle the last part after the final dot or if there is no dot at all
    size_t len = domain.length() - start;
    encoded.push_back(static_cast<uint8_t>(len));
    encoded.insert(encoded.end(), domain.begin() + start, domain.end());
    //push null terminator at the end 
    encoded.push_back(0);

    return encoded;
}


    // Decode domain name from DNS wire format
    string decodeDomainName(const uint8_t* buffer, size_t& offset) {
        string domain="";
        while(buffer[offset]!=0){
            int len=buffer[offset];
            offset++;
            domain.append(reinterpret_cast<const char*>(&buffer[offset]),len);
            offset+=len;
            if(buffer[offset]!=0)domain.push_back('.');
        }
        //move past the null terminator
        offset++;
        return domain;
    }

    // create DNS response
    vector<uint8_t> createDNSResponse(const DNSHeader& query_header, const string& domain, const string& ip_address) {
        vector<uint8_t> response;
        // Response header
        DNSHeader response_header = {
            query_header.id,     // Same ID as request query
            htons(0x8080),       // Response flag for standard response(QR = 1, AA = 0, Opcode = 0, RCODE = 0)
            htons(1),             // Question count
            htons(1),             // Answer count
            0,                    // Name server count
            0                     // Additional record count
        };

        // Add header to response
        uint8_t* header_ptr = reinterpret_cast<uint8_t*>(&response_header);
        response.insert(response.end(), header_ptr, header_ptr + sizeof(DNSHeader));

        // Encode domain name
        vector<uint8_t> encoded_domain = encodeDomainName(domain);
        response.insert(response.end(), encoded_domain.begin(), encoded_domain.end());
        // Query type and class (A record(for IPV4), IN (Internet) class)
        uint16_t qtype = htons(0x0001);   // A record
        uint16_t qclass = htons(0x0001);  // IN (Internet)
        response.insert(response.end(), reinterpret_cast<uint8_t*>(&qtype), reinterpret_cast<uint8_t*>(&qtype) + 2);
        response.insert(response.end(), reinterpret_cast<uint8_t*>(&qclass), reinterpret_cast<uint8_t*>(&qclass) + 2);
        
        // Answer section
        // Add the domain name again (encoded) for the answer section
        response.insert(response.end(), encoded_domain.begin(), encoded_domain.end());
        // Type A, Class IN
        uint16_t type_a = htons(0x0001);
        uint16_t class_in = htons(0x0001);
        response.insert(response.end(), reinterpret_cast<uint8_t*>(&type_a), reinterpret_cast<uint8_t*>(&type_a) + 2);
        response.insert(response.end(), reinterpret_cast<uint8_t*>(&class_in), reinterpret_cast<uint8_t*>(&class_in) + 2);

        // TTL (32-bit)
        uint32_t ttl = htonl(3600);  // 1 hour
        response.insert(response.end(), reinterpret_cast<uint8_t*>(&ttl), reinterpret_cast<uint8_t*>(&ttl) + 4);
        // RDLENGTH (4 bytes for IPv4)
        uint16_t rdlength = htons(4);
        response.insert(response.end(), reinterpret_cast<uint8_t*>(&rdlength), reinterpret_cast<uint8_t*>(&rdlength) + 2);
        // Parse and add IP address
        struct in_addr ip_addr;
        inet_pton(AF_INET, ip_address.c_str(), &ip_addr);
        uint32_t network_ip = ip_addr.s_addr;
        response.insert(response.end(), reinterpret_cast<uint8_t*>(&network_ip), reinterpret_cast<uint8_t*>(&network_ip) + 4);

        return response;
    }

public:
    DNSServer(unsigned int port) {
        // Create UDP socket
        sockfd = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
        if (sockfd < 0) {
            throw std::runtime_error("Failed to create socket");
        }

        // Set socket option to reuse address
        int optval = 1;
        if (setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &optval, sizeof(optval)) < 0) {
            throw runtime_error("Socket option setting failed");
        }

        // Configure server address
        memset(&server_addr, 0, sizeof(server_addr));
        server_addr.sin_family = AF_INET;
        server_addr.sin_addr.s_addr = INADDR_ANY;
        server_addr.sin_port = htons(port);

        // Bind socket
        if (bind(sockfd, (struct sockaddr*)&server_addr, sizeof(server_addr)) < 0) {
            throw std::runtime_error("Failed to bind socket");
        }

        cout << "DNS Server up and running on port "<<port<< endl;
        
        // Hardcoded DNS records
        dnsRecords["localhost"] = "127.0.0.1";
        dnsRecords["test.com"] = "10.0.0.1";
    }

    void start() {
        uint8_t buffer[1024];
        struct sockaddr_in client_addr;
        socklen_t client_len = sizeof(client_addr);

        while (true) {
            // Receive DNS query
            ssize_t recv_len = recvfrom(sockfd, buffer, sizeof(buffer), 0, (struct sockaddr*)&client_addr, &client_len);
            if (recv_len < 0) {
                cerr << "Receive error" << endl;
                continue;
            }
            cout << "Received query of " << recv_len << " bytes" << endl;
            // Parse DNS header
            DNSHeader* query_header = reinterpret_cast<DNSHeader*>(buffer);
            // Decode domain name
            size_t offset = sizeof(DNSHeader);
            string domain = decodeDomainName(buffer, offset);
            // Parse query type and class
            DNSQuery* query = reinterpret_cast<DNSQuery*>(&buffer[offset]);
            cout << "Query for domain: " << domain << endl<< "Type: " << ntohs(query->qType) << ", Class: " << ntohs(query->qClass) << endl;
            // Look up domain
            auto it = dnsRecords.find(domain);
            if (it != dnsRecords.end()) {
                // Create and send response
                vector<uint8_t> response = createDNSResponse(*query_header, domain, it->second);
                size_t sent_len = sendto(sockfd, response.data(), response.size(), 0, (struct sockaddr*)&client_addr, client_len);
                if (sent_len < 0) {
                    cerr << "Send error" << endl;
                } else if (sent_len != response.size()) {
                    cerr << "Partial send: Only " << sent_len << " of " << response.size() << " bytes sent" << endl; 
                } else {
                    cout << "Responded with IP: " << it->second << endl<< "Sent " << sent_len << " bytes" << endl;
                }
            } else {
                cout << "Domain not found: " << domain << endl;
                //send NXDOMAIN response here , work on it later...
            }
        }
    }

    void addRecord(const string& domain, const string& ip) {
        dnsRecords[domain] = ip;
    }

    ~DNSServer() {
        close(sockfd);
    }
};

int main() {
    try {
        DNSServer dns_server(DNS_port);
        dns_server.addRecord("example.com","194.44.34.001");
        dns_server.start();
    } catch (const exception& e) {
        cerr << "Error: " << e.what() << endl;
        return 1;
    }
    return 0;
}