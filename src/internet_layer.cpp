#include "npan.h"
#include "utils.h"

namespace npan
{
    void IPv4_handler(unsigned char *data, int length)
    {
        unsigned int header_length = (data[0] & 15) << 3;
        unsigned int total_length = GET_TWO_BYTE(2);
        unsigned int flags_offset = GET_TWO_BYTE(6);
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

        fmt::print("Source      IP address: {}.{}.{}.{}\n", data[12], data[13], data[14], data[15]);
        fmt::print("Destination IP address: {}.{}.{}.{}\n", data[16], data[17], data[18], data[19]);

        if (__builtin_expect(prot == Protocal::UNKNOWN, 0))
        {
            fmt::print("{:─^56}\n", "");
            return;
        }

        transport_layer(&data[header_length], prot, total_length - header_length);
    }

    void IPv6_handler(unsigned char *data, int length)
    {
    }

    void ARP_handler(unsigned char *data, int length)
    {
    }

    void RARP_handler(unsigned char *data, int length)
    {
    }

    void internet_layer(unsigned char *data, Protocal protocal, int length)
    {
        fmt::print("{:─^56}\n", " Internet layer ");

        switch (protocal)
        {
        case Protocal::IPV4:
            IPv4_handler(data, length);
            break;

        case Protocal::IPV6:
            IPv6_handler(data, length);
            break;

        case Protocal::ARP:
            ARP_handler(data, length);
            break;

        case Protocal::RARP:
            RARP_handler(data, length);
            break;

        default:
            fmt::print("{:─^56}\n", "");
            break;
        }
    }

} // namespace npan
