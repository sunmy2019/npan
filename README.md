# An network packact analyzer (May 29 2021)

It reads packets (in hex-stream form) from files, then does the analysis. [fmtlib 7.1.3](https://fmt.dev/latest/index.html) is needed.

Currently I only implemented the following protocals:

Broken TCP connection now supportted. It means you don't have to start a TCP connection with handshakes.

### Physical Layer

* Ethernet v2

### Internet Layer

* IPv4
* IPv6

### Transport Layer

* TCP
* UDP

### Application Layer

Since we can only guess about the protocal unless we are notified in some ways. One feasible plan is to try match a pattern.

I developed it using 2 days. Now the development is put until someday I am available. At that time, I will develop in the order:

1. ARP
2. 
3. 
