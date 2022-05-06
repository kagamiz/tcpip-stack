/**
 * @file layer2.hpp
 * @author Jayson Sho Toma
 * @brief defines layer 2 related implementation
 * @version 0.1
 * @date 2022-05-04
 */

#pragma once

#include <cstdint>
#include <cstring>
#include <list>
#include <string>

#include "../graph.hpp"
#include "../net.hpp"
#include "../printer.hpp"

#pragma pack(push,1)

struct ARPHeader {
    int16_t hw_type;                /* 1 for ethernet cable */
    int16_t proto_type;             /* 0x800 for IPv4 */
    int8_t  hw_addr_len;            /* 6 for MAC */
    int8_t  proto_addr_len;         /* 4 for IPv4 */
    int16_t op_code;                /* req or reply */
    MACAddress src_mac;             /* MAC of OIF interface */
    uint32_t src_ip;                /* IP of OIF */
    MACAddress dst_mac;             /* ? */
    uint32_t dst_ip;                /* IP for which ARP is being resolved */
};

struct EthernetHeader {
    MACAddress dst_mac;
    MACAddress src_mac;
    uint16_t type;
    uint8_t payload[248]; /* Max allowed : 1500 */
    uint32_t FCS;
};
#pragma pack(pop)

#define ETH_HDR_SIZE_EXCL_PAYLOAD       (sizeof(EthernetHeader) - sizeof(EthernetHeader::payload))
#define ETH_FCS(eth_hdr_ptr, payload_size) ( *(uint32_t *)((char *)((EthernetHeader *)eth_hdr_ptr)->payload + payload_size) )

static inline EthernetHeader *ALLOC_ETH_HEADER_WITH_PAYLOAD(char *packet, uint32_t packet_size)
{
    char *head_position = packet - ETH_HDR_SIZE_EXCL_PAYLOAD;
    char *iterator = head_position;
    // dst_mac
    memset(iterator, 0, sizeof(MACAddress));
    iterator += sizeof(MACAddress);
    // src_mac
    memset(iterator, 0, sizeof(MACAddress));
    iterator += sizeof(MACAddress);
    // type
    memset(iterator, 0, sizeof(uint16_t));
    iterator += sizeof(uint16_t);
    // payload
    memmove(iterator, packet, packet_size);
    iterator += packet_size;
    // FCS
    memset(iterator, 0, sizeof(uint32_t));

    return (EthernetHeader *)head_position;
}

static inline bool l2FrameRecvQualifyOnInterface(Interface *intf, EthernetHeader *ethernet_hdr)
{
    if (!intf->isL3Mode()) {
        if (intf->getL2Mode() == InterfaceNetworkProperty::L2Mode::L2_MODE_UNKOWN) {
            return false;
        }
        return true;
    }

    /* Return true if receiving machine must accept the frame */
    if (intf->getMACAddress() == ethernet_hdr->dst_mac) {
        return true;
    }

    if (ethernet_hdr->dst_mac == MACAddress::BROADCAST_MAC_ADDRESS) {
        return true;
    }

    return false;
}

void layer2FrameRecv(Node *node, Interface *interface, char *packet, uint32_t packet_size);

/* ARP Table APIs */
struct ARPEntry {

    ARPEntry();

    IPAddress ip_addr;
    MACAddress mac_addr;
    std::string oif_name;
};

class ARPTable : public IPrinter {
public:
    ARPTable() {}

    static ARPTable *getNewTable()
    {
        return new ARPTable();
    }

    bool addEntry(ARPEntry *arp_entry);
    ARPEntry *arpTableLookup(const std::string &ip_addr);
    void updateFromARPReply(ARPHeader *arp_header, Interface *iif);
    void deleteEntry(const std::string &ip_addr);

    virtual void dump() const override;

private:
    std::list<ARPEntry> arp_table;
};

ARPTable *getNewARPTable();
void deleteARPTable(ARPTable *arp_table);
void sendARPBroadcastRequest(Node *node, Interface *oif, const std::string &ip_addr);

/* VLAN support */
#pragma pack(push, 1)
struct VLAN8021QHeader {
    uint16_t tpid;  /* = 0x8100 */
    uint16_t tci_pcp : 3; /* initial 4 bits are not used in this code */
    uint16_t tci_dei : 1;
    uint16_t tci_vid : 12; /* tagged VLAN id */
    uint32_t getVLANID() const
    {
        return tci_vid;
    }
};

struct VLANEthernetHeader {
    MACAddress dst_mac;
    MACAddress src_mac;
    VLAN8021QHeader vlan_8021q_header;
    uint16_t type;
    uint8_t payload[248];
    uint32_t FCS;
};
#pragma pack(pop)

#define VLAN_ETH_HDR_SIZE_EXCL_PAYLOAD       (sizeof(VLANEthernetHeader) - sizeof(VLANEthernetHeader::payload))
#define VLAN_ETH_FCS(vlan_eth_hdr_ptr, payload_size) ( *(uint32_t *)((char *)((VLANEthernetHeader *)vlan_eth_hdr_ptr)->payload + payload_size) )

static inline VLAN8021QHeader *isPacketVLANTagged(EthernetHeader *ethernet_header)
{
    if (ethernet_header->type == 0x8100) {
        return reinterpret_cast<VLAN8021QHeader *>(ethernet_header->type);
    }
    return nullptr;
}

static inline uint8_t *getEthernetHeaderPayload(EthernetHeader *ethernet_header)
{
    if (VLAN8021QHeader *p = isPacketVLANTagged(ethernet_header); p) {
        return reinterpret_cast<VLANEthernetHeader *>(ethernet_header)->payload;
    }
    return ethernet_header->payload;
}

#define GET_COMMON_ETH_FCS(eth_hdr_ptr, payload_size)( *(uint32_t *)((char *)(getEthernetHeaderPayload(eth_hdr_ptr) + payload_size) )

static inline void setCommonEthernetFCS(EthernetHeader *ethernet_header, uint32_t payload_size, uint32_t new_fcs)
{
    if (VLAN8021QHeader *p = isPacketVLANTagged(ethernet_header); p) {
        reinterpret_cast<VLANEthernetHeader *>(ethernet_header)->FCS = new_fcs;
        return;
    }
    ethernet_header->FCS = new_fcs;
}

static inline uint32_t getEthernetHeaderSizeExcludingPayload(EthernetHeader *ethernet_header)
{
    if (VLAN8021QHeader *p = isPacketVLANTagged(ethernet_header); p) {
        return VLAN_ETH_HDR_SIZE_EXCL_PAYLOAD;
    }
    return ETH_HDR_SIZE_EXCL_PAYLOAD;
}

VLANEthernetHeader *tagPacketWithVLANID(EthernetHeader *ethernet_header, uint32_t total_packet_size, int32_t vlan_id, uint32_t *new_packet_size);
EthernetHeader *untagPacketWithVLANID(EthernetHeader *ethernet_header, uint32_t total_packet_size, uint32_t *new_packet_size);
