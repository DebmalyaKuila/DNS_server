# Project overview
A simple DNS Server (Domain Name System) written in C++. This project demonstrates how a DNS server operates by responding to DNS query requests and translating domain names into IP addresses. The server supports basic DNS queries and can handle requests from client following the DNS protocol.<br/>
This project is intended for educational purposes to understand the inner workings of DNS and C++ network programming. It implements fundamental DNS server functionality without the complexity of modern production DNS servers.

# Project setup
1. Clone the project by using command `git clone <project_url>`
2. In the project directory , oprn terminal and run command `g++ -o dns_server dns_server.cpp` to create executable
3. Now, run the executable and your dns server should be running on port 8080

# Testing
`echo -n "example.com" | nc -u localhost 8080` command prints something weird on terminal , its nor the ip_address or NOT_FOUND as expected.This typically happens because UDP is connectionless, so there’s no guarantee of delivery, and the data can arrive in an unexpected format,
especially if the receiving application (terminal in this case) doesn't handle the input properly.<br/>
open a new terminal and use command `printf "example.com" | nc -u localhost 8080` ( Use printf to ensure clean input )<br/>
*why this happens?*<br/>
echo automatically appends a newline (\n) to the end of its output, unless you explicitly disable it with the -n option.
printf, on the other hand, does not append a newline by default. It requires you to specify a newline explicitly if needed.
echo can introduce unintended newlines or escape sequences in its output, especially when piped or redirected.
printf gives you more precise control over the output, so when you're sending data to something like nc or a server, it’s often better to use printf because it won't introduce any unexpected characters or formatting.