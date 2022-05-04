/**
 * @file comm.hpp
 * @author Jayson Sho Toma
 * @brief
 * @version 0.1
 * @date 2022-05-04
 *
 */

#pragma once

#include <cstdint>

#define MAX_PACKET_BUFFER_SIZE  2048

extern char recv_buffer[MAX_PACKET_BUFFER_SIZE];
extern char send_buffer[MAX_PACKET_BUFFER_SIZE];

int sendPacketOut(int sock_fd, char *pkt_data, uint32_t pkt_size, uint32_t dst_udp_port_no);
