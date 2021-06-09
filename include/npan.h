#pragma once

/* npan user interface */

#include <fmt/format.h>
#include <vector>

namespace npan
{
    // represents a packet
    struct Packet
    {
        u_char *data;
        size_t length;

        Packet(u_char *d, size_t l) : data(d), length(l){};
        Packet(const Packet &) = delete;
        Packet &operator=(const Packet &) = delete;

        Packet(Packet &&) noexcept;
        Packet &operator=(Packet &&) = delete;

        ~Packet();
    };

    std::vector<Packet> read_packet_from_hex_dump(const char *filename);

    std::vector<Packet> read_packet_from_K12(const char *filename);

    void output_packet_to_console(const u_char *packet, int length);

    void dump_packet_to_file(FILE *file, const u_char *packet, int length);

    // main entry point
    void analyze_packet(u_char *data, int length);

} // namespace npan
