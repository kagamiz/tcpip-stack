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
        }
    }


    support_cmd_negation(config);
}
