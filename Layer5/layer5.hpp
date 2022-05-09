/**
 * @file layer5.hpp
 * @author your name (you@domain.com)
 * @brief
 * @version 0.1
 * @date 2022-05-10
 *
 * @copyright Copyright (c) 2022
 *
 */

#pragma once

#include "../graph.hpp"
#include "../net.hpp"

void promotePacketToLayer5(Node *node, Interface *intf, char *payload, uint32_t app_data_size, uint32_t l5_protocol_number);
