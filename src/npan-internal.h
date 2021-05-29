#pragma once

/* npan internal interface */
#include "npan.h"

#define LEFT_SHIFT(number, n) (number << (8 * n))
#define GET_TWO_BYTE(i) ((u_int16_t)(data[i] << 8) + (u_int16_t)data[i + 1])
#define GET_FOUR_BYTE(i) (((u_int32_t)data[i] << 24) | ((u_int32_t)data[i + 1] << 16) | ((u_int32_t)data[i + 2] << 8) | (u_int32_t)data[i + 3])

// #include <iostream>

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
        ICMPv6,
        // Application layer
        HTTP,
        TLSv2,
        // Fallback
        UNKNOWN
    };

    enum class IP_ver
    {
        FOUR,
        SIX
    };

    void physical_layer(u_char *data);

    void internet_layer(u_char *data, Protocal);

    template <IP_ver v>
    struct IP_addr;

    template <Protocal P, IP_ver V>
    struct Connection;

    template <IP_ver V>
    void transport_layer(u_char *data, Protocal, IP_addr<V> source_ip, IP_addr<V> dest_ip, u_int length);

    void application_layer(std::vector<u_char> data, u_int tcp_stream_no, Protocal);

    template <IP_ver V>
    void application_layer(std::vector<u_char> data, Connection<Protocal::UDP, V>, Protocal) {}

    template <>
    struct IP_addr<IP_ver::FOUR>
    {
        uint32_t first;
        friend std::strong_ordering operator<=>(const IP_addr &, const IP_addr &) = default;
    };

    template <>
    struct IP_addr<IP_ver::SIX>
    {
        uint64_t first;
        uint64_t last;
        friend std::strong_ordering operator<=>(const IP_addr &, const IP_addr &) = default;
    };

    template <Protocal P, IP_ver V>
    struct Connection
    {
        IP_addr<V> source_ip;
        u_int source_port;
        IP_addr<V> dest_ip;
        u_int dest_port;

        Connection get_conjugate()
        {
            return {dest_ip, dest_port, source_ip, source_port};
        }

        friend std::strong_ordering operator<=>(const Connection &, const Connection &) = default;
    };

    using IPv4_addr = IP_addr<IP_ver::FOUR>;
    using IPv6_addr = IP_addr<IP_ver::SIX>;

}

template <>
struct fmt::formatter<npan::IPv4_addr>
{
    constexpr auto parse(fmt::format_parse_context &ctx)
    {
        auto it = ctx.begin(), end = ctx.end();

        if (it != end && *it != '}')
            throw format_error("invalid format");

        // Return an iterator past the end of the parsed range:
        return it;
    }

    // Formats the ip address using the parsed format specification (presentation)
    // stored in this formatter.
    template <typename FormatContext>
    auto format(const npan::IPv4_addr &ip, FormatContext &ctx)
    {
        return format_to(ctx.out(), "{}.{}.{}.{}", (ip.first >> 24) & 0xff,
                         (ip.first >> 16) & 0xff, (ip.first >> 8) & 0xff, ip.first & 0xff);
    }
};

template <>
struct fmt::formatter<npan::IPv6_addr> : public fmt::formatter<string_view>
{
    constexpr auto parse(fmt::format_parse_context &ctx)
    {
        auto it = ctx.begin(), end = ctx.end();

        if (it != end && *it != '}')
            throw format_error("invalid format");

        // Return an iterator past the end of the parsed range:
        return it;
    }

    // Formats the ip address using the parsed format specification (presentation)
    // stored in this formatter.
    template <typename FormatContext>
    auto format(const npan::IPv6_addr &ip, FormatContext &ctx)
    {
        fmt::memory_buffer out;

        int status = 0;
        // 0: haven't met consecutive 0s
        // 1: current in omitted state
        // 2: already omitted

        for (int i = 48; i >= 0; i -= 16)
        {
            u_int16_t temp = (ip.first >> i) & 0xffff;

            if (temp == 0 && status == 0)
            {
                status = 1;
                format_to(out, "{::^{}}", "", i == 48 ? 2 : 1);
            }
            else if (temp != 0 && status == 1)
            {
                status = 2;
            }
            if (status != 1)
            {
                format_to(out, "{:x}:", temp);
            }
        }

        for (int i = 48; i >= 0; i -= 16)
        {
            u_int16_t temp = (ip.last >> i) & 0xffff;

            if (temp == 0 && status == 0)
            {
                status = 1;
                format_to(out, ":");
            }
            else if (temp != 0 && status == 1)
            {
                status = 2;
            }
            if (status != 1)
            {
                format_to(out, "{:x}{}", temp, i ? ':' : '\0');
            }
        }
        
        string_view sv{out.data(), out.size()};
        return fmt::formatter<string_view>::format(sv, ctx);
    }
};
