/*

"CRC32 Demystified"
https://github.com/Michaelangel007/crc32

Michaelangel007
Copyleft (C) 2017

*/

// Includes
    #include "common.h"

// Implememtation

    // ========================================================================
    uint32_t crc32_trace_normal( const uint32_t *CRC32, size_t len, const void *data )
    {
        const unsigned char *buffer = (const unsigned char*) data;
        uint32_t crc = -1;
        uint32_t i;

printf( "crc32=%08X\n", crc );
        while( len-- )
{
i = (reverse[ *buffer ] ^ (crc >> 24)) & 0xFF;
printf( "^buf[%04X]: %02X -> %02X bits reversed\n", (buffer - (const unsigned char*)data) & 0xFFFF, *buffer, reverse[ *buffer ] );
printf( "   = crc32[ %02X ]: %08X ^ %06X__\n", i, bCRC32[ i ], crc & 0xFFFFFF );

           crc = (crc << 8) ^ CRC32[ (reverse[*buffer++] ^ (crc >> 24)) & 0xFF ];

printf( "crc32=%08X\n", crc );
}
printf( "    ~=%08X\n", ~crc );
printf( "     =%08X bits reversed\n", reflect32( ~crc ) );

        return reflect32( ~crc );
    }

// Normal form where user didn't reverse the bits
    // ========================================================================
    uint32_t crc32_trace_normal_broken( const uint32_t *CRC32, size_t len, const void *data )
    {
        const unsigned char *buffer = (const unsigned char*) data;
        uint32_t crc = -1;
        uint32_t i;

printf( "crc32=%08X\n", crc );
        while( len-- )
{
i = (reverse[ *buffer ] ^ (crc >> 24)) & 0xFF;
printf( "^buf[%04X]: %02X                      \n", (buffer - (const unsigned char*)data) & 0xFFFF, *buffer );
printf( "   = crc32[ %02X ]: %08X ^ %06X__\n", i, bCRC32[ i ], crc & 0xFFFFFF );

           crc = (crc << 8) ^ CRC32[ (*buffer++ ^ (crc >> 24)) & 0xFF ];

printf( "crc32=%08X\n", crc );
}
printf( "    ~=%08X\n", ~crc );

        return ~crc;
    }

    // ========================================================================
    uint32_t crc32_reflect_trace( const uint32_t *CRC32, size_t len, const void *data )
    {
        const unsigned char *buffer = (const unsigned char*) data;
        uint32_t crc = -1;
        uint32_t i;

printf( "crc32=%08X\n", crc );
        while( len-- )
{
i = (crc ^ *buffer) & 0xFF;
printf( "^buf[%04X]: %02X\n", (buffer - (const unsigned char*)data) & 0xFFFF, *buffer );
printf( "   = crc32[ %02X ]: %08X ^ __%06X\n", i, aCRC32[ i ], (crc >> 8) & 0xFFFFFF );

            crc = CRC32[ (crc ^ *buffer++) & 0xFF ] ^ (crc >> 8);

printf( "crc32=%08X\n", crc );
}
printf( "    ~=%08X\n", ~crc );

        return ~crc;
    }


// ========================================================================
void trace( const int bytes, const char *data )
{
    uint32_t crc;
    int check = (strcmp( data, CRC32_CHECK_TXT ) == 0);

    printf( "========== Bytes: %d ==========\n", bytes );
    printf( "[ " );
    for( int i = 0; i < bytes; i++ )
        printf( "%02X, ", data[ i ] & 0xFF );
    printf( "]\n" );
    printf( "\n" );

    printf( "---------- Normal  ----------\n" );
    crc = crc32_trace_normal( aCRC32, bytes, data );
    if( check )
        printf( "%s\n", crc == CRC32_CHECK_SUM
            ? "pass"
            : "FAIL"
        );
    printf( "\n" );

    if( check )
    {
        printf( "---------- Mismatched Polynomial and Calculation  ----------\n" );
        crc = crc32_trace_normal_broken( bCRC32, bytes, data );
        if( check )
            printf( "%s\n", crc == CRC32_CHECK_SUM
                ? "pass"
                : "FAIL"
            );
        printf( "\n" );
    }

    printf( "---------- Reflect ==========\n" );
    crc = crc32_reflect_trace( dCRC32, bytes, data );
    if( check )
        printf( "%s\n", crc == CRC32_CHECK_SUM
            ? "pass"
            : "FAIL"
        );
    printf( "\n" );
}

// ========================================================================
int main()
{
    common_init();

    char buffer[] =
    {         // Normal   Reflect
         0xFF // FF000000 000000FF
        ,0xFF // FFFF0000 0000FFFF
        ,0xFF // FFFFFF00 00FFFFFF
        ,0xFF // FFFFFFFF FFFFFFFF
    };

    trace( 1, buffer );
    trace( 2, buffer );
    trace( 3, buffer ); 
    trace( 4, buffer );

    trace( 9, CRC32_CHECK_TXT );

    return 0;
}

