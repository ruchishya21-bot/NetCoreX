# NetCoreX — System Architecture

## 1. Overview

NetCoreX is a C++17-based network intelligence and routing simulator designed to model fundamental networking operations, packet forwarding, routing decisions, and network infrastructure behavior.

The project implements a software-based networking environment consisting of interconnected routers, network interfaces, Ethernet links, routing tables, and simulated packet transmission.

Its primary objective is to demonstrate how networking components interact across the control plane and data plane.

## 2. Technology Stack

* Programming Language: C++17
* Build System: CMake
* Compiler: GCC 13
* Operating Environment: Ubuntu 24.04 (WSL2)
* Debugging: GDB
* Testing: CTest

## 3. High-Level Architecture

NetCoreX follows a modular architecture.

### Packet Processing Layer

Responsible for representing, parsing, serializing, and processing network packets.

Components:

* Packet model
* IPv4 packet parser
* IPv4 packet codec
* Packet processor
* ICMP error handling

### Network Layer

Models routers, interfaces, Ethernet communication, and address resolution.

Components:

* Router
* Network interfaces
* MAC address management
* ARP cache
* ARP engine
* Ethernet frame model
* LAN segment simulation
* ARP network integration

### Control Plane

Responsible for calculating network paths and maintaining routing information.

Components:

* Route table
* Link-state routing
* Dijkstra shortest-path algorithm
* Control-plane route installation

### Data Plane

Responsible for forwarding packets according to routing decisions.

Components:

* Forwarding engine
* Forwarding decision engine
* TTL handling
* Ethernet transmission
* Packet reconstruction at receiving routers

### Security and Traffic Management

Implements simplified network security and address translation mechanisms.

Components:

* Access Control Lists
* Stateful TCP connection tracking
* NAT table
* NAT engine

## 4. Packet Processing Workflow

The simulated forwarding workflow is:

1. A packet enters the network through a source router.
2. The router checks applicable firewall and ACL rules.
3. NAT translation is performed when configured and applicable.
4. The forwarding engine performs a routing-table lookup.
5. The forwarding decision validates the selected interface.
6. ARP resolves the next-hop IP address into a MAC address.
7. The IPv4 packet is serialized.
8. The packet is encapsulated in an Ethernet frame.
9. The frame is transmitted through the simulated LAN segment.
10. The receiving router validates the frame and reconstructs the IPv4 packet.
11. The process repeats until the destination network is reached or forwarding fails.

TTL is decremented during router forwarding to prevent indefinite packet circulation.

## 5. Routing Architecture

NetCoreX implements static routing and a simplified link-state control plane.

The link-state routing module uses Dijkstra's shortest-path algorithm to calculate minimum-cost paths.

The control plane installs dynamic routes into router routing tables.

The routing model supports:

* Weighted network links
* Shortest-path calculation
* Next-hop selection
* Longest-prefix route lookup
* Link failure simulation
* Alternate path installation

The implementation models link-state route calculation but does not implement the complete OSPF protocol.

## 6. Ethernet and ARP Architecture

Ethernet frames contain:

* Source MAC address
* Destination MAC address
* EtherType
* Payload

The LAN segment simulation supports broadcast and unicast frame delivery.

ARP resolution is implemented through simulated request and reply messages.

Routers maintain ARP caches containing IP-to-MAC mappings with configurable expiration.

The current ARP payload is a custom simulation representation rather than the complete IEEE ARP wire format.

## 7. Network Security

### Access Control Lists

The ACL engine supports:

* Source IP matching
* Destination IP matching
* Protocol matching
* Permit and deny actions
* Ordered first-match evaluation
* Default-deny behavior

### Stateful Firewall

The connection tracker models TCP connection states and validates simplified connection transitions.

### Network Address Translation

The NAT engine supports TCP and UDP address and port translation.

It maintains mappings between private and public network endpoints and performs outbound and inbound translation.

## 8. Diagnostics and CLI

NetCoreX provides an interactive command-line interface.

Supported commands:

| Command              | Description                                  |
| -------------------- | -------------------------------------------- |
| `show routers`       | Lists registered routers                     |
| `show interfaces R1` | Displays router interfaces                   |
| `show routes R1`     | Displays routing information                 |
| `show arp R1`        | Displays ARP cache entries                   |
| `show stats`         | Displays forwarding statistics               |
| `ping R1 R4`         | Simulates packet forwarding                  |
| `trace R1 R4`        | Displays the calculated routing path         |
| `failover`           | Simulates primary control-plane link failure |
| `help`               | Displays available commands                  |
| `exit`               | Exits the CLI                                |

## 9. Testing Strategy

The project includes dedicated test suites for individual networking components and integration workflows.

Testing covers:

* IPv4 packet parsing and serialization
* Link-state routing
* Control-plane route installation
* Network simulation
* Forwarding decisions
* ACL evaluation
* TCP connection tracking
* NAT mapping and translation
* ARP resolution
* Ethernet frame validation
* LAN segment communication
* Network interface management
* Router-to-router Ethernet links

The current regression suite contains 17 test suites, all passing in the latest verified run.

## 10. Design Goals

NetCoreX was developed with the following engineering goals:

* Modular C++ architecture
* Separation of control-plane and data-plane responsibilities
* Testable networking components
* Reusable packet-processing abstractions
* Simulated network failure and recovery
* Observable forwarding and routing behavior
* Extensible networking functionality

## 11. Implementation Scope

NetCoreX is a software-based networking simulator.

It does not currently implement:

* Physical network hardware or ASIC packet processing
* Actual network-interface packet transmission
* Full OSPF or BGP protocol implementations
* Production-grade TCP/IP stack behavior
* Complete IEEE Ethernet and ARP wire-level implementations
* Native Cisco SAI or SONiC integration

These distinctions define the current implementation scope and provide opportunities for future development.
