#include "../utils.h"

namespace npan
{
    void IPv4_handler(u_char *data)
    {
        u_int header_length = (data[0] & 15) << 2;
        u_int total_length = GET_TWO_BYTE(2);
        u_int flags_offset = GET_TWO_BYTE(6);
        Protocal prot = Protocal::UNKNOWN;

        fmt::print("IP Version {}\n", data[0] >> 4);

        fmt::print("Header Length {} bytes\n", header_length);
        // omit differentiated field: data[1]

        fmt::print("Total Length {} bytes\n", total_length);
        fmt::print("Identification {}\n", GET_TWO_BYTE(4));

        // todo: handle flags and offsets

        fmt::print("Time to live: {}\n", data[8]);

        switch (data[9])
        {
        case 0x06:
            prot = Protocal::TCP;
            fmt::print("Protocal: TCP\n");
            break;
        case 0x11:
            prot = Protocal::UDP;
            fmt::print("Protocal: UDP\n");
            break;
        default:
            fmt::print("Unsupported Protocal!\n");
            break;
        }

        fmt::print("Source IP address:      {}.{}.{}.{}\n", data[12], data[13], data[14], data[15]);
        fmt::print("Destination IP address: {}.{}.{}.{}\n", data[16], data[17], data[18], data[19]);

        if (prot == Protocal::UNKNOWN) [[unlikely]]
        {
            fmt::print("{:─^56}\n", "");
            return;
        }

        transport_layer(&data[header_length], prot, GET_FOUR_BYTE(12), GET_FOUR_BYTE(16), total_length - header_length);
    }

    void IPv6_handler(u_char *data)
    {
    }

    void ARP_handler(u_char *data)
    {
    }

    void RARP_handler(u_char *data)
    {
    }

    void internet_layer(u_char *data, Protocal protocal)
    {
        fmt::print("{:─^56}\n", " Internet layer ");

        switch (protocal)
        {
        case Protocal::IPV4:
            IPv4_handler(data);
            break;

        case Protocal::IPV6:
            IPv6_handler(data);
            break;

        case Protocal::ARP:
            ARP_handler(data);
            break;

        case Protocal::RARP:
            RARP_handler(data);
            break;

        default:
            fmt::print("{:─^56}\n", "");
            break;
        }
    }

} // namespace npan
