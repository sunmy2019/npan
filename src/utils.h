#pragma once

#include "npan.h"

#define LEFT_SHIFT(number, n) (number << (8 * n))
#define GET_TWO_BYTE(i) ((data[i] << 8) + data[i + 1])
#define GET_FOUR_BYTE(i) ((data[i] << 24) | (data[i + 1] << 16) | (data[i + 2] << 8) | data[i + 3])
namespace npan
{
    enum class IP_version
    {
        FOUR,
        SIX
    };

    template <>
    struct IP_address<IP_version::FOUR>
    {
        uint32_t first;
    };

    template <>
    struct IP_address<IP_version::SIX>
    {
        uint64_t first;
        uint64_t last;
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
