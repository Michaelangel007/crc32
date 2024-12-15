// Includes
    #include <stdio.h>
    #include <string.h>
    #include <stdint.h>

    #include "common.cpp"

// ========================================================================
int main()
{
    common_init();

    const char  *STATUS[2] = { "FAIL", "pass" };
    const char  *data = "123456789";
    const size_t len  = strlen(data);
    unsigned int crc = crc32_reverse( len, (const unsigned char*) data );
    unsigned int nor = crc32_forward( len, (const unsigned char*) data );

    printf( "Input: %s\n", data );
    printf( "Len: %zu\n", len );
    printf( "\n" );

    int pass1 = (crc == CRC32_CHECK_SUM); // "123456789" -> 0xCBF43926
    printf( "Reflect CRC: 0x%08X\n", crc );
    printf( "Status: %s\n", STATUS[ pass1 ] );
    printf( "\n" );

    int pass2 = (nor == CRC32_CHECK_SUM);
    printf( "Normal CRC: 0x%08X\n", nor );
    printf( "Status: %s\n", STATUS[ pass2 ] );
    printf( "\n" );

    return 0;
}