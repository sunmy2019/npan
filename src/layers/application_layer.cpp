#include "../utils.h"

namespace npan
{
    void application_layer(std::unique_ptr<std::vector<unsigned char>> data, Protocal, unsigned int length)
    {
        output_packet_to_console(&((*data)[0]), data->size());
    }

} // namespace npan