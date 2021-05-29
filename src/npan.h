#pragma once

#include <fmt/format.h>
#include <vector>

namespace npan
{

    // represents a packet
    struct Packet;

    std::vector<Packet> read_packet_from_file(const char *filename);

    void output_packet_to_console(const unsigned char *packet, int length);

    void dump_packet_to_file(FILE *file, const unsigned char *packet, int length);

    // about the protocal

    enum class Protocal;

    void physical_layer(unsigned char *data);

    void internet_layer(unsigned char *data, Protocal);

    void transport_layer(unsigned char *data, Protocal, uint64_t source_ip, uint64_t dest_ip, int length);

    void application_layer(std::unique_ptr<std::vector<unsigned char>> data, Protocal, int length);

    // main entry point
    void inline analyze_packet(unsigned char *data, int length)
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
        unsigned char *data;
        size_t length;

        Packet(unsigned char *d, size_t l) : data(d), length(l){};
        Packet(const Packet &) = delete;
        Packet &operator=(const Packet &) = delete;

        Packet(Packet &&) noexcept;
        Packet &operator=(Packet &&) = delete;

        ~Packet();
    };

} // namespace npan
