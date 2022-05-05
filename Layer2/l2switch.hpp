/**
 * @file l2switch.hpp
 * @author your name (you@domain.com)
 * @brief
 * @version 0.1
 * @date 2022-05-05
 */

#include <list>

#include "../graph.hpp"
#include "../net.hpp"
#include "../printer.hpp"

 /* L2 Switching functionallity */
void nodeSetInterfaceL2Mode(Node *node, const std::string &interface_name, const InterfaceNetworkProperty::L2Mode &mode);

struct MACTableEntry {
    MACTableEntry();

    MACAddress mac_addr;
    std::string oif_name;
};

class MACTable : public IPrinter {
public:
    MACTable() {}

    static MACTable *getNewTable()
    {
        return new MACTable();
    }

    bool addEntry(MACTableEntry *mac_table_entry);
    MACTableEntry *MACTableLookup(const MACAddress &mac_addr);
    void deleteEntry(const MACAddress &mac_addr);

    virtual void dump() const override;

private:
    std::list<MACTableEntry> mac_table;
};

MACTable *getNewMACTable();
void deleteMACTable(MACTable *mac_table);
