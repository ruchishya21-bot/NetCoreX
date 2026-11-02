cmake_minimum_required(VERSION 3.16)

project(NetCoreX LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Main application
add_executable(netcorex
    src/main.cpp
    src/packet/packet.cpp
    src/packet/packet_processor.cpp
    src/packet/packet_parser.cpp
    src/network/ip_address.cpp
    src/network/network_interface.cpp
    src/routing/route_table.cpp
    src/routing/link_state_routing.cpp
    src/packet/icmp_handler.cpp
    src/forwarding/forwarding_engine.cpp
    src/network/router.cpp
    src/network/network_simulator.cpp
    src/packet/ipv4_codec.cpp
    src/routing/control_plane.cpp
    src/forwarding/forwarding_decision.cpp
    src/network/access_control_list.cpp
    src/connection/connection_tracker.cpp
    src/nat/nat_table.cpp
    src/nat/nat_engine.cpp
    src/network/arp_engine.cpp
    src/network/ethernet_frame.cpp
    src/network/mac_address.cpp
    src/network/arp_cache.cpp
    src/network/lan_segment.cpp
    src/network/arp_network.cpp
)

target_include_directories(netcorex PRIVATE include)

# Enable testing
enable_testing()

# Packet parser tests
add_executable(netcorex_tests
    tests/packet_parser_test.cpp
    src/packet/packet_parser.cpp
)

target_include_directories(netcorex_tests PRIVATE include)
add_test(NAME PacketParserTests COMMAND netcorex_tests)

# Link-state routing tests
add_executable(routing_tests
    tests/link_state_routing_test.cpp
    src/routing/link_state_routing.cpp
    src/packet/icmp_handler.cpp
)

target_include_directories(routing_tests PRIVATE include)
add_test(NAME LinkStateRoutingTests COMMAND routing_tests)

# Control Plane integration tests
add_executable(control_plane_tests
    tests/control_plane_test.cpp
    src/routing/control_plane.cpp
    src/routing/link_state_routing.cpp
    src/packet/icmp_handler.cpp
    src/routing/route_table.cpp
    src/network/router.cpp
    src/network/network_interface.cpp
    src/network/access_control_list.cpp
    src/connection/connection_tracker.cpp
    src/nat/nat_engine.cpp
    src/nat/nat_table.cpp
    src/network/mac_address.cpp
    src/network/arp_engine.cpp
src/network/arp_cache.cpp
src/network/mac_address.cpp
)

target_include_directories(control_plane_tests PRIVATE include)
add_test(NAME ControlPlaneTests COMMAND control_plane_tests)

# Network Simulator end-to-end tests
add_executable(network_simulator_tests
    tests/network_simulator_test.cpp
    src/network/network_simulator.cpp
    src/packet/ipv4_codec.cpp
    src/network/router.cpp
    src/network/network_interface.cpp
    src/network/access_control_list.cpp
    src/routing/route_table.cpp
    src/routing/control_plane.cpp
    src/routing/link_state_routing.cpp
    src/packet/icmp_handler.cpp
    src/forwarding/forwarding_decision.cpp
    src/connection/connection_tracker.cpp
    src/nat/nat_engine.cpp
    src/nat/nat_table.cpp
    src/network/lan_segment.cpp
    src/network/ethernet_frame.cpp
    src/network/mac_address.cpp
       src/network/arp_engine.cpp
src/network/arp_cache.cpp

src/packet/packet_parser.cpp
)



add_executable(ethernet_link_tests
    tests/ethernet_link_test.cpp
    src/network/network_simulator.cpp
    src/packet/ipv4_codec.cpp
    src/network/router.cpp
    src/network/network_interface.cpp
    src/network/access_control_list.cpp
    src/routing/route_table.cpp
    src/routing/control_plane.cpp
    src/routing/link_state_routing.cpp
    src/packet/icmp_handler.cpp
    src/forwarding/forwarding_decision.cpp
    src/connection/connection_tracker.cpp
    src/nat/nat_engine.cpp
    src/nat/nat_table.cpp
    src/network/lan_segment.cpp
    src/network/ethernet_frame.cpp
    src/network/mac_address.cpp
    src/network/arp_engine.cpp
src/network/arp_cache.cpp

src/packet/packet_parser.cpp
)

target_include_directories(ethernet_link_tests PRIVATE include)

add_test(NAME ethernet_link_tests COMMAND ethernet_link_tests)

target_include_directories(network_simulator_tests PRIVATE include)
add_test(NAME NetworkSimulatorTests COMMAND network_simulator_tests)

# Forwarding Decision Engine Tests
add_executable(forwarding_decision_tests
    tests/forwarding_decision_test.cpp
    src/forwarding/forwarding_decision.cpp
    src/routing/route_table.cpp
    src/network/network_interface.cpp
    src/network/mac_address.cpp
)

target_include_directories(forwarding_decision_tests PRIVATE include)
add_test(NAME ForwardingDecisionTests COMMAND forwarding_decision_tests)

# Access Control List Tests
add_executable(access_control_list_tests
    tests/access_control_list_test.cpp
    src/network/access_control_list.cpp
)

target_include_directories(access_control_list_tests PRIVATE include)
add_test(NAME AccessControlListTests COMMAND access_control_list_tests)

# TCP Connection Tracking Tests
add_executable(connection_tracker_tests
    tests/connection_tracker_test.cpp
    src/connection/connection_tracker.cpp
)

target_include_directories(connection_tracker_tests PRIVATE include)
add_test(NAME ConnectionTrackerTests COMMAND connection_tracker_tests)

# NAT Translation Table Tests
add_executable(nat_table_tests
    tests/nat_table_test.cpp
    src/nat/nat_table.cpp
)

target_include_directories(nat_table_tests PRIVATE include)
add_test(NAME NatTableTests COMMAND nat_table_tests)

# NAT Engine Tests
add_executable(nat_engine_tests
    tests/nat_engine_test.cpp
    src/nat/nat_engine.cpp
    src/nat/nat_table.cpp
)

target_include_directories(nat_engine_tests PRIVATE include)
add_test(NAME NatEngineTests COMMAND nat_engine_tests)

# NAT End-to-End Integration Tests
add_executable(nat_integration_tests
    tests/nat_integration_test.cpp
    src/network/network_simulator.cpp
    src/packet/ipv4_codec.cpp
    src/network/router.cpp
    src/network/network_interface.cpp
    src/network/access_control_list.cpp
    src/routing/route_table.cpp
    src/forwarding/forwarding_decision.cpp
    src/connection/connection_tracker.cpp
    src/packet/icmp_handler.cpp
    src/nat/nat_engine.cpp
    src/nat/nat_table.cpp
    src/network/lan_segment.cpp
    src/network/ethernet_frame.cpp
    src/network/mac_address.cpp
    src/network/arp_engine.cpp
src/network/arp_cache.cpp

src/packet/packet_parser.cpp
)

target_include_directories(nat_integration_tests PRIVATE include)

add_test(
    NAME NatIntegrationTests
    COMMAND nat_integration_tests
)

# ARP Cache Tests
add_executable(arp_cache_tests
    tests/arp_cache_test.cpp
    src/network/mac_address.cpp
    src/network/arp_cache.cpp
)

target_include_directories(arp_cache_tests PRIVATE include)

add_test(
    NAME ArpCacheTests
    COMMAND arp_cache_tests
)

# ARP Protocol Engine Tests
add_executable(arp_engine_tests
    tests/arp_engine_test.cpp
    src/network/arp_engine.cpp
    src/network/arp_cache.cpp
    src/network/mac_address.cpp
)

target_include_directories(arp_engine_tests PRIVATE include)

add_test(
    NAME ArpEngineTests
    COMMAND arp_engine_tests
)

# Ethernet Frame Tests
add_executable(ethernet_frame_tests
    tests/ethernet_frame_test.cpp
    src/network/ethernet_frame.cpp
    src/network/mac_address.cpp
)

target_include_directories(ethernet_frame_tests PRIVATE include)

add_test(
    NAME EthernetFrameTests
    COMMAND ethernet_frame_tests
)

# LAN Segment Tests
add_executable(lan_segment_tests
    tests/lan_segment_test.cpp
    src/network/lan_segment.cpp
    src/network/ethernet_frame.cpp
    src/network/mac_address.cpp
)

target_include_directories(lan_segment_tests PRIVATE include)

add_test(
    NAME LanSegmentTests
    COMMAND lan_segment_tests
)

# ARP and Ethernet Integration Tests
add_executable(arp_network_tests
    tests/arp_network_test.cpp
    src/network/arp_network.cpp
    src/network/arp_engine.cpp
    src/network/arp_cache.cpp
    src/network/lan_segment.cpp
    src/network/ethernet_frame.cpp
    src/network/mac_address.cpp
)

target_include_directories(arp_network_tests PRIVATE include)

add_test(
    NAME ArpNetworkTests
    COMMAND arp_network_tests
)

# Network Interface Tests
add_executable(network_interface_tests
    tests/network_interface_test.cpp
    src/network/network_interface.cpp
    src/network/mac_address.cpp
)

target_include_directories(network_interface_tests PRIVATE include)

add_test(
    NAME NetworkInterfaceTests
    COMMAND network_interface_tests
)