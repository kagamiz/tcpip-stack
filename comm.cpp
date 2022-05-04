/**
 * @file comm.cpp
 * @author Jayson Sho Toma
 * @brief
 * @version 0.1
 * @date 2022-05-04
 */

#include "comm.hpp"

#include <arpa/inet.h>
#include <netdb.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <unistd.h>

char recv_buffer[MAX_PACKET_BUFFER_SIZE];
char send_buffer[MAX_PACKET_BUFFER_SIZE];

int sendPacketOut(int sock_fd, char *pkt_data, uint32_t pkt_size, uint32_t dst_udp_port_no)
{
    sockaddr_in dest_addr;

    hostent *host = reinterpret_cast<hostent *>(gethostbyname("127.0.0.1"));
    dest_addr.sin_family = AF_INET;
    dest_addr.sin_port = dst_udp_port_no;
    dest_addr.sin_addr = *(reinterpret_cast<in_addr *>(host->h_addr));

    int rc = sendto(sock_fd, pkt_data, pkt_size, 0, reinterpret_cast<sockaddr *>(&dest_addr), sizeof(sockaddr));

    return rc;
}
