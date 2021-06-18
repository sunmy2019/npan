#include "npan-internal.h"
#include <fcntl.h>
#include <fstream>
#include <sys/mman.h>
#include <sys/stat.h>

namespace npan
{
    Packet::Packet(Packet &&other) noexcept : data(other.data), length(other.length)
    {
        other.data = nullptr;
    }

    Packet &Packet::operator=(Packet &&other) noexcept
    {
        // because this function never throw, we can release resources first
        if (data)
            delete[] data;

        data = other.data;
        length = other.length;

        other.data = nullptr;
        other.length = 0;
        return *this;
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
            u_int current_position = 0;
            char current_character = 0;
            u_char tmp = 0;
            while (iter != std::istreambuf_iterator<char>() && *iter != '\n')
            {
                current_character = ('0' <= *iter && *iter <= '9') ? (*iter - '0') : (('a' <= *iter && *iter <= 'f') ? (*iter - 'a' + 10) : (*iter - 'A' + 10));
                NPAN_ASSERT(current_character < 16, "Unallowed character '{}'! (ascii: {})\n", *iter, (u_int)(*iter));
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
            std::memcpy(data, buf, current_position);
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
            NPAN_ASSERT(*iter == '|', "Wrong K12 format! Expecting '|' here, but got '{}'", *iter);
            ++iter; // now is '0'
            ++iter; // now is  ' '
            ++iter; // now is  ' '
            ++iter; // now is  ' '
            ++iter; // now is  '|'

            buffer = buf;
            u_int current_position = 0;
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
                NPAN_ASSERT(current_character < 16, "Unallowed character '{}'! (ascii: {})\n", *iter, (u_int)(*iter));
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
            std::memcpy(data, buf, current_position);
            rt.emplace_back(data, current_position);
        }
        delete[] buf;
        return rt;
    }

    std::vector<Packet> read_packet_from_pcap(const char *filename)
    {
        std::vector<Packet> rt;
        int fd = ::open(filename, O_RDONLY);
        NPAN_ASSERT(fd >= 0, "Cannot open \"{}\"\n", filename);
        size_t file_length;

        {
            struct stat filestat;
            ::fstat(fd, &filestat);
            NPAN_ASSERT(filestat.st_size > 0, "File \"{}\"'s size < 0\n", filename);
            file_length = filestat.st_size;
        }

        void *const start_addr = ::mmap(NULL, file_length, PROT_READ, MAP_PRIVATE, fd, 0);
        ::close(fd);

        NPAN_ASSERT(start_addr != MAP_FAILED, "mmap faild.\n");

        u_char *buffer = static_cast<u_char *>(start_addr);

        if (GET_FOUR_BYTE_LE(buffer, 0) == 0xa1b2c3d4) [[likely]]
        {
            // Little endian
            NPAN_WARNING(GET_TWO_BYTE_LE(buffer, 4) == 2 && GET_TWO_BYTE_LE(buffer, 6) == 4, "Only PACAP version 2.4 is supported.\n");

            buffer += 24;

            while (buffer + 16 <= static_cast<u_char *>(start_addr) + file_length)
            { // still have something to extract
                u_int packet_length = GET_FOUR_BYTE_LE(buffer, 8);
                u_char *data;
                try
                {
                    data = new u_char[packet_length];
                }
                catch (std::bad_alloc &)
                {
                    NPAN_ASSERT(0, "Allocation failure. Packet length is {} bytes.\n", packet_length);
                }
                std::memcpy(data, buffer + 16, packet_length);
                rt.emplace_back(data, packet_length);
                buffer += 16 + packet_length;
            }
        }
        else if (GET_FOUR_BYTE_BE(buffer, 0) == 0xa1b2c3d4) [[likely]]
        {
            // Big endian
            NPAN_WARNING(GET_TWO_BYTE_BE(buffer, 4) == 2 && GET_TWO_BYTE_BE(buffer, 6) == 4, "Only PACAP version 2.4 is supported.\n");

            buffer += 24;

            while (buffer + 16 <= static_cast<u_char *>(start_addr) + file_length)
            { // still have something to extract
                u_int packet_length = GET_FOUR_BYTE_BE(buffer, 8);
                u_char *data;
                try
                {
                    data = new u_char[packet_length];
                }
                catch (std::bad_alloc &)
                {
                    NPAN_ASSERT(0, "Allocation failure. Packet length is {} bytes.\n", packet_length);
                }
                std::memcpy(data, buffer + 16, packet_length);
                rt.emplace_back(data, packet_length);
                buffer += 16 + packet_length;
            }
        }
        else
        {
            NPAN_ASSERT(0, "Unknown endianess.\n");
        }

        NPAN_WARNING(buffer == static_cast<u_char *>(start_addr) + file_length, "PCAP format error.\n");

        ::munmap(start_addr, file_length);
        return rt;
    }

    void output_packet_to_console(const u_char *packet, u_int length)
    {
        for (u_int i = 0; i < length; ++i)
        {
            detail::print("{:02x} ", packet[i]);
        }
        detail::print("\n");
    }

    void dump_packet_to_file(FILE *file, const u_char *packet, u_int length)
    {
        for (u_int i = 0; i < length; ++i)
        {
            detail::print(file, "{:02x}", packet[i]);
        }
        detail::print(file, "\n");
    }

} // namespace npan