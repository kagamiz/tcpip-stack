/**
 * @file testapp.cpp
 * @author Jayson Sho Toma
 * @brief test application
 * @version 0.1
 * @date 2022-05-03
 */

#include "graph.hpp"
#include "nwcli.hpp"
#include "topologies.hpp"
#include "CommandParser/libcli.h"

Graph *topo;

int main()
{
    nw_init_cli();
    topo = build_first_topo();
    start_shell();
    delete topo;
    return 0;
}
