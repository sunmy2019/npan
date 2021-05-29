#include "../utils.h"

namespace npan
{
    void physical_layer(u_char *data)
    {
        fmt::print("{:─^56}\n", " Physical layer ");
        fmt::print("Destination MAC address: {:02x}:{:02x}:{:02x}:{:02x}:{:02x}:{:02x}\n", data[0], data[1], data[2], data[3], data[4], data[5]);
        fmt::print("Source MAC address:      {:02x}:{:02x}:{:02x}:{:02x}:{:02x}:{:02x}\n", data[6], data[7], data[8], data[9], data[10], data[11]);

        Protocal prot;

        switch (GET_TWO_BYTE(12))
        {
        case 0x0800:
            prot = Protocal::IPV4;
            fmt::print("Protocal type: IPv4\n");
            break;
        case 0x86DD:
            prot = Protocal::IPV6;
            fmt::print("Protocal type: IPv6\n");
            break;
        case 0x0806:
            prot = Protocal::ARP;
            fmt::print("Protocal type: ARP\n");
            break;
        case 0x0835:
            prot = Protocal::RARP;
            fmt::print("Protocal type: RARP\n");
            break;

        default:
            fmt::print("Unsupported protocal type {:04x}\n", GET_TWO_BYTE(12));
            fmt::print("{:─^56}\n", "");
            return;
        }

        // fmt::print("{:─^56}\n", "");

        internet_layer(&data[14], prot);
    }

} // namespace npan
