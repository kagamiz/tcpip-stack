/**
 * @file testapp.cpp
 * @author Jayson Sho Toma
 * @brief test application
 * @version 0.1
 * @date 2022-05-03
 */

#include "graph.hpp"

extern Graph *build_first_topo();

int main()
{
    Graph *g = build_first_topo();
    g->dump();
    delete g;
    return 0;
}