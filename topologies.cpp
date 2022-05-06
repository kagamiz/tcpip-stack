/**
 * @file topologies.cpp
 * @author Jayson Sho Toma
 * @brief constructs topology by using graph APIs.
 * @version 0.1
 * @date 2022-05-03
 */

#include "graph.hpp"

#include "Layer2/layer2.hpp"
#include "Layer2/l2switch.hpp"

 /*

                           +----------+
                       0/4 |          |0/0
          +----------------+   R0_re  +---------------------------+
          |     40.1.1.1/24| 122.1.1.0|20.1.1.1/24                |
          |                +----------+                           |
          |                                                       |
          |                                                       |
          |                                                       |
          |40.1.1.2/24                                            |20.1.1.2/24
          |0/5                                                    |0/1
      +---+---+                                              +----+-----+
      |       |0/3                                        0/2|          |
      | R2_re +----------------------------------------------+    R1_re |
      |       |30.1.1.2/24                        30.1.1.1/24|          |
      +-------+                                              +----------+

 */
Graph *build_first_topo()
{
    Graph *topo = new Graph("Hello World Generic Graph");
    Node *R0_re = topo->addNode("R0_re");
    Node *R1_re = topo->addNode("R1_re");
    Node *R2_re = topo->addNode("R2_re");

    topo->insertLinkBetweenTwoNodes(R0_re, R1_re, "eth0/0", "eth0/1", 1);
    topo->insertLinkBetweenTwoNodes(R1_re, R2_re, "eth0/2", "eth0/3", 1);
    topo->insertLinkBetweenTwoNodes(R0_re, R2_re, "eth0/4", "eth0/5", 1);

    R0_re->setLoopbackAddress("122.1.1.0");
    R0_re->setInterfaceIPAddress("eth0/4", "40.1.1.1", 24);
    R0_re->setInterfaceIPAddress("eth0/0", "20.1.1.1", 24);

    R1_re->setLoopbackAddress("122.1.1.1");
    R1_re->setInterfaceIPAddress("eth0/1", "20.1.1.2", 24);
    R1_re->setInterfaceIPAddress("eth0/2", "30.1.1.1", 24);

    R2_re->setLoopbackAddress("122.1.1.2");
    R2_re->setInterfaceIPAddress("eth0/3", "30.1.1.2", 24);
    R2_re->setInterfaceIPAddress("eth0/5", "40.1.1.2", 24);

    topo->startPacketReceiverThread();

    return topo;
}

/*
                                       +-----------+
                                       |  H4       |
                                       | 122.1.1.4 |
                                       +----+------+
                                            |eth0/7 - 10.1.1.3/24
                                            |
                                            |eth0/1
                                       +----+----+                        +--------+
       +---------+                     |         |                        |        |
       |         |10.1.1.2/24          |   L2Sw  |eth0/2       10.1.1.1/24|  H3    |
       |  H1     +---------------------+         +------------------------+122.1.1.3|
       |122.1.1.1|eth0/5         eth0/4|         |                 eth0/6 |        |
       + --------+                     |         |                        |        |
                                       +----+----+                        +--------+
                                            |eth0/3
                                            |
                                            |
                                            |
                                            |10.1.1.4/24
                                            |eth0/8
                                      +----++------+
                                      |            |
                                      |   H2       |
                                      |122.1.1.2   |
                                      |            |
                                      +------------+
*/
Graph *build_simple_l2_switch_topo()
{
    Graph *topo = new Graph("Simple L2 Switch Demo graph");
    Node *H1 = topo->addNode("H1");
    Node *H2 = topo->addNode("H2");
    Node *H3 = topo->addNode("H3");
    Node *H4 = topo->addNode("H4");
    Node *L2SW = topo->addNode("L2SW");

    topo->insertLinkBetweenTwoNodes(H1, L2SW, "eth0/5", "eth0/4", 1);
    topo->insertLinkBetweenTwoNodes(H2, L2SW, "eth0/8", "eth0/3", 1);
    topo->insertLinkBetweenTwoNodes(H3, L2SW, "eth0/6", "eth0/2", 1);
    topo->insertLinkBetweenTwoNodes(H4, L2SW, "eth0/7", "eth0/1", 1);

    H1->setLoopbackAddress("122.1.1.1");
    H1->setInterfaceIPAddress("eth0/5", "10.1.1.2", 24);

    H2->setLoopbackAddress("122.1.1.2");
    H2->setInterfaceIPAddress("eth0/8", "10.1.1.4", 24);

    H3->setLoopbackAddress("122.1.1.3");
    H3->setInterfaceIPAddress("eth0/6", "10.1.1.1", 24);

    H4->setLoopbackAddress("122.1.1.4");
    H4->setInterfaceIPAddress("eth0/7", "10.1.1.3", 24);

    nodeSetInterfaceL2Mode(L2SW, "eth0/1", InterfaceNetworkProperty::L2Mode::ACCESS);
    nodeSetInterfaceL2Mode(L2SW, "eth0/2", InterfaceNetworkProperty::L2Mode::ACCESS);
    nodeSetInterfaceL2Mode(L2SW, "eth0/3", InterfaceNetworkProperty::L2Mode::ACCESS);
    nodeSetInterfaceL2Mode(L2SW, "eth0/4", InterfaceNetworkProperty::L2Mode::ACCESS);

    topo->startPacketReceiverThread();

    return topo;
}

