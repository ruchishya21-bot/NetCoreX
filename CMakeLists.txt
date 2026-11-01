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
    src/routing/control_plane.cpp
    src/forwarding/forwarding_decision.cpp
    src/network/access_control_list.cpp
    src/connection/connection_tracker.cpp
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

add_test(
    NAME PacketParserTests
    COMMAND netcorex_tests
)

# Link-state routing tests
add_executable(routing_tests
    tests/link_state_routing_test.cpp
    src/routing/link_state_routing.cpp
    src/packet/icmp_handler.cpp
)

target_include_directories(routing_tests PRIVATE include)

add_test(
    NAME LinkStateRoutingTests
    COMMAND routing_tests
)

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
)

target_include_directories(control_plane_tests PRIVATE include)

add_test(
    NAME ControlPlaneTests
    COMMAND control_plane_tests
)

# Network Simulator end-to-end tests
add_executable(network_simulator_tests
    tests/network_simulator_test.cpp
    src/network/network_simulator.cpp
    src/network/router.cpp
    src/network/network_interface.cpp
    src/network/access_control_list.cpp
    src/routing/route_table.cpp
    src/routing/control_plane.cpp
    src/routing/link_state_routing.cpp
    src/packet/icmp_handler.cpp
    src/forwarding/forwarding_decision.cpp
    src/connection/connection_tracker.cpp
)

target_include_directories(network_simulator_tests PRIVATE include)

add_test(
    NAME NetworkSimulatorTests
    COMMAND network_simulator_tests
)

# Forwarding Decision Engine Tests
add_executable(forwarding_decision_tests
    tests/forwarding_decision_test.cpp
    src/forwarding/forwarding_decision.cpp
    src/routing/route_table.cpp
    src/network/network_interface.cpp
)

target_include_directories(forwarding_decision_tests PRIVATE include)

add_test(
    NAME ForwardingDecisionTests
    COMMAND forwarding_decision_tests
)

# Access Control List Tests
add_executable(access_control_list_tests
    tests/access_control_list_test.cpp
    src/network/access_control_list.cpp
)

target_include_directories(access_control_list_tests PRIVATE include)

add_test(
    NAME AccessControlListTests
    COMMAND access_control_list_tests
)

# TCP Connection Tracking Tests
add_executable(connection_tracker_tests
    tests/connection_tracker_test.cpp
    src/connection/connection_tracker.cpp
)

target_include_directories(connection_tracker_tests PRIVATE include)

add_test(
    NAME ConnectionTrackerTests
    COMMAND connection_tracker_tests
)