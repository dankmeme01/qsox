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

## Examples

Working with IP addresses

```cpp
#include <qsox/IpAddress.hpp>

// Creating IPv4 addresses

qsox::Ipv4Address ip{1, 1, 1, 1};
// or
std::array<uint8_t, 4> octets{1, 1, 1, 1};
qsox::Ipv4Address ip{octets};
// or
qsox::Ipv4Address ip = qsox::Ipv4Address::fromBits(0x01010101);
// or parsing from string
auto res = qsox::Ipv4Address::parse("1.1.1.1");
if (!res) {
    std::cerr << "Failed to parse: " << res.unwrapErr().message() << '\n';
    return;
}

qsox::Ipv4Address ip = res.unwrap();
std::cout << ip.toString() << '\n'; // prints 1.1.1.1

// Same with IPv6

qsox::Ipv6Address ip = qsox::Ipv6Address::LOCALHOST;
qsox::Ipv6Address ip{ /* 16 octets ... */ };
auto res = qsox::Ipv6Address::parse("[2a00:5b7f::931]");
if (!res) {
    std::cerr << "Failed to parse: " << res.unwrapErr().message() << '\n';
    return;
}

qsox::Ipv6Address ip = res.unwrap();

// IpAddress can be used if both v4 and v6 work for you

qsox::IpAddress ip = qsox::Ipv4Address::LOCALHOST;
std::cout << "ipv4: " << ip.isV4() << ' ' << ip.asV4().toString() << '\n';

// Parsing works as well

auto res = qsox::IpAddress::parse("1.1.1.1");
// ...
```

Working with socket addresses

```cpp
#include <qsox/SocketAddress.hpp>

// Construct from IP and port
qsox::SocketAddressV4 addr1{qsox::Ipv4Address{1, 1, 1, 1}, 53};
qsox::SocketAddress addr2{qsox::Ipv6Address::UNSPECIFIED, 80};

std::cout << addr1.toString() << '\n'; // prints '1.1.1.1:53'
std::cout << addr2.toString() << '\n'; // prints '[::]:80'

qsox::IpAddress ip = addr2.ip();
uint16_t port = addr2.port();
```

DNS resolution

```cpp
#include <qsox/Resolver.hpp>

auto res = qsox::resolver::resolve("github.com");
// or, resolveIpv4 / resolveIpv6 for specific family

if (!res) {
    std::cerr << "DNS resolution failed: " << res.unwrapErr().message() << '\n';
    return;
}

IpAddress ip = res.unwrap();
std::cout << ip.toString() << '\n';
```

Basic UDP echo server

```cpp
#include <qsox/UdpSocket.hpp>

int main() {
    // we can use bindAny to bind to unspecified address with random port,
    // alternatively use `bind` to bind to a specific address and/or port
    auto socket = qsox::UdpSocket::bindAny().unwrap();

    qsox::SocketAddress sender{};
    char buf[1024];

    while (true) {
        auto result = socket.recvFrom(buf, 1024, sender);
        if (!result) {
            std::cerr << "failed to receive packet: " << result.unwrapErr().message() << '\n';
            break;
        }

        size_t count = result.unwrap();

        // in a real application make sure to handle the error returned!
        socket.sendTo(buf, count, sender);
    }
}
```

Basic single connection TCP echo server

```cpp
#include <qsox/TcpListener.hpp>

int main() {
    auto socket = qsox::TcpListener::bind(qsox::SocketAddress::parse("0.0.0.0:4000").unwrap());

    while (true) {
        auto res = socket.accept();
        if (!res) {
            std::cerr << "failed to accept connection: " << res.unwrapErr().message() << '\n';
            break;
        }

        auto [stream, address] = std::move(res).unwrap();

        char buf[1024];

        while (true) {
            auto result = stream.receive(buf, 1024);
            if (!result) {
                std::cerr << "failed to receive data: " << result.unwrapErr().message() << '\n';
                break;
            }

            size_t count = result.unwrap();

            // in a real application make sure to handle the error returned!
            auto res = socket.send(buf, count);
            if (!res) {
                std::cerr << "Connection terminated: " << res.unwrapErr().message() << std::endl;
                break;
            }
        }
    }
}
```

Basic TCP client

```cpp
#include <qsox/TcpListener.hpp>

int timeoutMs = 2500;
auto res = qsox::TcpStream::connect(qsox::SocketAddress::parse("127.0.0.1:4000").unwrap(), timeoutMs);

if (!res) {
    std::cerr << "Failed to connect: " << res.unwrapErr().message() << '\n';
    return;
}

auto stream = std::move(res).unwrap();

// get/set various options
auto fd = stream.handle();
auto localAddr = stream.localAddress();
auto remoteAddr = stream.remoteAddress();
stream.setNoDelay(true).unwrap();
stream.setLinger(true, 100).unwrap();
stream.setNonBlocking(true).unwrap();
stream.setReadTimeout(1000).unwrap();
stream.setWriteTimeout(1000).unwrap();

auto data = "hi";

// `sendAll` is almost always recommended to avoid data loss.
// if you have your own buffering, you can use `send` instead,
// whose behavior is closer to the send syscall, and will return the amount of bytes written
stream.sendAll(data, sizeof(data) - 1);

char buf[256];

// `receive` will receive as much data is readily available, or block until a packet arrives (unless nonBlocking is enabled)
// to receive an exact amount of bytes, use `receiveExact`
size_t count = stream.receive(buf, 256).unwrap();

// shutdown the connection
stream.shutdown(qsox::ShutdownMode::Both).unwrap();
```