/*
                                    +---------+                               +----------+
                                    |         |                               |          |
                                    |  H2     |                               |  H5      |
                                    |122.1.1.2|                               |122.1.1.5 |
                                    +---+-----+                               +-----+----+
                                        |10.1.1.2/24                                +10.1.1.5/24
                                        |eth0/3                                     |eth0/8
                                        |                                           |
                                        |eth0/7,AC,V10                              |eth0/9,AC,V10
                                  +-----+----+                                +-----+---+
                                  |          |                                |         |
   +------+---+                   |          |                                |         |                         +--------+
   |  H1      |10.1.1.1/24        |   L2SW1  |eth0/5                   eth0/14| L2SW2   |eth0/10           eth0/11|  H6    |
   |122.1.1.1 +-------------------|          |+-------------------------------|         +-------------+----------+122.1.1.6|
   +------+---+ eth0/1      eth0/2|          |TR,V10,V11            TR,V10,V11|         |AC,V10        10.1.1.6/24|        |
                            AC,V10|          |                                |         |                         +-+------+
                                  +-----+----+                                +----+----+
                                        |eth0/6                                    |eth0/12
                                        |AC,V11                                    |AC,V11
                                        |                                          |
                                        |                                          |
                                        |                                          |
                                        |                                          |eth0/13
                                        |eth0/4                                    |10.1.1.4/24
                                        |10.1.1.3/24                             +--+-----+
                                   +----+---+|                                   | H4     |
                                   |  H3     |                                   |        |
                                   |122.1.1.3|                                   |122.1.1.4|
                                   +--------+|                                   +--------+
#endif
*/
Graph *build_dualswitch_topo()
{
    Graph *topo = new Graph("Dual Switch Topo");
    Node *H1 = topo->addNode("H1");
    H1->setLoopbackAddress("122.1.1.1");
    Node *H2 = topo->addNode("H2");
    H2->setLoopbackAddress("122.1.1.2");
    Node *H3 = topo->addNode("H3");
    H3->setLoopbackAddress("122.1.1.3");
    Node *H4 = topo->addNode("H4");
    H4->setLoopbackAddress("122.1.1.4");
    Node *H5 = topo->addNode("H5");
    H5->setLoopbackAddress("122.1.1.5");
    Node *H6 = topo->addNode("H6");
    H6->setLoopbackAddress("122.1.1.6");

    // no loopback address for L2 switches
    Node *L2SW1 = topo->addNode("L2SW1");
    Node *L2SW2 = topo->addNode("L2SW2");

    topo->insertLinkBetweenTwoNodes(H1, L2SW1, "eth0/1", "eth0/2", 1);
    topo->insertLinkBetweenTwoNodes(H2, L2SW1, "eth0/3", "eth0/7", 1);
    topo->insertLinkBetweenTwoNodes(H3, L2SW1, "eth0/4", "eth0/6", 1);
    topo->insertLinkBetweenTwoNodes(L2SW1, L2SW2, "eth0/5", "eth0/14", 1);
    topo->insertLinkBetweenTwoNodes(H5, L2SW2, "eth0/8", "eth0/9", 1);
    topo->insertLinkBetweenTwoNodes(H4, L2SW2, "eth0/13", "eth0/12", 1);
    topo->insertLinkBetweenTwoNodes(H6, L2SW2, "eth0/11", "eth0/10", 1);

    H1->setInterfaceIPAddress("eth0/1", "10.1.1.1", 24);
    H2->setInterfaceIPAddress("eth0/3", "10.1.1.2", 24);
    H3->setInterfaceIPAddress("eth0/4", "10.1.1.3", 24);
    H4->setInterfaceIPAddress("eth0/13", "10.1.1.4", 24);
    H5->setInterfaceIPAddress("eth0/8", "10.1.1.5", 24);
    H6->setInterfaceIPAddress("eth0/11", "10.1.1.6", 24);

    nodeSetInterfaceL2Mode(L2SW1, "eth0/2", InterfaceNetworkProperty::L2Mode::ACCESS);
    nodeSetInterfaceVLANMembership(L2SW1, "eth0/2", 10);
    nodeSetInterfaceL2Mode(L2SW1, "eth0/7", InterfaceNetworkProperty::L2Mode::ACCESS);
    nodeSetInterfaceVLANMembership(L2SW1, "eth0/7", 10);
    nodeSetInterfaceL2Mode(L2SW1, "eth0/5", InterfaceNetworkProperty::L2Mode::TRUNK);
    nodeSetInterfaceVLANMembership(L2SW1, "eth0/5", 10);
    nodeSetInterfaceVLANMembership(L2SW1, "eth0/5", 11);
    nodeSetInterfaceL2Mode(L2SW1, "eth0/6", InterfaceNetworkProperty::L2Mode::ACCESS);
    nodeSetInterfaceVLANMembership(L2SW1, "eth0/6", 11);

    nodeSetInterfaceL2Mode(L2SW2, "eth0/14", InterfaceNetworkProperty::L2Mode::TRUNK);
    nodeSetInterfaceVLANMembership(L2SW2, "eth0/14", 10);
    nodeSetInterfaceVLANMembership(L2SW2, "eth0/14", 11);
    nodeSetInterfaceL2Mode(L2SW2, "eth0/9", InterfaceNetworkProperty::L2Mode::ACCESS);
    nodeSetInterfaceVLANMembership(L2SW2, "eth0/9", 10);
    nodeSetInterfaceL2Mode(L2SW2, "eth0/10", InterfaceNetworkProperty::L2Mode::ACCESS);
    nodeSetInterfaceVLANMembership(L2SW2, "eth0/10", 10);
    nodeSetInterfaceL2Mode(L2SW2, "eth0/12", InterfaceNetworkProperty::L2Mode::ACCESS);
    nodeSetInterfaceVLANMembership(L2SW2, "eth0/12", 11);

    topo->startPacketReceiverThread();

    return topo;
}

