# NetCoreX — Network Topology and Routing

## 1. Overview

NetCoreX uses a four-router topology to demonstrate packet forwarding, shortest-path routing, Ethernet communication, and link-failure recovery.

The topology contains:

* Four routers
* Four simulated point-to-point Ethernet links
* Two alternative paths between the source and destination routers
* A source LAN and destination LAN

## 2. Network Topology

```text
                    PRIMARY PATH
                 Cost 2       Cost 2

             eth12             eth24
       +-------------+   +-------------+
       |             |   |             |
       |  ROUTER-1   +---+  ROUTER-2   |
       |             |   |             |
       +------+------+   +------+------+
              |                 |
              |                 |
         eth13|                 |eth42
              |                 |
              |                 |
       +------+------+   +------+------+
       |             |   |             |
       |  ROUTER-3   +---+  ROUTER-4   |
       |             |   |             |
       +-------------+   +------+------+
          ALTERNATE             |
          PATH                  |
       Cost 5 + Cost 5       Destination
                              LAN
                          192.168.2.0/24
```

The alternate path connects Router-1 through Router-3 to Router-4.

## 3. Interface Addressing

| Router   | Interface | IPv4 Address |
| -------- | --------- | ------------ |
| Router-1 | eth12     | 10.0.0.1     |
| Router-1 | eth13     | 10.0.0.2     |
| Router-1 | ethLAN    | 192.168.1.1  |
| Router-2 | eth21     | 10.0.0.17    |
| Router-2 | eth24     | 10.0.0.18    |
| Router-3 | eth31     | 10.0.0.33    |
| Router-3 | eth34     | 10.0.0.34    |
| Router-4 | eth42     | 10.0.0.49    |
| Router-4 | eth43     | 10.0.0.50    |
| Router-4 | ethLAN    | 192.168.2.1  |

## 4. Routing Costs

The topology provides two paths.

### Primary Path

Router-1 → Router-2 → Router-4

* Router-1 to Router-2: Cost 2
* Router-2 to Router-4: Cost 2
* Total path cost: 4

### Alternate Path

Router-1 → Router-3 → Router-4

* Router-1 to Router-3: Cost 5
* Router-3 to Router-4: Cost 5
* Total path cost: 10

The link-state routing algorithm selects the minimum-cost path.

## 5. Packet Forwarding

A simulated packet travels from:

* Source IP: 192.168.1.2
* Destination IP: 192.168.2.2

Under normal conditions, the forwarding path is:

```text
Router-1 → Router-2 → Router-4
```

The simulator resolves next-hop MAC addresses using ARP, encapsulates IPv4 packets in Ethernet frames, and reconstructs them at receiving routers.

## 6. Failover Demonstration

When the Router-2–Router-4 control-plane link is removed, the control plane recalculates routes.

The new path becomes:

```text
Router-1 → Router-3 → Router-4
```

The alternate path is selected because it is now the available route to the destination network.

The simulator demonstrates route recalculation and successful forwarding after this control-plane topology change.

**Note:** The failover command removes the control-plane link and reinstalls routes. It does not physically disconnect the corresponding simulated Ethernet link.

## 7. Verification Commands

```text
show routers
show interfaces R1
show routes R1
ping R1 R4
trace R1 R4
failover
show routes R1
trace R1 R4
show stats
```

## 8. Scope

This topology is a controlled software simulation. It models router interfaces, routing decisions, Ethernet links, ARP resolution, and packet forwarding without requiring physical networking hardware.
