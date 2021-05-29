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

    // about the protocal

    enum class Protocal;

    void physical_layer(u_char *data);

    void internet_layer(u_char *data, Protocal);

    void transport_layer(u_char *data, Protocal, uint64_t source_ip, uint64_t dest_ip, u_int length);

    void application_layer(std::unique_ptr<std::vector<u_char>> data, Protocal, u_int length);

    // main entry point
    void inline analyze_packet(u_char *data, int length)
    {
        physical_layer(data);
    }

    enum class Protocal
    {
        // physical layer
        ETHERNET,
        // Internet layer
        IPV4,
        IPV6,
        ARP,
        RARP,
        // Transport layer
        TCP,
        UDP,
        // Application layer
        HTTP,
        TLSv2,
        // Fall back
        UNKNOWN
    };

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
