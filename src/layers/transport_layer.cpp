#include "../npan-internal.h"
#include <cassert>
#include <map>
namespace npan
{
    static uint global_tcp_stream_no = 0;

    struct TCP_connection_status
    {
        u_int tcp_stream_no;
        u_int init_seq;
        u_int init_ack;
        bool started = 0;
        bool finished = 0;
        uint32_t buffer_start_seq = 0; // absolute
        std::vector<u_char> buffer;

        void flush_buffer()
        {
            buffer_start_seq += buffer.size();
            // move the whole buffer into application layer
            application_layer(std::move(buffer), tcp_stream_no);
        }
    };

    template <IP_ver Ver>
    static std::map<Connection<Protocal::TCP, Ver>, TCP_connection_status> tcp_map; // records the connection

    template <IP_ver Ver>
    void TCP_handler(u_char *data, IP_addr<Ver> source_ip, IP_addr<Ver> dest_ip, u_int length)
    {
        using TCP_connection = Connection<Protocal::TCP, Ver>;
        // output_packet_to_console(data, length);
        u_int source_port = GET_TWO_BYTE(0);
        u_int dest_port = GET_TWO_BYTE(2);
        uint32_t seq = GET_FOUR_BYTE(4);
        uint32_t ack = GET_FOUR_BYTE(8);
        uint32_t init_seq = 0;
        uint32_t init_ack = 0;

        u_int flag = GET_TWO_BYTE(12);
        u_int header_length = (flag & 0xf000) >> 10; // in bytes

        std::vector<u_char> *buffer = nullptr;
        uint32_t *buffer_start_seq = nullptr;

        flag &= 0x0fff;

        uint32_t payload_length = length - header_length;

        u_int window = GET_TWO_BYTE(14);

        TCP_connection tcps{source_ip, source_port, dest_ip, dest_port};
        TCP_connection tcpr = tcps.get_conjugate();

        switch (flag)
        {
        case 0x02: // SYNC
            fmt::print("Flag: SYNC\n");
            // first handshake
            tcp_map<Ver>[tcps] = TCP_connection_status{++global_tcp_stream_no, seq, 0};
            tcp_map<Ver>[tcpr] = TCP_connection_status{++global_tcp_stream_no, 0, seq};
            init_ack = ack;
            init_seq = seq;
            break;

        case 0x12: // SYNC, ACK
            fmt::print("Flag: SYNC, ACK\n");

            // second handshake
            tcp_map<Ver>[tcps].started = 1; // start self
            tcp_map<Ver>[tcps].init_seq = seq;
            tcp_map<Ver>[tcpr].init_ack = seq;
            // fmt::print("{},{}", tcp_map<Ver>[tcps].init_seq, tcp_map<Ver>[tcps].init_ack);
            // fmt::print("{},{}", tcp_map<Ver>[tcpr].init_seq, tcp_map<Ver>[tcpr].init_ack);
            init_seq = seq - 1;
            init_ack = ack;
            break;

        case 0x10: // ACK
            fmt::print("Flag: ACK\n");

            init_ack = tcp_map<Ver>[tcps].init_ack;
            init_seq = tcp_map<Ver>[tcps].init_seq;

            if (tcp_map<Ver>[tcpr].finished && tcp_map<Ver>[tcps].finished) [[unlikely]]
            { // if both finished, remove both tcps and tcpr
                tcp_map<Ver>.erase(tcps);
                tcp_map<Ver>.erase(tcpr);
                fmt::print("Connection closed\n");
                break;
            }

            if (!tcp_map<Ver>[tcps].started) [[unlikely]]
            { // third handshake
                assert(ack == init_ack + 1);
                assert(seq == init_seq + 1);
                tcp_map<Ver>[tcps].started = 1;
                break;
            }

            if (payload_length != 0) [[likely]]
            {
                buffer_start_seq = &tcp_map<Ver>[tcps].buffer_start_seq;
                buffer = &tcp_map<Ver>[tcps].buffer;

                if (*buffer_start_seq == 0) [[unlikely]] // first arrival of this tcp connection
                    *buffer_start_seq = init_seq + 1;

                if (*buffer_start_seq > seq) [[unlikely]] // package arrived after its buffer being PUSHed
                {
                    fmt::print("Package arrived after its buffer being pushed\n");
                    fmt::print("Source port:      {}\n", source_port);
                    fmt::print("Destination port: {}\n", dest_port);
                    fmt::print("{:─^56}\n", "");
                    return;
                }

                if (*buffer_start_seq + buffer->size() < seq + payload_length) [[likely]]
                    buffer->resize(seq - *buffer_start_seq + payload_length); // needs to enlarge
                // output_packet_to_console(&data[header_length], payload_length);
                memcpy(&((*buffer)[seq - *buffer_start_seq]), &data[header_length], payload_length);
            }
            break;

        case 0x18: // ACK, PUSH

            fmt::print("Flag: ACK, PUSH\n");

            init_ack = tcp_map<Ver>[tcps].init_ack;
            init_seq = tcp_map<Ver>[tcps].init_seq;

            buffer_start_seq = &tcp_map<Ver>[tcps].buffer_start_seq;
            buffer = &tcp_map<Ver>[tcps].buffer;

            if (*buffer_start_seq == 0) [[unlikely]] // first arrival of this tcp connection
                *buffer_start_seq = init_seq + 1;

            if (*buffer_start_seq > seq) [[unlikely]] // package arrived after its buffer being PUSHed
            {
                fmt::print("Package arrived after its buffer being pushed\n");
                fmt::print("Source port:      {}\n", source_port);
                fmt::print("Destination port: {}\n", dest_port);
                fmt::print("{:─^56}\n", "");
                return;
            }

            if (*buffer_start_seq + buffer->size() < seq + payload_length) [[likely]]
                buffer->resize(seq - *buffer_start_seq + payload_length); // needs to enlarge
            // output_packet_to_console(&data[header_length], payload_length);
            memcpy(&((*buffer)[seq - *buffer_start_seq]), &data[header_length], payload_length);

            break;

        case 0x11: // FIN, ACK
            fmt::print("Flag: FIN, ACK\n");

            // set a finish mark
            tcp_map<Ver>[tcps].finished = 1;

            init_ack = tcp_map<Ver>[tcps].init_ack;
            init_seq = tcp_map<Ver>[tcps].init_seq;

            if (payload_length != 0) [[unlikely]]
            {
                buffer_start_seq = &tcp_map<Ver>[tcps].buffer_start_seq;
                buffer = &tcp_map<Ver>[tcps].buffer;

                if (*buffer_start_seq == 0) [[unlikely]] // first arrival of this tcp connection
                    *buffer_start_seq = init_seq + 1;

                if (*buffer_start_seq > seq) [[unlikely]] // package arrived after its buffer being PUSHed
                {
                    fmt::print("Package arrived after its buffer being pushed\n");
                    fmt::print("Source port:      {}\n", source_port);
                    fmt::print("Destination port: {}\n", dest_port);
                    fmt::print("{:─^56}\n", "");
                    return;
                }

                if (*buffer_start_seq + buffer->size() < seq + payload_length) [[likely]]
                    buffer->resize(seq - *buffer_start_seq + payload_length); // needs to enlarge
                // output_packet_to_console(&data[header_length], payload_length);
                memcpy(&((*buffer)[seq - *buffer_start_seq]), &data[header_length], payload_length);
            }
            break;

        case 0x04: // RST
            fmt::print("Flag: RST\n");
            fmt::print("Source port:      {}\n", source_port);
            fmt::print("Destination port: {}\n", dest_port);
            fmt::print("{:─^56}\n", "");
            return;

        default:
            break;
        }

        fmt::print("Relative seq {}, Absolute seq {}\n", seq - init_seq, seq);
        fmt::print("Relative ack {}, Absolute ack {}\n", ack - init_ack, ack);
        fmt::print("Source port:      {}\n", source_port);
        fmt::print("Destination port: {}\n", dest_port);
        fmt::print("Header length  {} bytes\n", header_length);
        fmt::print("Payload length {} bytes\n", payload_length);
        fmt::print("Window size    {} bytes\n", window);

        if (flag == 0x18) // PUSH
            tcp_map<Ver>[tcps].flush_buffer();
        else
            fmt::print("{:─^56}\n", "");
    }

