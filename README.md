# qsox

Low level networking library for C++, partly mimicking Rust's `std::net` module.

Features:

* Convenient `Ipv4Address`, `Ipv6Address`, classes with parsing, formatting, conversions to native C types, and more
* `SocketAddressV4`, `SocketAddressV6` classes which consist of an IP address and a port, also can be parsed and formatted
* `IpAddress` and `SocketAddress` classes that can hold either an IPv4 or an IPv6 address
* A `NetworkAddress` class that can hold an IP address or a domain name (+ a port), which is lazily resolved only when requested
* Simple DNS resolution via `qsox::resolver::resolve[Ipv4|Ipv6]` APIs
* `UdpSocket`, `TcpStream` and `TcpListener` classes, which are simple and user friendly interfaces for creating TCP/UDP sockets
* Endianness conversion utils (`qsox::byteswap`)
