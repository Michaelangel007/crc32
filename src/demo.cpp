#include <stdio.h>
#include <string.h>
#include <stdint.h>

    uint32_t REVERSE[ 256 ]; // 8-bit reverse bit look-up table
    unsigned int reflect32( const unsigned int x )
    {
        unsigned int bits = 0;
        unsigned int mask = x;

        for( int i = 0; i < sizeof(x)*8; i++ )
        {
            bits <<= 1;
            if( mask & 1 )
                bits |= 1;
            mask >>= 1;
        }

        return bits;
    }

    uint32_t reverse32( const uint32_t x )
    {
        return 0
        | REVERSE[ (x >> 24) & 0xFF ] <<  0L
        | REVERSE[ (x >> 16) & 0xFF ] <<  8L
        | REVERSE[ (x >>  8) & 0xFF ] << 16L
        | REVERSE[ (x >>  0) & 0xFF ] << 24L;
    }

   void Reverse_Init()
    {
        for( int byte = 0; byte < 256; byte++ )
            REVERSE[ byte ] = (reflect32( byte ) >> 24) & 0xFF;
    }

const unsigned int CRC32_POLY    = 0x04C11DB7; // normal = shift left
const unsigned int CRC32_REVERSE = 0xEDB88320; // reverse = shift right

/* */ unsigned int CRC32_FORWARD[256]; // init with poly = 0x04C11DB7 
/* */ unsigned int CRC32_REFLECT[256]; // init with poly = 0xEDB88320

void CRC32_Init()
{
    for( unsigned short byte = 0; byte < 256; byte++ )
    {
        unsigned int crc = (unsigned int) byte;
        for( char bit = 0; bit < 8; bit++ )
            if( crc & 1 ) crc = (crc >> 1) ^ CRC32_REVERSE; // reverse/reflected Form
            else          crc = (crc >> 1);
        CRC32_REFLECT[ byte ] = crc;
    }

    for( unsigned short byte = 0; byte < 256; byte++ )
    {
        unsigned int crc = (unsigned int) byte << 24;
        for( char bit = 0; bit < 8; bit++ )
            if( crc & (1L << 31)) crc = (crc << 1) ^ CRC32_POLY; // forward Form
            else                  crc = (crc << 1);
        CRC32_FORWARD[ byte ] = crc;
    }

    if (CRC32_REFLECT[8] != (CRC32_REVERSE >> 4))
        printf("ERROR: CRC32 Reverse Table not initialized properly! 0x%08X != 0x%08X\n", CRC32_REFLECT[8], CRC32_REVERSE );

    if (CRC32_FORWARD[1] != CRC32_POLY)
        printf("ERROR: CRC32 Forward Table not initialized properly! 0x%08x != 0x%08X\n", CRC32_FORWARD[1], CRC32_POLY );

    if (CRC32_FORWARD[16] != (CRC32_POLY << 4))
        printf("ERROR: CRC32 Forward Table not initialized properly! 0x%08x != 0x%08X\n", CRC32_FORWARD[16], CRC32_POLY );
}

unsigned int crc32_reverse( const unsigned char *pData, int nLength )
{
    unsigned int crc = -1; // CRC32_INIT
    while( nLength --> 0 )
        crc = CRC32_REFLECT[ (crc         ^         *pData++) & 0xFF ] ^ (crc >> 8); // reverse/reflected form
    return ~crc; // ^CRC32_DONE
}

unsigned int crc32_forward( const unsigned char *pData, int nLength )
{
    unsigned int crc = -1; // CRC32_INIT
    while( nLength --> 0 )
        crc = CRC32_FORWARD[ ((crc >> 24) ^ REVERSE[*pData++]) & 0xFF ] ^ (crc << 8); // normal form
    return reflect32(~crc); // ^CRC32_DONE
}

int main()
{
    CRC32_Init();
    Reverse_Init();

    const char *STATUS[2] = { "FAIL", "pass" };
    const char *data = "123456789";
    const int   len  = strlen(data);
    unsigned int crc = crc32_reverse( (unsigned char*) data, len );
    unsigned int nor = crc32_forward( (unsigned char*) data, len );

    printf( "Input: %s\n", data );
    printf( "Len: %d\n", len );
    printf( "\n" );

    int pass1 = (crc == 0xCBF43926); // "123456789" -> 0xCBF43926
    printf( "Reflect CRC: 0x%08X\n", crc );
    printf( "Status: %s\n", STATUS[ pass1 ] );
    printf( "\n" );

    int pass2 = (nor == 0xCBF43926);
    printf( "Normal CRC: 0x%08X\n", nor );
    printf( "Status: %s\n", STATUS[ pass2 ] );
    printf( "\n" );

    return 0;
}