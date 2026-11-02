# NetCoreX

### A C++17-Based Network Intelligence and Routing Simulator

NetCoreX is a modular network simulation project built using **C++17**. It models how routers make forwarding decisions, resolve neighboring devices, process packets, manage routes, and handle network traffic across interconnected networks.

The project combines a simplified link-state control plane with an Ethernet-based forwarding simulation, ARP resolution, firewall rules, connection tracking, and Network Address Translation (NAT).

## Key Features

### 1. Packet Processing

* IPv4 packet parsing and serialization.
* TCP, UDP, and ICMP protocol identification.
* Packet header and payload handling.
* TTL management and expiration.

### 2. Routing and Control Plane

* Static routing and longest-prefix matching.
* Dijkstra-based shortest-path calculation.
* Simplified link-state route installation.
* Primary and alternate routing paths.
* Simulated link-failure and route reconvergence.

### 3. Ethernet and ARP

* Ethernet frame modeling.
* Simulated LAN communication.
* ARP request and reply processing.
* Dynamic neighbor-cache management.
* MAC address-based frame delivery.

### 4. Data Plane and Forwarding

* Multi-router packet forwarding.
* Next-hop resolution using ARP.
* Interface validation and forwarding decisions.
* Packet TTL updates.
* Forwarding statistics.

### 5. Network Security and Traffic Management

* Access Control Lists (ACLs).
* Permit and deny rules.
* Simplified TCP connection tracking.
* Stateful firewall behavior.
* TCP and UDP NAT translation.

### 6. Interactive Command-Line Interface

NetCoreX provides commands to inspect and interact with the simulated network.

| Command              | Description                                           |
| -------------------- | ----------------------------------------------------- |
| `show routers`       | Display registered routers                            |
| `show interfaces R1` | Display router interfaces                             |
| `show routes R1`     | Display routing table                                 |
| `show arp R1`        | Display ARP cache                                     |
| `show stats`         | Display forwarding statistics                         |
| `ping R1 R4`         | Simulate packet forwarding from R1 toward R4          |
| `trace R1 R4`        | Display the route-table-based forwarding path         |
| `failover`           | Simulate primary-path failure and route recalculation |
| `help`               | Display available commands                            |
| `exit`               | Close the simulator                                   |

## Network Topology

NetCoreX uses a four-router topology with two alternative paths between the source and destination networks.

```text
                 Primary Path
              Cost 2       Cost 2
         R1 ---------- R2 ---------- R4
          \                         /
           \                       /
            -------- R3 ----------
                 Alternate Path
                   Cost 5 + 5
```

* Primary path: R1 → R2 → R4
* Alternate path: R1 → R3 → R4
* Source network: `192.168.1.0/24`
* Destination network: `192.168.2.0/24`

The control plane calculates the shortest path using Dijkstra's algorithm. When the primary control-plane link is removed, routes are recalculated to use the alternate path.

See [`docs/network-topology.md`](docs/network-topology.md) for addressing and topology details.

## Architecture

The project is organized into modular components:

```text
NetCoreX/
├── include/
│   ├── packet/
│   ├── network/
│   ├── routing/
│   ├── forwarding/
│   ├── connection/
│   └── nat/
├── src/
│   ├── packet/
│   ├── network/
│   ├── routing/
│   ├── forwarding/
│   ├── connection/
│   ├── nat/
│   └── main.cpp
├── tests/
├── docs/
│   ├── architecture.md
│   ├── network-topology.md
│   └── limitations.md
└── CMakeLists.txt
```

The implementation separates packet processing, network interfaces, routing, forwarding, connection tracking, and NAT into independent modules.

For more details, see [`docs/architecture.md`](docs/architecture.md).

## Technology Stack

* **Language:** C++17
* **Build System:** CMake
* **Compiler:** GCC 13
* **Development Environment:** Ubuntu 24.04 (WSL2)
* **Debugging:** GDB
* **Testing:** CTest

## Build and Run

### Prerequisites

* C++17-compatible compiler
* CMake 3.20 or later
* Git

### Clone the repository

```bash
git clone <YOUR_GITHUB_REPOSITORY_URL>
cd NetCoreX
```

Replace `<YOUR_GITHUB_REPOSITORY_URL>` with your actual GitHub repository URL.

### Build

```bash
cmake -S . -B build
cmake --build build
```

### Run

```bash
./build/netcorex
```

### Run automated tests

```bash
ctest --test-dir build --output-on-failure
```

The latest confirmed test run completed with **17 out of 17 test suites passing**.

## Engineering Concepts Demonstrated

* Object-oriented design in C++.
* Graph algorithms and Dijkstra's shortest-path algorithm.
* Routing tables and longest-prefix matching.
* Packet parsing and serialization.
* Ethernet frame processing and MAC addressing.
* ARP resolution and neighbor caching.
* Control-plane and data-plane separation.
* Stateful connection tracking.
* Network address translation.
* Modular architecture and automated testing.

## Implementation Scope

NetCoreX is a software-based network simulator, not a physical router or production networking stack.

Its link-state implementation is a simplified routing model rather than a complete OSPF implementation. It does not currently implement BGP, MPLS, ASIC programming, physical packet transmission, or Cisco Silicon One hardware integration.

See [`docs/limitations.md`](docs/limitations.md) for the detailed implementation scope and future improvements.

## Project Status

**Current status:** Core simulation implementation completed; automated test suites passing.

Future development may extend the simulator with additional routing protocols, more realistic packet processing, and expanded network emulation capabilities.
