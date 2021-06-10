#include "../npan-internal.h"

namespace npan
{
    void physical_layer(u_char *data)
    {
        detail::print("{:─^56}\n", " Physical layer ");
        detail::print("Destination MAC address: {:02x}:{:02x}:{:02x}:{:02x}:{:02x}:{:02x}\n", data[0], data[1], data[2], data[3], data[4], data[5]);
        detail::print("Source MAC address:      {:02x}:{:02x}:{:02x}:{:02x}:{:02x}:{:02x}\n", data[6], data[7], data[8], data[9], data[10], data[11]);

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

    void analyze_packet(u_char *data, int length)
    {
        physical_layer(data);
    }
} // namespace npan
