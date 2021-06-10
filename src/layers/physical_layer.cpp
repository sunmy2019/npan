#include "../npan-internal.h"

namespace npan
{
    void physical_layer(u_char *data)
    {
        detail::print("{:─^56}\n", " Physical layer ");
        MAC_addr dest_mac{GET_SIX_BYTE(0)};
        MAC_addr source_mac{GET_SIX_BYTE(6)};
        detail::print("Destination MAC address: {}\n", dest_mac);
        detail::print("Source MAC address:      {}\n", source_mac);

        Protocal prot;

        switch (GET_TWO_BYTE(12))
        {
        case 0x0800:
            prot = Protocal::IPV4;
            detail::print("Protocal type: IPv4\n");
            break;
        case 0x86dd:
            prot = Protocal::IPV6;
            detail::print("Protocal type: IPv6\n");
            break;
        case 0x0806:
            prot = Protocal::ARP;
            detail::print("Protocal type: ARP\n");
            break;

        default:
            detail::print("Unsupported protocal type {:04x}\n", GET_TWO_BYTE(12));
            detail::print("{:─^56}\n", "");
            return;
        }

        internet_layer(&data[14], prot);
    }

    void analyze_packet(u_char *data, u_int length)
    {
        physical_layer(data);
    }
} // namespace npan
