/**
 * @file comm.hpp
 * @author Jayson Sho Toma
 * @brief Supports UDP socket transmission behind the logical network configuration.
 * @version 0.1
 * @date 2022-05-04
 */

#pragma once

#include <cstdint>

#define MAX_PACKET_BUFFER_SIZE  2048

extern char recv_buffer[MAX_PACKET_BUFFER_SIZE];
extern char send_buffer[MAX_PACKET_BUFFER_SIZE];

/**
 * @brief sends UDP packet `pkt_data` from file descriptor `sock_fd`.
 *
 * @param sock_fd file descriptor
 * @param pkt_data raw packet data
 * @param pkt_size size of the data
 * @param dst_udp_port_no destination UDP port number
 * @return int size of the sent data
 */
int sendPacketOut(int sock_fd, char *pkt_data, uint32_t pkt_size, uint32_t dst_udp_port_no);
