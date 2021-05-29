# An network packact analyzer (May 29 2021)

It reads packets (in hex-stream form) from files, then does the analysis. [fmtlib](https://fmt.dev/latest/index.html) is needed.

Currently I only implemented the following protocals:

**Error handling is not implemented for simplicity. Assuming every packet is ordered and correctly transmitted only once.**

### Physical Layer

* Ethernet v2

### Internet Layer

* IPv4
* IPv6
* ARP (TODO)

### Transport Layer

* TCP
* UDP (TODO)
* ICMPv6 (TODO)

### Application Layer

Since we can only guess about the protocal unless we are notified in some ways. The plan is put until someday I am interested.
