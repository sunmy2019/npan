#include "../utils.h"

namespace npan
{
    void application_layer(std::unique_ptr<std::vector<u_char>> data, Protocal, u_int length)
    {
        fmt::print("{:─^56}\n", " Application layer ");
        output_packet_to_console(&((*data)[0]), data->size());
        fmt::print("{:─^56}\n", "");
    }

} // namespace npan