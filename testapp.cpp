/**
 * @file testapp.cpp
 * @author Jayson Sho Toma
 * @brief test application
 * @version 0.1
 * @date 2022-05-03
 */

#include <chrono>
#include <thread>

#include "graph.hpp"
#include "nwcli.hpp"
#include "topologies.hpp"
#include "CommandParser/libcli.h"

Graph *topo;

int main()
{
    nw_init_cli();
    topo = build_dualswitch_topo();

    // wait for few seconds to ensure receiver thread is ready
    std::this_thread::sleep_for(std::chrono::seconds(2));

    start_shell();
    delete topo;
    return 0;
}
