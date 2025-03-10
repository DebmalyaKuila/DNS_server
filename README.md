# Project overview
A simple DNS Server (Domain Name System) with custom LRU(Least Recently Used) cache with TTL(Time To Live) handling , written in C++ . This project demonstrates how a DNS server operates by responding to DNS query requests and translating domain names into IP addresses. The server supports basic DNS queries and can handle requests from client following the standard RFC 1035 DNS protocol.
This project is intended to understand the inner workings of DNS and C++ network programming. It implements fundamental DNS server functionality .

### Workflow
1 . client sends DNS query to the server.
2 . server searches for the domain quey in it's cache and if a record is found in cache , check whther TTL has expired or not .
3 . If cache record is not expired , create a DNS response from the data in cache (domain,ip,ttl) and directly send it.
4 . If cache record is expired or not found , forward query to root server () and send the query back to client and also cache the response.

### Motivation/Need of Project
- To Understand :
    - The working of DNS requests from a client to the server, basically how data is organized and     transmitted over the network, such as the format of DNS messages and the types of DNS records.
    - understand the fundamentals of socket programming
- DNS Server do :
    - The DNS server translates human-readable domain names into IP addresses, allowing clients to access websites and services over the internet.
    - The server stores and serves different types of DNS records (A, CNAME, MX, etc.) to respond accurately to queries related to domain names, mail servers, and other resources.
- Limitations :
    - It can only handle a single client currently 
    - It can only handle A(IPV4) record type only
    - Fails to handle huge traffic load
- Enhancements :
    - Implement multithreading to handle multiple clients simultaneously
    - Serve different types of DNS records (A, AAAA, CNAME, MX, etc.)
    - To handle huge traffic , create a a load balancer to distribute traffic across multiple instances of DNS servers running on different ports
    - Rate Limiting and Anti-DDoS Protection

# Project setup
**Important Note** - This project can only run on linux enviroment and make sure you have cmake installed 
1. Clone the project by using command `git clone <project_url>`
2. From root of project directory ,open terminal to configure and build your project
    ### configure your project and build executable
        just run these commands on terminal from the root of the project
            1.mkdir build
            2.cd build
            3.cmake ..
            4.cmake --build .
3. Now, run the executable `./src/Exexutable` on terminal and your dns server should be running on port 8080

# Testing
open a new terminal and use command `dig @127.0.0.1 -p 8080 example.com `
