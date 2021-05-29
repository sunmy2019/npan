#pragma once

#include <fmt/format.h>
#include <vector>

namespace npan
{
    // represents a packet
    struct Packet;

    std::vector<Packet> read_packet_from_file(const char *filename);

    void output_packet_to_console(const u_char *packet, int length);

    void dump_packet_to_file(FILE *file, const u_char *packet, int length);

    // about the protocals
    enum class Protocal
    {
        // physical layer
        ETHERNET,
        // Internet layer
        IPV4,
        IPV6,
        ARP,
        // Transport layer
        TCP,
        UDP,
        // Application layer
        HTTP,
        TLSv2,
        // Fallback
        UNKNOWN
    };

    void physical_layer(u_char *data);

    void internet_layer(u_char *data, Protocal);


    enum class IP_version;

    template <IP_version v>
    struct IP_address;

    template <Protocal P, IP_version V>
    struct Connection;



    template <IP_version V>
    void transport_layer(u_char *data, Protocal, IP_address<V> source_ip, IP_address<V> dest_ip, u_int length);

    void application_layer(std::vector<u_char> data, u_int tcp_stream_no, Protocal);

    template <IP_version V>
    void application_layer(std::vector<u_char> data, Connection<Protocal::UDP, V>, Protocal);

    // main entry point
    void inline analyze_packet(u_char *data, int length)
    {
        physical_layer(data);
    }

    struct Packet
    {
        u_char *data;
        size_t length;

        Packet(u_char *d, size_t l) : data(d), length(l){};
        Packet(const Packet &) = delete;
        Packet &operator=(const Packet &) = delete;

        Packet(Packet &&) noexcept;
        Packet &operator=(Packet &&) = delete;

        ~Packet();
    };

} // namespace npan
