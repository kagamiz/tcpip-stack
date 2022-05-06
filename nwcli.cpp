/**
 * @file nwcli.cpp
 * @author Jayson Sho Toma
 * @brief CLI for accessing network library
 * @version 0.1
 * @date 2022-05-04
 */

#include <iostream>
#include <regex>
#include <string>

#include "CommandParser/libcli.h"
#include "CommandParser/cmdtlv.h"

#include "cmdcodes.hpp"
#include "color.hpp"
#include "graph.hpp"

#include "Layer2/layer2.hpp"
#include "Layer2/l2switch.hpp"

#include "Layer3/layer3.hpp"

#include "Layer5/ping.hpp"

extern Graph *topo;

/* Generic Topology Commands */
int show_nw_topology_handler(param_t *param, ser_buff_t *tlv_buf, op_mode enable_or_disable)
{
    int CMDCODE = -1;
    CMDCODE = EXTRACT_CMD_CODE(tlv_buf);

    switch (CMDCODE) {
    case CMDCODE_SHOW_NW_TOPOLOGY:
        topo->dump();
        break;
    default:
        break;
    }
    return 0;
}

/* Generic ARP Commands */
int arp_handler(param_t *param, ser_buff_t *tlv_buf, op_mode enable_or_disable)
{
    int cmd_code = EXTRACT_CMD_CODE(tlv_buf);

    tlv_struct_t *tlv = NULL;
    std::string node_name, ip_address;

    TLV_LOOP_BEGIN(tlv_buf, tlv)
    {
        if (std::string(tlv->leaf_id) == "node-name") {
            node_name = tlv->value;
        }
        if (std::string(tlv->leaf_id) == "ip-address") {
            ip_address = tlv->value;
        }
    } TLV_LOOP_END;

    switch (cmd_code) {
    case CMDCODE_RUN_RESOLVE_ARP:
    {
        Node *node = topo->getNodeByNodeName(node_name);
        sendARPBroadcastRequest(node, nullptr, ip_address);
        break;
    }
    }
    return 0;
}

int show_arp_handler(param_t *param, ser_buff_t *tlv_buf, op_mode enable_or_disable)
{
    int cmd_code = EXTRACT_CMD_CODE(tlv_buf);

    tlv_struct_t *tlv = NULL;
    std::string node_name;

    TLV_LOOP_BEGIN(tlv_buf, tlv)
    {
        if (std::string(tlv->leaf_id) == "node-name") {
            node_name = tlv->value;
        }
    } TLV_LOOP_END;

    switch (cmd_code) {
    case CMDCODE_SHOW_ARP:
    {
        Node *node = topo->getNodeByNodeName(node_name);
        node->getARPTable()->dump();
        break;
    }
    }
    return 0;
}

int show_mac_handler(param_t *param, ser_buff_t *tlv_buf, op_mode enable_or_disable)
{
    int cmd_code = EXTRACT_CMD_CODE(tlv_buf);

    tlv_struct_t *tlv = NULL;
    std::string node_name;

    TLV_LOOP_BEGIN(tlv_buf, tlv)
    {
        if (std::string(tlv->leaf_id) == "node-name") {
            node_name = tlv->value;
        }
    } TLV_LOOP_END;

    switch (cmd_code) {
    case CMDCODE_SHOW_MAC:
    {
        Node *node = topo->getNodeByNodeName(node_name);
        node->getMACTable()->dump();
        break;
    }
    }
    return 0;
}

int show_rt_handler(param_t *param, ser_buff_t *tlv_buf, op_mode enable_or_disable)
{
    int cmd_code = EXTRACT_CMD_CODE(tlv_buf);

    tlv_struct_t *tlv = NULL;
    std::string node_name;

    TLV_LOOP_BEGIN(tlv_buf, tlv)
    {
        if (std::string(tlv->leaf_id) == "node-name") {
            node_name = tlv->value;
        }
    } TLV_LOOP_END;

    switch (cmd_code) {
    case CMDCODE_SHOW_ROUTING_TABLE:
    {
        Node *node = topo->getNodeByNodeName(node_name);
        node->getRoutingTable()->dump();
        break;
    }
    }
    return 0;
}

