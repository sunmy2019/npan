#include "npan.h"
#include <sys/stat.h>
// #include <iostream>

int main(int argc, char *argv[])
{
    if (argc != 2)
    {
        fmt::print(stderr, "Usage: {} filename.\n", argv[0]);
        exit(-1);
    }
    {
        struct stat buffer;
        int exist = stat(argv[1], &buffer);
        if (exist != 0)
        {
            fmt::print(stderr, "\"{}\" does not exist.\n", argv[1]);
            exit(-1);
        }
    }

    std::vector<npan::Packet> vec_pac = npan::read_packet_from_file(argv[1]);

    for (auto &pac : vec_pac)
    {
        npan::output_packet_to_console(pac.data, pac.length);
        npan::analyze_packet(pac.data, pac.length);
    }
}