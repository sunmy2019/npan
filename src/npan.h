#pragma once

#include <fmt/format.h>

namespace npan
{
    enum class Protocal;

    void physical_layer(unsigned char *data);

    void internet_layer(unsigned char *data, Protocal);

    void transport_layer(unsigned char *data, Protocal, uint64_t source_ip, uint64_t dest_ip, int length);

    void application_layer(std::unique_ptr<std::string> *data, Protocal, int length);

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
