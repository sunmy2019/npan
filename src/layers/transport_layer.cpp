#include "../utils.h"
#include <cassert>
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
        friend std::strong_ordering operator<=>(const TCP_connection &, const TCP_connection &) = default;
    };

    struct TCP_connection_status
    {
        unsigned int init_seq;
        unsigned int init_ack;
        bool started = 0;
        bool finished = 0;
        uint32_t buffer_start_seq = 0; // absolute
        std::vector<unsigned char> buffer;

        void flush_buffer()
        {
            buffer_start_seq = 0;
            application_layer(std::make_unique<std::vector<unsigned char>>(std::move(buffer)), Protocal::UNKNOWN, 0);
        }
    };

    static std::map<TCP_connection, TCP_connection_status> tcp_map;

    void TCP_handler(unsigned char *data, uint64_t source_ip, uint64_t dest_ip, unsigned int length)
    {
        // output_packet_to_console(data, length);

        unsigned int source_port = GET_TWO_BYTE(0);
        unsigned int dest_port = GET_TWO_BYTE(2);
        uint32_t seq = GET_FOUR_BYTE(4);
        uint32_t ack = GET_FOUR_BYTE(8);
        uint32_t init_seq = 0;
        uint32_t init_ack = 0;

        std::vector<unsigned char> *buffer = nullptr;
        uint32_t *buffer_start_seq = nullptr;

        unsigned int flag = GET_TWO_BYTE(12);

        unsigned int header_length = (flag & 0xf000) >> 10; // in bytes

        flag &= 0x0fff;

        uint32_t payload_length = length - header_length;

        unsigned int window = GET_TWO_BYTE(14);

        TCP_connection tcps{source_ip, source_port, dest_ip, dest_port};
        TCP_connection tcpr = tcps.get_conjugate();

        switch (flag)
        {
        case 0x02: // SYNC
            fmt::print("Flag: SYNC\n");
            // first handshake
            tcp_map[tcps] = TCP_connection_status{seq, 0};
            tcp_map[tcpr] = TCP_connection_status{0, seq};
            init_ack = ack;
            init_seq = seq;
            break;

        case 0x12: // SYNC, ACK
            fmt::print("Flag: SYNC, ACK\n");

            // second handshake
            tcp_map[tcps].started = 1; // start self
            tcp_map[tcps].init_seq = seq;
            tcp_map[tcpr].init_ack = seq;
            // fmt::print("{},{}", tcp_map[tcps].init_seq, tcp_map[tcps].init_ack);
            // fmt::print("{},{}", tcp_map[tcpr].init_seq, tcp_map[tcpr].init_ack);
            init_seq = seq - 1;
            init_ack = ack;
            break;

        case 0x10: // ACK
            fmt::print("Flag: ACK\n");

            init_ack = tcp_map[tcps].init_ack;
            init_seq = tcp_map[tcps].init_seq;

            if (!tcp_map[tcps].started) [[unlikely]]
            { // third handshake
                assert(ack == init_ack + 1);
                assert(seq == init_seq + 1);
                tcp_map[tcps].started = 1;
                break;
            }

            buffer_start_seq = &tcp_map[tcps].buffer_start_seq;
            buffer = &tcp_map[tcps].buffer;

            if (*buffer_start_seq == 0) // first part
                *buffer_start_seq = seq;

            if (*buffer_start_seq + buffer->size() < seq + payload_length) [[likely]]
                buffer->resize(seq - *buffer_start_seq + payload_length); // needs to enlarge
            // output_packet_to_console(&data[header_length], payload_length);
            memcpy(&((*buffer)[seq - *buffer_start_seq]), &data[header_length], payload_length);

            break;
        case 0x18: // ACK, PUSH

            fmt::print("Flag: ACK, PUSH\n");

            init_ack = tcp_map[tcps].init_ack;
            init_seq = tcp_map[tcps].init_seq;

            buffer_start_seq = &tcp_map[tcps].buffer_start_seq;
            buffer = &tcp_map[tcps].buffer;

            if (*buffer_start_seq == 0) // first part
                *buffer_start_seq = seq;

            if (*buffer_start_seq + buffer->size() < seq + payload_length) [[likely]]
                buffer->resize(seq - *buffer_start_seq + payload_length); // needs to enlarge
            // output_packet_to_console(&data[header_length], payload_length);
            memcpy(&((*buffer)[seq - *buffer_start_seq]), &data[header_length], payload_length);
            break;

        case 0x11: // FIN, ACK
            fmt::print("Flag: FIN, ACK\n");

            // here we assumes when FIN is recieved, both tcp_connection exists. may change later to be more rubost
            if (tcp_map[tcpr].finished)
            { // if both finished, remove both tcps and tcpr
                tcp_map.erase(tcps);
                tcp_map.erase(tcpr);
            }
            else
            { // otherwise set a finish mark
                tcp_map[tcps].finished = 1;
            }
            break;

        default:

            break;
        }

        fmt::print("Relative seq {}, Absolute seq {}\n", seq - init_seq, seq);
        fmt::print("Relative ack {}, Absolute ack {}\n", ack - init_ack, ack);

        fmt::print("Payload length {} bytes\n", payload_length);

        if (flag == 0x18) // PUSH
            tcp_map[tcps].flush_buffer();
        else
            fmt::print("{:─^56}\n", "");
    }

    void UDP_handler(unsigned char *data, uint64_t source_ip, uint64_t dest_ip, int length)
    {
    }

    void transport_layer(unsigned char *data, Protocal protocal, uint64_t source_ip, uint64_t dest_ip, unsigned int length)
    {
        fmt::print("{:─^56}\n", " Transport layer ");
        switch (protocal)
        {
        case Protocal::TCP:
            TCP_handler(data, source_ip, dest_ip, length);
            break;

        case Protocal::UDP:
            UDP_handler(data, source_ip, dest_ip, length);
            break;

        default:
            fmt::print("{:─^56}\n", "");
            break;
        }
    }
} // namespace npan