int l3_config_handler(param_t *param, ser_buff_t *tlv_buf, op_mode enable_or_disable)
{
    int cmd_code = EXTRACT_CMD_CODE(tlv_buf);

    tlv_struct_t *tlv = NULL;
    std::string node_name, dest, mask, gw_ip, oif_name;

    TLV_LOOP_BEGIN(tlv_buf, tlv)
    {
        if (std::string(tlv->leaf_id) == "node-name") {
            node_name = tlv->value;
        }
        if (std::string(tlv->leaf_id) == "dest") {
            dest = tlv->value;
        }
        if (std::string(tlv->leaf_id) == "mask") {
            mask = tlv->value;
        }
        if (std::string(tlv->leaf_id) == "gw-ip") {
            gw_ip = tlv->value;
        }
        if (std::string(tlv->leaf_id) == "oif-name") {
            oif_name = tlv->value;
        }
    } TLV_LOOP_END;

    Node *node = topo->getNodeByNodeName(node_name);
    switch (cmd_code) {
    case CMDCODE_CONFIG_REMOTE_ROUTE:
    {
        switch (enable_or_disable) {
        case CONFIG_ENABLE:
        {
            Interface *intf = node->getNodeInterfaceByName(oif_name);
            if (!intf) {
                std::cout << "Config Error : Non existing interface : " << oif_name << std::endl;
                return -1;
            }
            if (!intf->isL3Mode()) {
                std::cout << "Config Error : Not L3 mode interface : " << oif_name << std::endl;
                return -1;
            }
            RoutingTable *routing_table = const_cast<RoutingTable *>(node->getRoutingTable());
            routing_table->addRoute(IPAddress(dest), std::stoi(mask), gw_ip, oif_name);
            break;
        }
        case CONFIG_DISABLE:
        {
            RoutingTable *routing_table = const_cast<RoutingTable *>(node->getRoutingTable());
            routing_table->deleteEntry(IPAddress(dest), std::stoi(mask));
            break;
        }
        }
    }
    }
    return 0;
}

int ping_handler(param_t *param, ser_buff_t *tlv_buf, op_mode enable_or_disable)
{
    int cmd_code = EXTRACT_CMD_CODE(tlv_buf);

    tlv_struct_t *tlv = NULL;
    std::string node_name, ip_address;

    TLV_LOOP_BEGIN(tlv_buf, tlv)
    {
        if (std::string(tlv->leaf_id) == "node-name") {
            node_name = tlv->value;
        }
        if (std::string(tlv->leaf_id) == "ip-address") {
            ip_address = tlv->value;
        }
    } TLV_LOOP_END;

    switch (cmd_code) {
    case CMDCODE_RUN_PING:
    {
        Node *node = topo->getNodeByNodeName(node_name);
        layer5PingFunc(node, ip_address);
        break;
    }
    }
    return 0;
}

int validate_node_name(char *value)
{
    if (!topo->getNodeByNodeName(value)) {
        std::cout << getColoredString("Error : non-existing node name.", "Red") << std::endl;
        return VALIDATION_FAILED;
    }
    return VALIDATION_SUCCESS;
}

int validate_ipv4_address(char *value)
{
    std::cmatch m;
    if (!std::regex_search(value, m, std::regex("^((25[0-5]|2[0-4][0-9]|1[0-9][0-9]|[1-9]?[0-9])\\.){3}(25[0-5]|2[0-4][0-9]|1[0-9][0-9]|[1-9]?[0-9])$"))) {
        std::cout << getColoredString("Error : wrong IPv4 format.", "Red") << std::endl;
        return VALIDATION_FAILED;
    }
    return VALIDATION_SUCCESS;
}

int validate_mask(char *value)
{
    std::cmatch m;
    if (!std::regex_search(value, m, std::regex("^([12]?[0-9]|3[012])$"))) {
        std::cout << getColoredString("Error : wrong mask value.", "Red") << std::endl;
        return VALIDATION_FAILED;
    }
    return VALIDATION_SUCCESS;
}

