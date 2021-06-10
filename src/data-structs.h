#include "npan.h"

#define LEFT_SHIFT(number, n) (number << (8 * n))
#define GET_TWO_BYTE(i) ((u_int16_t)(data[i] << 8) + (u_int16_t)data[i + 1])
#define GET_FOUR_BYTE(i) (((u_int32_t)data[i] << 24) | ((u_int32_t)data[i + 1] << 16) | ((u_int32_t)data[i + 2] << 8) | (u_int32_t)data[i + 3])

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

    void application_layer(std::vector<u_char> data, u_int tcp_stream_no);

    template <IP_ver V>
    void application_layer(u_char *data, u_int length, Connection<Protocal::UDP, V>);

    template <>
    struct IP_addr<IP_ver::FOUR>
    {
        u_int32_t first;
        friend std::strong_ordering operator<=>(const IP_addr &, const IP_addr &) = default;
    };

    template <>
    struct IP_addr<IP_ver::SIX>
    {
        u_int64_t first;
        u_int64_t last;
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

namespace std
{
#define NPAN_BITXOR_(typea, a, typeb, b) (((std::hash<typea>()(a) << 1) ^ std::hash<typeb>()(b)) >> 1)
#define NPAN_BITXOR__(a, typeb, b) (((a << 1) ^ std::hash<typeb>()(b)) >> 1)
    template <>
    struct hash<npan::IPv4_addr>
    {
        std::size_t operator()(const npan::IPv4_addr &k) const
        {
            return std::hash<u_int32_t>()(k.first);
        }
    };

    template <>
    struct hash<npan::IPv6_addr>
    {
        std::size_t operator()(const npan::IPv6_addr &k) const
        {
            return NPAN_BITXOR_(u_int32_t, k.first, u_int32_t, k.last);
        }
    };

    template <npan::Protocal P, npan::IP_ver V>
    struct hash<npan::Connection<P, V>>
    {
        std::size_t operator()(const npan::Connection<P, V> &k) const
        {
            return NPAN_BITXOR__(
                NPAN_BITXOR__(
                    NPAN_BITXOR_(npan::IP_addr<V>, k.source_ip, u_int, k.source_port),
                    npan::IP_addr<V>, k.dest_ip),
                u_int, k.dest_port);
        }
    };

#undef NPAN_BITXOR_
#undef NPAN_BITXOR__
} // namespace std

template <>
struct fmt::formatter<npan::IPv4_addr>
{
    constexpr auto parse(fmt::format_parse_context &ctx) // does not support wide char stream
    {
        auto it = ctx.begin(), end = ctx.end();

        if (it != end && *it != '}')
            throw format_error("invalid format");

        // Return an iterator past the end of the parsed range:
        return it;
    }

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
    constexpr auto parse(fmt::format_parse_context &ctx) // does not support wide char stream
    {
        auto it = ctx.begin(), end = ctx.end();

        if (it != end && *it != '}')
            throw format_error("invalid format");

        // Return an iterator past the end of the parsed range:
        return it;
    }

    template <typename FormatContext>
    auto format(const npan::IPv6_addr &ip, FormatContext &ctx)
    {
        fmt::memory_buffer buf;
        auto back_inserter = std::back_inserter(buf);

        char status = 0;
        // 0: haven't met consecutive 0s
        // 1: current in omitted state
        // 2: already omitted

        for (int i = 48; i >= 0; i -= 16)
        {
            u_int16_t temp = (ip.first >> i) & 0xffff;

            if (temp == 0 && status == 0)
            {
                status = 1;
                format_to(back_inserter, "{::^{}}", "", i == 48 ? 2 : 1);
            }
            else if (temp != 0 && status == 1)
            {
                status = 2;
            }
            if (status != 1)
            {
                format_to(back_inserter, "{:x}:", temp);
            }
        }

        for (int i = 48; i >= 0; i -= 16)
        {
            u_int16_t temp = (ip.last >> i) & 0xffff;

            if (temp == 0 && status == 0)
            {
                status = 1;
                format_to(back_inserter, ":");
            }
            else if (temp != 0 && status == 1)
            {
                status = 2;
            }
            if (status != 1)
            {
                format_to(back_inserter, "{:x}{}", temp, i ? ':' : '\0');
            }
        }

        string_view sv{buf.data(), buf.size()};
        return fmt::formatter<string_view>::format(sv, ctx);
    }
};