/**

                                        +---------|                                  +----------+
+--------+                              |         |                                  |R3        |
|R1      |eth0/1                  eth0/2|R2       |eth0/3                      eth0/4|122.1.1.3 |
|122.1.1.1+-----------------------------+122.1.1.2|+---------------------------------+          |
|        |10.1.1.1/24        10.1.1.2/24|         |11.1.1.2/24            11.1.1.1/24|          |
+--------+                              +-------+-|                                  +----------+

*/

Graph *linear_3_node_topo()
{
    Graph *topo = new Graph("3 node linerar topo");
    Node *R1 = topo->addNode("R1");
    Node *R2 = topo->addNode("R2");
    Node *R3 = topo->addNode("R3");

    topo->insertLinkBetweenTwoNodes(R1, R2, "eth0/1", "eth0/2", 1);
    topo->insertLinkBetweenTwoNodes(R2, R3, "eth0/3", "eth0/4", 1);

    R1->setLoopbackAddress("122.1.1.1");
    R1->setInterfaceIPAddress("eth0/1", "10.1.1.1", 24);

    R2->setLoopbackAddress("122.1.1.2");
    R2->setInterfaceIPAddress("eth0/2", "10.1.1.2", 24);
    R2->setInterfaceIPAddress("eth0/3", "11.1.1.2", 24);

    R3->setLoopbackAddress("122.1.1.3");
    R3->setInterfaceIPAddress("eth0/4", "11.1.1.1", 24);

    topo->startPacketReceiverThread();

    return topo;
}
