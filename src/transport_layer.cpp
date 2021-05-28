#include "npan.h"
#include "utils.h"

namespace npan
{
    void TCP_handler(unsigned char *data, int length)
    {
        unsigned int source_port = GET_TWO_BYTE(0);
        unsigned int dest_port = GET_TWO_BYTE(2);
        uint32_t seq = GET_FOUR_BYTE(4);
        uint32_t ack = GET_FOUR_BYTE(8);
        unsigned int flag = GET_TWO_BYTE(12);
        
        int header_length = (flag & 0xf000) >> 12;
        flag &= 0x0fff;

        unsigned int window = GET_TWO_BYTE(14);

        // application_layer(&data[18], Protocal::UNKNOWN, length);
    }

    void UDP_handler(unsigned char *data, int length)
    {
    }

    void transport_layer(unsigned char *data, Protocal protocal, int length)
    {
        fmt::print("{:─^56}\n", " Transport layer ");

        switch (protocal)
        {
        case Protocal::TCP:
            TCP_handler(data, length);
            break;

        case Protocal::UDP:
            UDP_handler(data, length);
            break;

        default:
            fmt::print("{:─^56}\n", "");
            break;
        }
    }
} // namespace npan