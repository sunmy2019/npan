#include "../npan-internal.h"
#include <unordered_map>

namespace npan
{
    static u_int global_tcp_stream_no = 0;

    struct TCP_connection_status
    {
        u_int32_t tcp_stream_no;
        u_int32_t init_seq;
        u_int32_t init_ack;
        bool started = 0;
        bool finished = 0;
        u_int32_t buffer_start_seq = 0; // absolute
        bool broken_connection = false;
        std::vector<u_char> buffer;

        void flush_buffer()
        {
            buffer_start_seq += buffer.size();
            if (buffer.size())
                // move the whole buffer into application layer
                application_layer(std::move(buffer), tcp_stream_no);
        }
    };

    template <IP_ver Ver>
    static std::unordered_map<Connection<Protocal::TCP, Ver>, TCP_connection_status> tcp_map; // records the connection

    template <IP_ver Ver>
    void TCP_handler(const u_char *data, IP_addr<Ver> source_ip, IP_addr<Ver> dest_ip, u_int length)
    {
        using TCP_connection = Connection<Protocal::TCP, Ver>;
        u_int source_port = GET_TWO_BYTE(0);
        u_int dest_port = GET_TWO_BYTE(2);
        u_int32_t seq = GET_FOUR_BYTE(4);
        u_int32_t ack = GET_FOUR_BYTE(8);
        u_int32_t init_seq = 0;
        u_int32_t init_ack = 0;

        u_int flag = GET_TWO_BYTE(12);
        u_int header_length = (flag & 0xf000) >> 10; // in bytes

        std::vector<u_char> *buffer = nullptr;
        u_int32_t *buffer_start_seq = nullptr;
        bool broken_connection = false;

        flag &= 0x0fff;

        u_int32_t payload_length = length - header_length;

        u_int window = GET_TWO_BYTE(14);

        TCP_connection tcps{source_ip, source_port, dest_ip, dest_port};
        TCP_connection tcpr = tcps.get_conjugate();

        switch (flag)
        {
        case 0x02: // SYNC
            detail::print("Flag: SYNC\n");
            // first handshake
            tcp_map<Ver>[tcps] = TCP_connection_status{++global_tcp_stream_no, seq, 0};
            tcp_map<Ver>[tcpr] = TCP_connection_status{++global_tcp_stream_no, 0, seq};
            init_ack = ack;
            init_seq = seq;
            break;

        case 0x12: // SYNC, ACK
            detail::print("Flag: SYNC, ACK\n");
            // second handshake

            // if the first handshake is lost or the last connection is not erased, we will make a new one
            if ((tcp_map<Ver>.find(tcps) == tcp_map<Ver>.end() || tcp_map<Ver>.find(tcpr) == tcp_map<Ver>.end()) || (tcp_map<Ver>[tcps].finished || tcp_map<Ver>[tcpr].finished)) [[unlikely]]
            {
                tcp_map<Ver>[tcpr].started = 1; // start other
                tcp_map<Ver>[tcpr].tcp_stream_no = ++global_tcp_stream_no;
                tcp_map<Ver>[tcps].tcp_stream_no = ++global_tcp_stream_no;
                tcp_map<Ver>[tcpr].init_seq = ack - 1;
                tcp_map<Ver>[tcps].init_ack = ack - 1;
            }

            tcp_map<Ver>[tcps].started = 1; // start self
            // detail::warning("{} {}\n", tcp_map<Ver>[tcpr].started, tcp_map<Ver>[tcps].started);

            tcp_map<Ver>[tcps].init_seq = seq;
            tcp_map<Ver>[tcpr].init_ack = seq;

            init_seq = seq - 1;
            init_ack = ack;
            break;

        case 0x10: // ACK
            detail::print("Flag: ACK\n");

            if (tcp_map<Ver>.find(tcps) == tcp_map<Ver>.end() || tcp_map<Ver>.find(tcpr) == tcp_map<Ver>.end()) [[unlikely]]
            { // handshakes are all lost! Fill in whatever is usable
                tcp_map<Ver>[tcps] = TCP_connection_status{++global_tcp_stream_no, seq, ack, 1 /*started*/, 0, 0, /*broken*/ true};
                tcp_map<Ver>[tcpr] = TCP_connection_status{++global_tcp_stream_no, ack, seq, 1 /*started*/, 0, 0, /*broken*/ true};
                broken_connection = true;
            }

            if (tcp_map<Ver>[tcpr].finished && tcp_map<Ver>[tcps].finished) [[unlikely]]
            { // if both finished, remove both tcps and tcpr
                tcp_map<Ver>.erase(tcps);
                tcp_map<Ver>.erase(tcpr);
                detail::print("Connection closed\n");
                break;
            }

            init_ack = tcp_map<Ver>[tcps].init_ack;
            init_seq = tcp_map<Ver>[tcps].init_seq;
            // detail::warning("{} {}\n", tcp_map<Ver>[tcpr].started, tcp_map<Ver>[tcps].started);
            if (!tcp_map<Ver>[tcps].started) [[unlikely]]
            {
                if (ack == init_ack + 1 && seq == init_seq + 1) [[likely]]
                { // third handshake, second handshake is not lost
                    tcp_map<Ver>[tcps].started = 1;
                    break;
                }
                else
                {
                    detail::warning("Second or Third handshake was lost.\n");
                    tcp_map<Ver>[tcps] = TCP_connection_status{tcp_map<Ver>[tcps].tcp_stream_no, seq, ack, 1 /*started*/, 0, 0, /*broken*/ true};
                    tcp_map<Ver>[tcpr] = TCP_connection_status{tcp_map<Ver>[tcpr].tcp_stream_no, ack, seq, 1 /*started*/, 0, 0, /*broken*/ true};
                    init_ack = ack;
                    init_seq = seq;
                }
            }

            if (payload_length != 0) [[likely]]
            {
                buffer_start_seq = &tcp_map<Ver>[tcps].buffer_start_seq;
                buffer = &tcp_map<Ver>[tcps].buffer;

                broken_connection = tcp_map<Ver>[tcps].broken_connection;

                if (*buffer_start_seq == 0) [[unlikely]]               // first arrival of this tcp connection
                    *buffer_start_seq = init_seq + !broken_connection; // bool->int

                if (*buffer_start_seq > seq) [[unlikely]] // package arrived after its buffer being PUSHed
                {
                    detail::warning("Package arrived its buffer being pushed\nBuffer Seq: {}, Packet Seq {}\n", *buffer_start_seq, seq);
                    detail::print("Source port:      {}\n", source_port);
                    detail::print("Destination port: {}\n", dest_port);
                    detail::print("{:─^56}\n", "");
                    return;
                }

                if (*buffer_start_seq + buffer->size() < seq + payload_length) [[likely]]
                    buffer->resize(seq - *buffer_start_seq + payload_length); // needs to enlarge
                std::memcpy(&((*buffer)[seq - *buffer_start_seq]), &data[header_length], payload_length);
            }
            break;

        case 0x18: // ACK, PUSH

            detail::print("Flag: ACK, PUSH\n");

            if (tcp_map<Ver>.find(tcps) == tcp_map<Ver>.end() || tcp_map<Ver>.find(tcpr) == tcp_map<Ver>.end()) [[unlikely]]
            { // handshakes are all lost! Fill in whatever is usable
                tcp_map<Ver>[tcps] = TCP_connection_status{++global_tcp_stream_no, seq, ack, 1 /*started*/, 0, 0, /*broken*/ true};
                tcp_map<Ver>[tcpr] = TCP_connection_status{++global_tcp_stream_no, ack, seq, 1 /*started*/, 0, 0, /*broken*/ true};
            }

            init_ack = tcp_map<Ver>[tcps].init_ack;
            init_seq = tcp_map<Ver>[tcps].init_seq;

            if (!tcp_map<Ver>[tcps].started) [[unlikely]]
            {
                if (ack == init_ack + 1 && seq == init_seq + 1) [[likely]]
                { // third handshake, second handshake is not lost
                    tcp_map<Ver>[tcps].started = 1;
                    // detail::warning("Start Two");
                    break;
                }
                else
                {
                    detail::warning("Second or Third handshake was lost.\n");
                    tcp_map<Ver>[tcps] = TCP_connection_status{tcp_map<Ver>[tcps].tcp_stream_no, seq, ack, 1 /*started*/, 0, 0, /*broken*/ true};
                    tcp_map<Ver>[tcpr] = TCP_connection_status{tcp_map<Ver>[tcpr].tcp_stream_no, ack, seq, 1 /*started*/, 0, 0, /*broken*/ true};
                    init_ack = ack;
                    init_seq = seq;
                }
            }

            buffer_start_seq = &tcp_map<Ver>[tcps].buffer_start_seq;
            buffer = &tcp_map<Ver>[tcps].buffer;

            broken_connection = tcp_map<Ver>[tcps].broken_connection;

            if (*buffer_start_seq == 0) [[unlikely]]               // first arrival of this tcp connection
                *buffer_start_seq = init_seq + !broken_connection; // bool->int

            if (*buffer_start_seq > seq) [[unlikely]] // package arrived after its buffer being PUSHed
            {
                detail::warning("Package arrived its buffer being pushed\nBuffer Seq: {}, Packet Seq {}\n", *buffer_start_seq, seq);
                detail::print("Source port:      {}\n", source_port);
                detail::print("Destination port: {}\n", dest_port);
                detail::print("{:─^56}\n", "");
                return;
            }

            if (*buffer_start_seq + buffer->size() < seq + payload_length) [[likely]]
                buffer->resize(seq - *buffer_start_seq + payload_length); // needs to enlarge
            std::memcpy(&((*buffer)[seq - *buffer_start_seq]), &data[header_length], payload_length);
            break;

        case 0x11: // FIN, ACK
            detail::print("Flag: FIN, ACK\n");

            // set a finish mark
            tcp_map<Ver>[tcps].finished = 1;

            init_ack = tcp_map<Ver>[tcps].init_ack;
            init_seq = tcp_map<Ver>[tcps].init_seq;

            if (payload_length != 0) [[unlikely]]
            {
                buffer_start_seq = &tcp_map<Ver>[tcps].buffer_start_seq;
                buffer = &tcp_map<Ver>[tcps].buffer;

                if (*buffer_start_seq == 0) [[unlikely]]               // first arrival of this tcp connection
                    *buffer_start_seq = init_seq + !broken_connection; // bool->int

                if (*buffer_start_seq > seq) [[unlikely]] // package arrived after its buffer being PUSHed
                {
                    detail::warning("Package arrived its buffer being pushed\nBuffer Seq: {}, Packet Seq {}\n", *buffer_start_seq, seq);
                    detail::print("Source port:      {}\n", source_port);
                    detail::print("Destination port: {}\n", dest_port);
                    detail::print("{:─^56}\n", "");
                    return;
                }

                if (*buffer_start_seq + buffer->size() < seq + payload_length) [[likely]]
                    buffer->resize(seq - *buffer_start_seq + payload_length); // needs to enlarge
                // output_packet_to_console(&data[header_length], payload_length);
                std::memcpy(&((*buffer)[seq - *buffer_start_seq]), &data[header_length], payload_length);
            }

            if (tcp_map<Ver>[tcpr].finished && tcp_map<Ver>[tcps].finished) [[unlikely]]
            { // if both finished, flush the buffer
                tcp_map<Ver>[tcps].flush_buffer();
                tcp_map<Ver>[tcpr].flush_buffer();
                detail::print("Connection closed\n");
                break;
            }
            break;

        case 0x04: // RST
            detail::warning("Flag: RST\n");
            detail::print("Source port:      {}\n", source_port);
            detail::print("Destination port: {}\n", dest_port);
            detail::print("{:─^56}\n", "");
            if (tcp_map<Ver>.find(tcps) != tcp_map<Ver>.end())
                tcp_map<Ver>.erase(tcps);
            if (tcp_map<Ver>.find(tcpr) != tcp_map<Ver>.end())
                tcp_map<Ver>.erase(tcpr);
            return;

        case 0x14: // RST, ACK
            detail::warning("Flag: RST, ACK\n");
            detail::print("Source port:      {}\n", source_port);
            detail::print("Destination port: {}\n", dest_port);
            detail::print("{:─^56}\n", "");
            if (tcp_map<Ver>.find(tcps) != tcp_map<Ver>.end())
                tcp_map<Ver>.erase(tcps);
            if (tcp_map<Ver>.find(tcpr) != tcp_map<Ver>.end())
                tcp_map<Ver>.erase(tcpr);
            return;

        default:
            break;
        }

        detail::print("Relative seq {}, Absolute seq {}\n", seq - init_seq, seq);
        detail::print("Relative ack {}, Absolute ack {}\n", ack - init_ack, ack);
        detail::print("Source port:      {}\n", source_port);
        detail::print("Destination port: {}\n", dest_port);
        detail::print("Header length  {} bytes\n", header_length);
        detail::print("Payload length {} bytes\n", payload_length);
        detail::print("Window size    {} bytes\n", window);

        if (flag & 0x8) // PUSH
            tcp_map<Ver>[tcps].flush_buffer();
        else
            detail::print("{:─^56}\n", "");
    }

