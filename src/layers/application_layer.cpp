#include "../npan-internal.h"

namespace npan
{
    void application_layer(std::vector<u_char> data, u_int tcp_stream_no)
    {
        fmt::print("{:─^56}\n", " Application layer ");
        fmt::print("TCP stream number: {}\n", (tcp_stream_no + 1) >> 1);
        output_packet_to_console(&data[0], data.size());
        fmt::print("{:─^56}\n", "");
    }

    template <IP_ver V>
    void application_layer(u_char *data, u_int length, Connection<Protocal::UDP, V>)
    {
        fmt::print("{:─^56}\n", " Application layer ");
        fmt::print("UDP stream\n");
        output_packet_to_console(&data[0], length);
        fmt::print("{:─^56}\n", "");
    }

    template void application_layer(u_char *, u_int, Connection<Protocal::UDP, IP_ver::FOUR>);
    template void application_layer(u_char *, u_int, Connection<Protocal::UDP, IP_ver::SIX>);
} // namespace npan