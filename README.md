# An network packact analyzer (May 29 2021)

It reads packets (in hex-stream form) from files, then does the analysis. Linux and [fmtlib(8.0.0)](https://github.com/fmtlib/fmt/releases/tag/8.0.0) is needed.

Currently I only implemented the following protocals:

Broken TCP connection now supportted. It means you don't have to start a TCP connection with handshakes.

`include/npan.h` is user interface. There are two demos in `demos/` demostrating how the library should be used. Linking to npan should do.

### Physical Layer

* Ethernet v2

### Internet Layer

* IPv4
* IPv6
* ARP

### Transport Layer

* TCP
* UDP
* ICMPv6 (Partial)

### Application Layer

Since we can only guess about the protocal unless we are notified in some ways. One feasible plan is to try match a pattern.

I developed it using 2 days. Now the development is put until someday I am available. At that time, I will develop in the order:

1. 
2. 