    template <IP_ver V>
    void UDP_handler(const u_char *data, IP_addr<V> source_ip, IP_addr<V> dest_ip, u_int length)
    {
        u_int source_port = GET_TWO_BYTE(0);
        u_int dest_port = GET_TWO_BYTE(2);
        NPAN_WARNING(length == GET_TWO_BYTE(4), "Different length given by the Internet Layer: {} and UDP packet {}", length, GET_TWO_BYTE(4));
        detail::print("Source port:      {}\n", source_port);
        detail::print("Destination port: {}\n", dest_port);
        detail::print("Total length  {} bytes\n", length);

        application_layer(&data[8], length - 8, Connection<Protocal::UDP, V>{source_ip, source_port, dest_ip, dest_port});
    }

    void ICMPv6_handler(const u_char *data, IPv6_addr source_ip, IPv6_addr dest_ip, u_int length)
    {
        detail::print("{:─^56}\n", "");
    }

    template <IP_ver V>
    void transport_layer(const u_char *data, Protocal protocal, IP_addr<V> source_ip, IP_addr<V> dest_ip, u_int length)
    {
        detail::print("{:─^56}\n", " Transport layer ");
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
                detail::print("{:─^56}\n", "");
            break;

        default:
            detail::print("{:─^56}\n", "");
            break;
        }
    }

    template void transport_layer(const u_char *, Protocal, IPv4_addr, IPv4_addr, u_int);
    template void transport_layer(const u_char *, Protocal, IPv6_addr, IPv6_addr, u_int);

} // namespace npan