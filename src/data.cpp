#include "utils.h"
#include <fstream>
#include <iostream>
#include <cassert>
namespace npan
{
    size_t read_packet_from_file(const char *filename, unsigned char *buffer)
    {
        std::ifstream inputFile(filename);
        auto iter = std::istreambuf_iterator<char>(inputFile);
        int current_position = 0;
        char current_character = 0;
        unsigned char tmp;
        while (iter != std::istreambuf_iterator<char>())
        {
            current_character = ('0' <= *iter && *iter <= '9') ? (*iter - '0') : (('a' <= *iter && *iter <= 'f') ? (*iter - 'a' + 10) : (*iter - 'A' + 10));
            assert(current_character < 16);
            if (++current_position % 2)
            {
                tmp = current_character << 4;
            }
            else
            {
                tmp |= current_character;
                *buffer++ = tmp;
            }

            ++iter;
        }
        return current_position / 2;
    }

    void output_packet_to_console(const unsigned char *packet, int length)
    {
        for (int i = 0; i < length; ++i)
        {
            fmt::print("{:02x} ", packet[i]);
        }
        fmt::print("\n");
    }
    
} // namespace npan