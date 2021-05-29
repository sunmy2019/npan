#pragma once

/* npan internal interface */
#include "npan.h"

#define LEFT_SHIFT(number, n) (number << (8 * n))
#define GET_TWO_BYTE(i) ((data[i] << 8) + data[i + 1])
#define GET_FOUR_BYTE(i) ((data[i] << 24) | (data[i + 1] << 16) | (data[i + 2] << 8) | data[i + 3])


namespace npan
{
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

    enum class IP_version
    {
        FOUR,
        SIX
    };

    void physical_layer(u_char *data);

    void internet_layer(u_char *data, Protocal);

    template <IP_version v>
    struct IP_address;

    template <Protocal P, IP_version V>
    struct Connection;

    template <IP_version V>
    void transport_layer(u_char *data, Protocal, IP_address<V> source_ip, IP_address<V> dest_ip, u_int length);

    void application_layer(std::vector<u_char> data, u_int tcp_stream_no, Protocal);

    template <IP_version V>
    void application_layer(std::vector<u_char> data, Connection<Protocal::UDP, V>, Protocal);

    template <>
    struct IP_address<IP_version::FOUR>
    {
        uint32_t first;
        friend std::strong_ordering operator<=>(const IP_address &, const IP_address &) = default;
    };

    template <>
    struct IP_address<IP_version::SIX>
    {
        uint64_t first;
        uint64_t last;
        friend std::strong_ordering operator<=>(const IP_address &, const IP_address &) = default;
    };

    template <Protocal P, IP_version V>
    struct Connection
    {
        IP_address<V> source_ip;
        u_int source_port;
        IP_address<V> dest_ip;
        u_int dest_port;

        Connection get_conjugate()
        {
            return {dest_ip, dest_port, source_ip, source_port};
        }

        friend std::strong_ordering operator<=>(const Connection &, const Connection &) = default;
    };

}
