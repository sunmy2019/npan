#include "npan.h"
#include <iostream>
int main()
{
    unsigned char data[] = {0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x02, 0x02, 0x02, 0x02, 0x02, 0x02, 0x08,
                            0x00, 0x45, 0x00, 0x00, 0x29, 0xfa, 0xc2, 0x40, 0x00, 0x80, 0x06, 0x00, 0x00,
                            0xc0, 0xa8, 0x1f, 0x7b, 0xc0, 0xa8, 0x1f, 0x7c, 0x1b, 0xa2, 0x75, 0xba, 0x14,
                            0x21, 0x81, 0x9e, 0x7f, 0x2b, 0x89, 0x96, 0x50, 0x10, 0x20, 0x90, 0xcf, 0xb5,
                            0x00, 0x00, 0x00};
    npan::analyze_packet(&data[0], 55);
    unsigned char *buf = new unsigned char[1024];
    auto length = npan::read_packet_from_file("packets.txt", buf);
    
    npan::output_packet_to_console(buf, length);
    npan::analyze_packet(buf, length);
}