void nw_init_cli()
{
    init_libcli();

    param_t *show = libcli_get_show_hook();
    param_t *debug = libcli_get_debug_hook();
    param_t *config = libcli_get_config_hook();
    param_t *run = libcli_get_run_hook();
    param_t *debug_show = libcli_get_debug_show_hook();
    param_t *root = libcli_get_root();

    // currently unused
    (void)root;
    (void)debug_show;
    (void)debug;

    {
        /* show topology */
        static param_t topology;
        init_param(
            &topology,
            CMD,
            "topology",
            show_nw_topology_handler,
            0,
            INVALID,
            0,
            "Dump Complete Network Topology"
        );
        libcli_register_param(show, &topology);
        set_param_cmd_code(&topology, CMDCODE_SHOW_NW_TOPOLOGY);


        /* show node */
        static param_t node;
        init_param(
            &node,
            CMD,
            "node",
            0,
            0,
            INVALID,
            0,
            "Help : show node"
        );

        libcli_register_param(show, &node);
        {
            /* show node <node-name> */
            static param_t node_name;
            init_param(
                &node_name,
                LEAF,
                0,
                0,
                validate_node_name,
                STRING,
                "node-name",
                "Help : Node name"
            );
            libcli_register_param(&node, &node_name);

            {
                static param_t arp;
                init_param(
                    &arp,
                    CMD,
                    "arp",
                    show_arp_handler,
                    0,
                    INVALID,
                    0,
                    "Help : arp"
                );
                libcli_register_param(&node_name, &arp);
                set_param_cmd_code(&arp, CMDCODE_SHOW_ARP);
            }

            {
                static param_t mac;
                init_param(
                    &mac,
                    CMD,
                    "mac",
                    show_mac_handler,
                    0,
                    INVALID,
                    0,
                    "Help : mac"
                );
                libcli_register_param(&node_name, &mac);
                set_param_cmd_code(&mac, CMDCODE_SHOW_MAC);
            }

            {
                static param_t rt;
                init_param(
                    &rt,
                    CMD,
                    "rt",
                    show_rt_handler,
                    0,
                    INVALID,
                    0,
                    "Help : rt"
                );
                libcli_register_param(&node_name, &rt);
                set_param_cmd_code(&rt, CMDCODE_SHOW_ROUTING_TABLE);
            }
        }
    }

    {
        /* run node */
        static param_t node;
        init_param(
            &node,
            CMD,
            "node",
            0,
            0,
            INVALID,
            0,
            "Help : Node"
        );
        libcli_register_param(run, &node);
        {
            static param_t node_name;
            init_param(
                &node_name,
                LEAF,
                0,
                0,
                validate_node_name,
                STRING,
                "node-name",
                "Help : Node name"
            );
            libcli_register_param(&node, &node_name);

            {
                static param_t resolve_arp;
                init_param(
                    &resolve_arp,
                    CMD,
                    "resolve-arp",
                    0,
                    0,
                    INVALID,
                    0,
                    "Help : resolve-arp"
                );
                libcli_register_param(&node_name, &resolve_arp);
                {
                    static param_t ip_address;
                    init_param(
                        &ip_address,
                        LEAF,
                        0,
                        arp_handler,
                        validate_ipv4_address,
                        IPV4,
                        "ip-address",
                        "Help : ip-address"
                    );
                    libcli_register_param(&resolve_arp, &ip_address);
                    set_param_cmd_code(&ip_address, CMDCODE_RUN_RESOLVE_ARP);
                }
            }

            {
                static param_t ping;
                init_param(
                    &ping,
                    CMD,
                    "ping",
                    0,
                    0,
                    INVALID,
                    0,
                    "Help : ping"
                );
                libcli_register_param(&node_name, &ping);
                {
                    static param_t ip_address;
                    init_param(
                        &ip_address,
                        LEAF,
                        0,
                        ping_handler,
                        validate_ipv4_address,
                        IPV4,
                        "ip-address",
                        "Help : ip-address"
                    );
                    libcli_register_param(&ping, &ip_address);
                    set_param_cmd_code(&ip_address, CMDCODE_RUN_PING);
                }
            }
        }
    }

    {
        /* config node */
        static param_t node;
        init_param(
            &node,
            CMD,
            "node",
            0,
            0,
            INVALID,
            0,
            "Help : Node"
        );
        libcli_register_param(config, &node);
        {
            static param_t node_name;
            init_param(
                &node_name,
                LEAF,
                0,
                0,
                validate_node_name,
                STRING,
                "node-name",
                "Help : Node name"
            );
            libcli_register_param(&node, &node_name);

            {
                static param_t route;
                init_param(
                    &route,
                    CMD,
                    "route",
                    0,
                    0,
                    INVALID,
                    0,
                    "Help : route"
                );
                libcli_register_param(&node_name, &route);
                {
                    static param_t dest;
                    init_param(
                        &dest,
                        LEAF,
                        0,
                        0,
                        validate_ipv4_address,
                        IPV4,
                        "dest",
                        "Help : destination IP address"
                    );
                    libcli_register_param(&route, &dest);
                    {
                        static param_t mask;
                        init_param(
                            &mask,
                            LEAF,
                            0,
                            0,
                            validate_mask,
                            INT,
                            "mask",
                            "Help : mask bit length"
                        );
                        libcli_register_param(&dest, &mask);
                        {
                            static param_t gw_ip;
                            init_param(
                                &gw_ip,
                                LEAF,
                                0,
                                0,
                                validate_ipv4_address,
                                IPV4,
                                "gw-ip",
                                "Help : gateway IP address"
                            );
                            libcli_register_param(&mask, &gw_ip);
                            {
                                static param_t oif;
                                init_param(
                                    &oif,
                                    LEAF,
                                    0,
                                    l3_config_handler,
                                    0,
                                    STRING,
                                    "oif-name",
                                    "Help : outgoing interface name"
                                );
                                libcli_register_param(&gw_ip, &oif);
                                set_param_cmd_code(&oif, CMDCODE_CONFIG_REMOTE_ROUTE);
                            }
                        }
                    }
                }
            }
        }
    }


    support_cmd_negation(config);
}
