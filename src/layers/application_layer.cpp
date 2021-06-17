#include "../npan-internal.h"

namespace npan
{
    void application_layer(std::vector<u_char> data, u_int tcp_stream_no)
    {
        detail::print("{:─^56}\n", " Application layer ");
        detail::print("TCP stream number {} with payload length: {} bytes\n", (tcp_stream_no + 1) >> 1, data.size());
        output_packet_to_console(&data[0], data.size());
        detail::print("{:─^56}\n", "");
    }

    template <IP_ver V>
    void application_layer(const u_char *data, u_int length, Connection<Protocal::UDP, V>)
    {
        detail::print("{:─^56}\n", " Application layer ");
        detail::print("UDP stream with payload length: {} bytes\n", length);
        output_packet_to_console(&data[0], length);
        detail::print("{:─^56}\n", "");
    }

    template void application_layer(const u_char *, u_int, Connection<Protocal::UDP, IP_ver::FOUR>);
    template void application_layer(const u_char *, u_int, Connection<Protocal::UDP, IP_ver::SIX>);
} // namespace npan