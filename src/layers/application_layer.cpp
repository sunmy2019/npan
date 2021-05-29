#include "../npan-internal.h"

namespace npan
{
    void application_layer(std::vector<u_char> data, u_int tcp_stream_no, Protocal)
    {
        fmt::print("{:─^56}\n", " Application layer ");
        fmt::print("TCP stream no {}\n", (tcp_stream_no + 1) >> 1);
        output_packet_to_console(&data[0], data.size());
        fmt::print("{:─^56}\n", "");
    }
} // namespace npan