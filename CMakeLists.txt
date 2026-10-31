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
    src/forwarding/forwarding_engine.cpp
    src/network/router.cpp
    src/network/network_simulator.cpp
    src/routing/control_plane.cpp
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
)

target_include_directories(routing_tests PRIVATE include)

add_test(NAME LinkStateRoutingTests COMMAND routing_tests)

# Control Plane integration tests
add_executable(control_plane_tests
    tests/control_plane_test.cpp
    src/routing/control_plane.cpp
    src/routing/link_state_routing.cpp
    src/routing/route_table.cpp
    src/network/router.cpp
    src/network/network_interface.cpp
)

target_include_directories(control_plane_tests PRIVATE include)

add_test(NAME ControlPlaneTests COMMAND control_plane_tests)