#include "../utils.h"

#include <map>
namespace npan
{
    struct TCP_connection
    {
        uint64_t source_ip;
        unsigned int source_port;
        uint64_t dest_ip;
        unsigned int dest_port;

        TCP_connection get_conjugate()
        {
            return {dest_ip, dest_port, source_ip, source_port};
        }
    };

    struct TCP_connection_status
    {
        TCP_connection *connection;

        unsigned int init_seq;
        unsigned int init_ack;
        std::vector<unsigned char> buffer;

        void flush_buffer()
        {
            application_layer(std::make_unique<std::vector<unsigned char>>(std::move(buffer)), Protocal::UNKNOWN, 0);
        }
    };

    static std::map<TCP_connection, TCP_connection_status> tcp_map;

    void TCP_handler(unsigned char *data, uint64_t source_ip, uint64_t dest_ip, int length)
    {
        unsigned int source_port = GET_TWO_BYTE(0);
        unsigned int dest_port = GET_TWO_BYTE(2);
        uint32_t seq = GET_FOUR_BYTE(4);
        uint32_t ack = GET_FOUR_BYTE(8);
        unsigned int flag = GET_TWO_BYTE(12);

        unsigned int header_length = (flag & 0xf000) >> 9; // in bytes
        flag &= 0x0fff;

        uint32_t payload_length = length - header_length;

        unsigned int window = GET_TWO_BYTE(14);

        // look up connection first

        // create a connection if connection not exist. Use the TCP connection rules. (Handshakes)
        // close the connection according to the rules.

        // if the payload_length == 1440, write into the correct position of the buffer
        // otherwise flush the buffer.
    }

    void UDP_handler(unsigned char *data, uint64_t source_ip, uint64_t dest_ip, int length)
    {
    }

    void transport_layer(unsigned char *data, Protocal protocal, uint64_t source_ip, uint64_t dest_ip, int length)
    {
        fmt::print("{:─^56}\n", " Transport layer ");

        switch (protocal)
        {
        case Protocal::TCP:
            TCP_handler(data, length, source_ip, dest_ip);
            break;

        case Protocal::UDP:
            UDP_handler(data, length, source_ip, dest_ip);
            break;

        default:
            fmt::print("{:─^56}\n", "");
            break;
        }
    }
} // namespace npan