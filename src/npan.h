#pragma once

#include <fmt/format.h>
#include <vector>

namespace npan
{
    enum class Protocal;

    size_t read_packet_from_file(const char *filename, unsigned char *buffer);

    void output_packet_to_console(const unsigned char *packet, int length);

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

} // namespace npan
