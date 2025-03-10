#include <bits/stdc++.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <cstring>
#include <string>

#include "../includes/LRUCache.h"

using namespace std;

unsigned int DNS_port=8080;
string Forward_DNS_ip="9.9.9.9";//Quad9 root server ip address
unsigned int Forward_DNS_port=53;

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
    int forward_sockfd;
    struct sockaddr_in forwarder_addr;
    //initialize a cache 
    LRUCache cache;

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
    vector<uint8_t> createDNSResponse(const DNSHeader& query_header, const string& domain, const string& ip_address ,time_t timeToLive) {
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
        long int ttl_seconds=(long int)timeToLive;
        uint32_t ttl = htonl(ttl_seconds);  
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

    vector<uint8_t> createNXDomainResponse(const DNSHeader& query_header, const string& domain) {
    vector<uint8_t> response;

    // NXDOMAIN Response Header
    DNSHeader response_header = {
        query_header.id,     // Same ID as request query
        htons(0x8383),       // Response flag with NXDOMAIN (QR=1, RCODE=3)
        htons(1),            // Question count
        htons(0),            // Answer count
        htons(0),            // Name server count
        htons(0)             // Additional record count
    };

    // Add header to response
    uint8_t* header_ptr = reinterpret_cast<uint8_t*>(&response_header);
    response.insert(response.end(), header_ptr, header_ptr + sizeof(DNSHeader));

    // Encode original domain name without compression
    vector<uint8_t> encoded_domain = encodeDomainName(domain);
    response.insert(response.end(), encoded_domain.begin(), encoded_domain.end());

    // Query type and class
    uint16_t qtype = htons(0x0001);   // A record
    uint16_t qclass = htons(0x0001);  // IN (Internet)
    response.insert(response.end(), reinterpret_cast<uint8_t*>(&qtype), reinterpret_cast<uint8_t*>(&qtype) + 2);
    response.insert(response.end(), reinterpret_cast<uint8_t*>(&qclass), reinterpret_cast<uint8_t*>(&qclass) + 2);

    return response;
    }

    vector<uint8_t> forwardQuery(const uint8_t* query_buffer, size_t query_len){
        vector<uint8_t> response(1024);
        
        ssize_t sent_len = sendto(forward_sockfd, query_buffer, query_len, 0, (struct sockaddr*)&forwarder_addr, sizeof(forwarder_addr));
        if (sent_len < 0) {
            cout<< "\033[33m"<<"Failed to forward query to root server"<< "\033[0m"<<endl;
            response.resize(0);
            return response;
        }

        socklen_t forwarder_len = sizeof(forwarder_addr);
        ssize_t recv_len = recvfrom(forward_sockfd, response.data(), response.size(), 0, (struct sockaddr*)&forwarder_addr, &forwarder_len);   
        if (recv_len < 0) {
            cout<<"Failed to receive root DNS response"<<endl;
            response.resize(0);
            return response;
        }

        response.resize(recv_len);
        return response;
    }

    // We need to handle DNS compression pointers and thread-safe buffer handling
    string extractIPv4(const vector<uint8_t>& response) {
        if (response.size() < sizeof(DNSHeader)) {
            throw runtime_error("Response too short for DNS header");
        }
        size_t offset = sizeof(DNSHeader);
        // Skip question section with compression pointer handling
        while (offset < response.size() && response[offset] != 0) {
            // Check for DNS compression pointer (top 2 bits set)
            if ((response[offset] & 0xC0) == 0xC0) {
                offset += 2;  // Skip the entire pointer
                break;
            }
            offset += response[offset] + 1;
        }
        if (offset >= response.size()) {
            throw runtime_error("Response truncated in question section");
        }
        // Skip NULL terminator if not compression pointer
        if ((response[offset-1] & 0xC0) != 0xC0) {
            offset++;
        }
        // Skip QTYPE and QCLASS
        offset += 4;
        if (offset + 12 >= response.size()) {  // Need at least 12 more bytes for answer
            throw runtime_error("Response truncated before answer section");
        }
        // Handle compressed name in answer section
        if ((response[offset] & 0xC0) == 0xC0) {
            offset += 2;  // Skip compression pointer
        } else {
            // Skip full name
            while (offset < response.size() && response[offset] != 0) {
                offset += response[offset] + 1;
            }
            offset++;  // Skip NULL terminator
        }
        // Skip TYPE and CLASS
        offset += 4;
        // Read TTL 
        uint32_t ttl;
        memcpy(&ttl, &response[offset], 4);
        ttl = ntohl(ttl);
        // Skip TTL and RDLENGTH
        offset += 6;
        if (offset + 4 > response.size()) {
            throw runtime_error("Response truncated before IP address");
        }
        // at IP address
        stringstream ss;
        ss << (int)response[offset] << "."
        << (int)response[offset + 1] << "."
        << (int)response[offset + 2] << "."
        << (int)response[offset + 3];
        
        return ss.str();
    }

    // Thread-safe buffer handling for TTL extraction
    time_t extractTTL(const vector<uint8_t>& response) {
        if (response.size() < sizeof(DNSHeader)) {
            throw runtime_error("Response too short for DNS header");
        }
        size_t offset = sizeof(DNSHeader);
        // Skip question section with compression pointer handling
        while (offset < response.size() && response[offset] != 0) {
            if ((response[offset] & 0xC0) == 0xC0) {
                offset += 2;
                break;
            }
            offset += response[offset] + 1;
        }
        if ((response[offset-1] & 0xC0) != 0xC0) {
            offset++;
        }
        // Skip QTYPE and QCLASS
        offset += 4;
        // Skip name in answer section
        if ((response[offset] & 0xC0) == 0xC0) {
            offset += 2;
        } else {
            while (offset < response.size() && response[offset] != 0) {
                offset += response[offset] + 1;
            }
            offset++;
        }
        // Skip TYPE and CLASS
        offset += 4;
        if (offset + 4 >= response.size()) {
            throw runtime_error("Response truncated before TTL");
        }
        // Read TTL
        uint32_t ttl_seconds;
        memcpy(&ttl_seconds, &response[offset], sizeof(ttl_seconds));
        ttl_seconds = ntohl(ttl_seconds);
        
        time_t current_time = time(nullptr);
        return current_time + static_cast<time_t>(ttl_seconds);
    }


public:
    DNSServer(unsigned int port) : cache(10000) {
        
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

        cout << "DNS Server up and running on port "<< "\033[1m"<<port<< "\033[0m"<< endl<<endl;

        // Create DNS query forwarding socket
        forward_sockfd = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
        if (forward_sockfd < 0) {
            close(sockfd);
            throw runtime_error("Failed to create DNS forwarding socket");
        }

        // Configure DNS query forwarder socket address
        memset(&forwarder_addr, 0, sizeof(forwarder_addr));
        forwarder_addr.sin_family = AF_INET;
        forwarder_addr.sin_port = htons(Forward_DNS_port);
        if (inet_pton(AF_INET, Forward_DNS_ip.c_str(), &forwarder_addr.sin_addr) <= 0) {
            throw runtime_error("Invalid forwarder IP address");
        }

        //set timeout option for DNS query forwarder
        struct timeval tv;
        tv.tv_sec = 5;  // 5 seconds timeout
        tv.tv_usec = 0;
        if (setsockopt(forward_sockfd, SOL_SOCKET, SO_RCVTIMEO, &tv, sizeof(tv)) < 0) {
            throw runtime_error("Failed to set socket timeout");
        }
        
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
            cout <<endl << "Received query of " << recv_len << " bytes" << endl;
            // Parse DNS header
            DNSHeader* query_header = reinterpret_cast<DNSHeader*>(buffer);
            // Decode domain name
            size_t offset = sizeof(DNSHeader);
            string domain = decodeDomainName(buffer, offset);
            // Parse query type and class
            DNSQuery* query = reinterpret_cast<DNSQuery*>(&buffer[offset]);
            cout << "Query for domain: " << domain << endl<< "Type: " << ntohs(query->qType) << ", Class: " << ntohs(query->qClass) << endl;

            // Look up domain
            pair<string,time_t> it = cache.get(domain);

            //when ip address is found in local records
            if ((it.first).size() != 0) {
                // Create and send response
                vector<uint8_t> response = createDNSResponse(*query_header, domain, it.first , it.second);
                size_t sent_len = sendto(sockfd, response.data(), response.size(), 0, (struct sockaddr*)&client_addr, client_len);
                if (sent_len < 0) {
                    cerr << "Send error" << endl;
                } else if (sent_len != response.size()) {
                    cerr << "Partial send: Only " << sent_len << " of " << response.size() << " bytes sent" << endl; 
                } else {
                    cout<< "\033[32m"<<"Responded with IP: " <<it.first<< endl<< "Sent " << sent_len << " bytes"<< "\033[0m" << endl;
                }
            } else {
                vector<uint8_t> response;
                //Didn't found the query domain's ip address in local record , forwarding dns query
                response=forwardQuery(buffer,recv_len);

                //Din't get the ip adress even after DNS query forwarding
                if(response.size()==0){
                    cout << "\033[33m"<< "Domain not found: " << domain << "\033[0m"<< endl;
                    // Create and send response
                    response = createNXDomainResponse(*query_header, domain);
                }else{
                    //Before sending it to the client , cache the ip address and TTL of this domain
                    string ipAddress=extractIPv4(response);
                    time_t timeToLive=static_cast<time_t>(extractTTL(response));
                    cache.put(domain,ipAddress,timeToLive);
                }

                size_t sent_len = sendto(sockfd, response.data(), response.size(), 0, (struct sockaddr*)&client_addr, client_len);
                if (sent_len < 0) {
                    cerr << "Send error" << endl;
                } else if (sent_len != response.size()) {
                    cerr << "Partial send: Only " << sent_len << " of " << response.size() << " bytes sent" << endl; 
                } else {
                    cout<< "\033[32m" <<"Response sent from root server"<< endl<<"Sent " << sent_len << " bytes" << "\033[0m"<< endl;
                }
            }
        }
    }

    ~DNSServer() {
        close(sockfd);
        close(forward_sockfd);
    }
};

int main() {
    try {
        DNSServer dns_server(DNS_port);
        dns_server.start();
    } catch (const exception& e) {
        cerr << "Error: " << e.what() << endl;
        return 1;
    }
    return 0;
}