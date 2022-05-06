/**
 * @file topologies.hpp
 * @author Jayson Sho Toma
 * @brief constructs topology by using graph APIs.
 * @version 0.1
 * @date 2022-05-04
 */

#pragma once

#include "graph.hpp"

Graph *build_first_topo();
Graph *build_simple_l2_switch_topo();
Graph *build_dualswitch_topo();
