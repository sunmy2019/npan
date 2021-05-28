
namespace npan
{
    enum class Protocal;

    void physical_layer(unsigned char *data, int length);

    void internet_layer(unsigned char *data, Protocal, int length);

    void transport_layer(unsigned char *data, Protocal, int length);

    void application_layer(unsigned char *data, Protocal, int length);

    // main entry point
    void inline analyze_packet(unsigned char *data, int length)
    {
        physical_layer(data, 0);
    }

    enum class Protocal
    {
        // physical layer
        ETHERNET,
        // Internet layer
        IPV4,
        IPV6,
        ARP,
        RARP,
        // Transport layer
        TCP,
        UDP,
        // Application layer
        HTTP,
        // Fall back
        UNKNOWN
    };

} // namespace npan
