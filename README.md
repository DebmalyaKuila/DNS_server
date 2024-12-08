# Project overview
A simple DNS Server (Domain Name System) written in C++ . This project demonstrates how a DNS server operates by responding to DNS query requests and translating domain names into IP addresses. The server supports basic DNS queries and can handle requests from client following the standard RFC 1035 DNS protocol.<br/>
This project is intended to understand the inner workings of DNS and C++ network programming. It implements fundamental DNS server functionality .

### Motivation/Need of Project

- To Understand :<br/>
    - The working of DNS requests from a client to the server, basically how data is organized and     transmitted over the network, such as the format of DNS messages and the types of DNS records.
    - understand the fundamentals of socket programming

- DNS Server do :<br/>
    - The DNS server translates human-readable domain names into IP addresses, allowing clients to access websites and services over the internet.
    - The server stores and serves different types of DNS records (A, CNAME, MX, etc.) to respond accurately to queries related to domain names, mail servers, and other resources.

- Limitations :<br/>
    - It doesn't send proper response when corresponding ip address of query domain is not found
    - It can only handle a single client currently 
    - It can only handle A(IPV4) record type only
- Enhancements :<br/>
    - Send NXDOMAIN response properly 
    - Implement multithreading to handle multiple clients simultaneously
    - Serve different types of DNS records (A, AAAA, CNAME, MX, etc.)
    - Storing DNS Records in a Database
    - DNS Caching with Expiry and TTL
    - Rate Limiting and Anti-DDoS Protection

# Project setup
**Important Note** - This project can only run on linux enviroment
1. Clone the project by using command `git clone <project_url>`
2. In the project directory , oprn terminal and run command `g++ -o dns_server dns_server.cpp` to create executable
3. Now, run the executable and your dns server should be running on port 8080

# Testing
open a new terminal and use command `dig @127.0.0.1 -p 8080 example.com `<br/>

