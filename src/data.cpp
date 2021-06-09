#include "npan-internal.h"
#include <fstream>
// #include <iostream>
#include <cassert>
namespace npan
{
    Packet::Packet(Packet &&other) noexcept : data(other.data), length(other.length)
    {
        other.data = nullptr;
    }

    Packet::~Packet()
    {
        if (data)
            delete[] data;
    }

    std::vector<Packet> read_packet_from_hex_dump(const char *filename)
    {
        std::vector<Packet> rt;
        u_char *buf = new u_char[65536];
        u_char *buffer;

        std::ifstream inputFile(filename);
        auto iter = std::istreambuf_iterator<char>(inputFile);

        while (iter != std::istreambuf_iterator<char>())
        {
            if (!(('0' <= *iter && *iter <= '9') || ('a' <= *iter && *iter <= 'f') || ('A' <= *iter && *iter <= 'F')))
            {
                ++iter;
                continue;
            }
            buffer = buf;
            int current_position = 0;
            char current_character = 0;
            u_char tmp = 0;
            while (iter != std::istreambuf_iterator<char>() && *iter != '\n')
            {
                current_character = ('0' <= *iter && *iter <= '9') ? (*iter - '0') : (('a' <= *iter && *iter <= 'f') ? (*iter - 'a' + 10) : (*iter - 'A' + 10));
                assert(current_character < 16); // otherwise unallowed character has appeared
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
            current_position /= 2;
            u_char *data = new u_char[current_position + 1];
            memcpy(data, buf, current_position);
            rt.emplace_back(data, current_position);
        }
        delete[] buf;
        return rt;
    }

    std::vector<Packet> read_packet_from_K12(const char *filename)
    {
        std::vector<Packet> rt;
        u_char *buf = new u_char[65536];
        u_char *buffer;

        std::ifstream inputFile(filename);
        auto iter = std::istreambuf_iterator<char>(inputFile);

        while (iter != std::istreambuf_iterator<char>())
        {
            if (*iter != '+')
            {
                ++iter;
                continue;
            }
            // We met a '+'
            while (iter != std::istreambuf_iterator<char>() && *iter != '\n')
                ++iter;
            ++iter;

            // Now Entering second line
            while (iter != std::istreambuf_iterator<char>() && *iter != '\n')
                ++iter;
            ++iter;

            // Now Entering third line
            assert(*iter == '|');
            ++iter; // now is '0'
            ++iter; // now is  ' '
            ++iter; // now is  ' '
            ++iter; // now is  ' '
            ++iter; // now is  '|'

            buffer = buf;
            int current_position = 0;
            char current_character = 0;
            u_char tmp = 0;
            while (iter != std::istreambuf_iterator<char>() && *iter != '\n')
            {
                if (*iter == '|')
                {
                    ++iter;
                    continue;
                }
                current_character = ('0' <= *iter && *iter <= '9') ? (*iter - '0') : (('a' <= *iter && *iter <= 'f') ? (*iter - 'a' + 10) : (*iter - 'A' + 10));
                assert(current_character < 16); // otherwise unallowed character has appeared
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
            current_position /= 2;
            u_char *data = new u_char[current_position + 1];
            memcpy(data, buf, current_position);
            rt.emplace_back(data, current_position);
        }
        delete[] buf;
        return rt;
    }

    void output_packet_to_console(const u_char *packet, int length)
    {
        for (int i = 0; i < length; ++i)
        {
            fmt::print("{:02x} ", packet[i]);
        }
        fmt::print("\n");
    }

    void dump_packet_to_file(FILE *file, const u_char *packet, int length)
    {
        for (int i = 0; i < length; ++i)
        {
            fmt::print(file, "{:02x}", packet[i]);
        }
        fmt::print(file, "\n");
    }

} // namespace npan