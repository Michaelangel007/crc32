/*

"CRC32 Demystified"
https://github.com/Michaelangel007/crc32

Michaelangel007
Copyleft (C) 2017

*/

// Includes
    #include "common.cpp"

// ========================================================================
int main()
{
    common_init( true );

    dump( "Reverse Bits:", REVERSE_BITS );

#if 0

    uint32_t x = 0xFF000000;
    printf( "Reflect: %08X -> %08X\n", x, reflect32( x ) );

    // Verify reverse table == reflect formula
    for( int byte = 0; byte < 256; byte++ )
    {
        printf( "%02X -> %08X == %08X %s\n"
            , byte, reflect32( byte ), reverse32( byte )
            , reflect32( byte ) == reverse32( byte )
                ? "pass"
                : "FAIL"
        );
    }
#endif

    return 0;
}

