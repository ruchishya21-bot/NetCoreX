# CMake generated Testfile for 
# Source directory: /home/ruchi/projects/NetCoreX
# Build directory: /home/ruchi/projects/NetCoreX/build
# 
# This file includes the relevant testing commands required for 
# testing this directory and lists subdirectories to be tested as well.
add_test(PacketParserTests "/home/ruchi/projects/NetCoreX/build/netcorex_tests")
set_tests_properties(PacketParserTests PROPERTIES  _BACKTRACE_TRIPLES "/home/ruchi/projects/NetCoreX/CMakeLists.txt;37;add_test;/home/ruchi/projects/NetCoreX/CMakeLists.txt;0;")
add_test(LinkStateRoutingTests "/home/ruchi/projects/NetCoreX/build/routing_tests")
set_tests_properties(LinkStateRoutingTests PROPERTIES  _BACKTRACE_TRIPLES "/home/ruchi/projects/NetCoreX/CMakeLists.txt;47;add_test;/home/ruchi/projects/NetCoreX/CMakeLists.txt;0;")
add_test(ControlPlaneTests "/home/ruchi/projects/NetCoreX/build/control_plane_tests")
set_tests_properties(ControlPlaneTests PROPERTIES  _BACKTRACE_TRIPLES "/home/ruchi/projects/NetCoreX/CMakeLists.txt;61;add_test;/home/ruchi/projects/NetCoreX/CMakeLists.txt;0;")
