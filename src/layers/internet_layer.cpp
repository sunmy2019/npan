#include "../npan-internal.h"

namespace npan
{
    void IPv4_handler(u_char *data)
    {
        u_int header_length = (data[0] & 15) << 2;
        u_int total_length = GET_TWO_BYTE(2);
        // u_int flags_offset = GET_TWO_BYTE(6);
        Protocal prot = Protocal::UNKNOWN;

        fmt::print("IP version {}\n", data[0] >> 4);

        fmt::print("Header length {} bytes\n", header_length);
        // omit differentiated field: data[1]

        fmt::print("Total length {} bytes\n", total_length);
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
            fmt::print("Unsupported protocal.\n");
            break;
        }

        fmt::print("Source IP address:      {}.{}.{}.{}\n", data[12], data[13], data[14], data[15]);
        fmt::print("Destination IP address: {}.{}.{}.{}\n", data[16], data[17], data[18], data[19]);

        if (prot == Protocal::UNKNOWN) [[unlikely]]
        {
            fmt::print("{:─^56}\n", "");
            return;
        }

        transport_layer(&data[header_length], prot, IPv4_addr(GET_FOUR_BYTE(12)), IPv4_addr(GET_FOUR_BYTE(16)),
                        total_length - header_length);
    }
    void IPv6_handler(u_char *data)
    {
        u_int payload_length = GET_TWO_BYTE(4);
        u_int next_header = data[6];
        u_int hop_limit = data[7];
        u_int header_offset = 40;

        Protocal prot = Protocal::UNKNOWN;

        IPv6_addr source_ip{LEFT_SHIFT((u_int64_t)GET_FOUR_BYTE(8), 4) + GET_FOUR_BYTE(12),
                            LEFT_SHIFT((u_int64_t)GET_FOUR_BYTE(16), 4) + GET_FOUR_BYTE(20)};
        IPv6_addr dest_ip{LEFT_SHIFT((u_int64_t)GET_FOUR_BYTE(24), 4) + GET_FOUR_BYTE(28),
                          LEFT_SHIFT((u_int64_t)GET_FOUR_BYTE(32), 4) + GET_FOUR_BYTE(36)};

        fmt::print("IP version {}\n", data[0] >> 4);
        fmt::print("Traffic class {:x}\n", (GET_TWO_BYTE(0) & 0xff0) >> 4);
        fmt::print("Flow label {:x}\n", (u_int32_t)((data[1] & 15) << 16) + GET_TWO_BYTE(2));
        fmt::print("Payload length {} bytes\n", payload_length);

        fmt::print("Hop limit {}\n", hop_limit);

        fmt::print("Source IP address: {}\n", source_ip);
        fmt::print("Destination IP address: {}\n", dest_ip);
        bool ext = false;
        do
        {
            switch (next_header)
            {
            case 0: // IPv6 Hop-by-Hop Options Header
                fmt::print("Next header: IPv6 Hop-by-Hop options header\n");
                next_header = data[header_offset];
                header_offset += (data[header_offset + 1] + 1) << 3;
                break;

            case 6: // TCP
                prot = Protocal::TCP;
                fmt::print("Protocal: TCP\n");
                ext = true;
                break;

            case 17: // UDP
                prot = Protocal::UDP;
                fmt::print("Protocal: UDP\n");
                ext = true;
                break;

            case 58: // ICMPv6
                fmt::print("Protocal: ICMPv6\n");
                ext = true;
                break;

            case 59:
                fmt::print("No next header.\n");
                ext = true;
                break;

            case 43: // Routing header
                fmt::print("Next header: Routing header\n");
                next_header = data[header_offset];
                header_offset += (data[header_offset + 1] + 1) << 3;
                break;

            case 44: // Fragment header
                fmt::print("Next header: Fragment header\n");
                next_header = data[header_offset];
                header_offset += (data[header_offset + 1] + 1) << 3;
                break;

            case 51: // Authentication header
                fmt::print("Next header: Authentication header\n");
                next_header = data[header_offset];
                header_offset += (data[header_offset + 1] + 1) << 3;
                break;

            case 60: // Destination Options header
                fmt::print("Next header: Destination options header\n");
                next_header = data[header_offset];
                header_offset += (data[header_offset + 1] + 1) << 3;
                break;

            case 41: // Encapsulated IPv6 header
                [[fallthrough]];

            case 50: // Encapsulating Security Payload header
                [[fallthrough]];

            default:
                fmt::print("Unsupported protocal.\n");
                ext = true;
                break;
            }
        } while (!ext);


        if (prot == Protocal::UNKNOWN) [[unlikely]]
        {
            fmt::print("{:─^56}\n", "");
            return;
        }

        transport_layer(&data[header_offset], prot, source_ip, dest_ip, payload_length);
    }

    void ARP_handler(u_char *data)
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

        default:
            fmt::print("{:─^56}\n", "");
            break;
        }
    }

} // namespace npan