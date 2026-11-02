# NetCoreX — Implementation Scope and Limitations

## 1. Project Scope

NetCoreX is a C++17-based software network simulator designed to demonstrate fundamental networking concepts, routing decisions, packet forwarding, Ethernet communication, and network traffic management.

The project models selected networking behaviors in software. It is not a physical router, network processor, or production networking stack.

## 2. Current Implementation

The following capabilities are implemented:

* IPv4 packet parsing, serialization, and deserialization.
* TCP and UDP packet header modeling.
* Static routing and longest-prefix matching.
* Dijkstra-based shortest-path calculation.
* Simplified link-state control-plane route installation.
* Multi-router packet forwarding simulation.
* TTL handling and forwarding loop detection.
* Ethernet frame modeling and simulated LAN communication.
* ARP request, reply, and neighbor-cache management.
* Access Control Lists (ACLs) with permit and deny rules.
* Simplified TCP connection tracking and stateful firewall behavior.
* TCP and UDP Network Address Translation (NAT).
* Router interface statistics and command-line diagnostics.
* Alternative routing paths and simulated link-failure scenarios.

## 3. Known Limitations

### Routing Protocols

The link-state routing implementation uses Dijkstra's shortest-path algorithm. It does not implement the complete OSPF protocol, including neighbor discovery, adjacency states, LSA flooding, and protocol timers.

BGP and MPLS are not implemented.

### Packet Processing

The IPv4 codec supports a simplified packet model. TCP sequence numbers, acknowledgment numbers, and full transport-layer behavior are not implemented.

The parser does not currently validate IPv4 or TCP checksums and does not provide complete IP fragmentation and reassembly.

### Ethernet and ARP

Ethernet communication is simulated in software. The ARP message representation uses a custom serialized format rather than the complete IEEE ARP wire format.

The current router-to-router links are modeled as separate point-to-point LAN segments.

### Stateful Firewall

Connection tracking models selected TCP connection states. It does not implement the complete TCP state machine or sequence-number validation.

Connection tracking is maintained per router, so asymmetric routing can affect return-traffic validation.

### NAT

NAT supports simplified TCP and UDP address-and-port translation. It does not implement every behavior of production NAT devices or all NAT traversal mechanisms.

### End-Host Communication

The simulator models packet delivery to a directly connected destination network. It does not currently implement complete end-host devices or physical network transmission.

### Hardware Integration

NetCoreX does not directly program network processors, ASICs, physical interfaces, or hardware forwarding pipelines.

It does not currently integrate with Cisco Silicon One hardware, SAI, or SONiC.

## 4. Future Improvements

Potential future extensions include:

* More complete OSPF protocol behavior.
* BGP route exchange and policy modeling.
* MPLS label operations.
* IPv4 fragmentation and reassembly.
* Improved TCP state tracking and checksum validation.
* More realistic ARP and Ethernet wire-format serialization.
* End-host simulation and multi-access LAN support.
* Packet capture, tracing, and debugging enhancements.
* Performance benchmarking for routing and forwarding operations.
* Integration with external network simulation or emulation environments.

## 5. Testing

The project includes automated C++ test suites covering packet processing, routing, forwarding, Ethernet, ARP, ACLs, connection tracking, NAT, and network simulation.

The latest confirmed test run completed with all 17 test suites passing.

## 6. Project Positioning

NetCoreX is an educational and experimental networking software simulator. Its purpose is to demonstrate how networking components interact through modular C++ implementations, automated tests, and an interactive command-line interface.