    template <IP_ver V>
    void UDP_handler(u_char *data, IP_addr<V> source_ip, IP_addr<V> dest_ip, u_int length)
    {
        u_int source_port = GET_TWO_BYTE(0);
        u_int dest_port = GET_TWO_BYTE(2);
        assert(length == GET_TWO_BYTE(4));
        fmt::print("Source port:      {}\n", source_port);
        fmt::print("Destination port: {}\n", dest_port);
        fmt::print("Total length  {} bytes\n", length);

        application_layer(&data[8], length - 8, Connection<Protocal::UDP, V>{source_ip, source_port, dest_ip, dest_port});
    }

    void ICMPv6_handler(u_char *data, IPv6_addr source_ip, IPv6_addr dest_ip, int length)
    {
        fmt::print("{:─^56}\n", "");
    }

    template <IP_ver V>
    void transport_layer(u_char *data, Protocal protocal, IP_addr<V> source_ip, IP_addr<V> dest_ip, u_int length)
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

        case Protocal::ICMPv6:
            if constexpr (std::is_same_v<IP_addr<V>, IPv6_addr>)
                ICMPv6_handler(data, source_ip, dest_ip, length);
            else
                fmt::print("{:─^56}\n", "");
            break;

        default:
            fmt::print("{:─^56}\n", "");
            break;
        }
    }

    template void transport_layer(u_char *, Protocal, IPv4_addr, IPv4_addr, u_int);
    template void transport_layer(u_char *, Protocal, IPv6_addr, IPv6_addr, u_int);

} // namespace npan