/*

"CRC32 Demystified"
https://github.com/Michaelangel007/crc32

Michaelangel007
Copyleft (C) 2017

Also see:
http://stackoverflow.com/questions/26049150/calculate-a-32-bit-crc-lookup-table-in-c-c

*/

// Includes
    #include "common.h"

// ========================================================================
int main()
{
    uint32_t      crc;
    uint32_t      check  = CRC32_CHECK_SUM;
    const char   *text   = CRC32_CHECK_TXT;
    const size_t  length = strlen( text );
    uint32_t      CRC32A = 0xFC891918;

    common_init();

    printf( "String: '%s' (Input:%d)\n", text, (int)length );
    printf( "CRC = 0x%08X (Expected)\n", check );
    printf( "\n" );

    printf( "Enumerating CRC polynomials and functions ...\n" );
    printf( "Find which permutation(s) equal to standard CRC32 ...\n" );

    printf( "\n1. Formulaic CRC calculations... (Actual)\n" );

    for( int i = 0; i < nDesc; i++ )
        printf( "   %s( 0x%08X ): 0x%08X %s\n", aType[i], aPoly[i], aSimple[i]( aPoly[i], length, text ), aStatus[i] );

    printf( "\n2. Table-driven CRC calculations... (Actual)\n" );
    for( int iDesc = 0; iDesc < nDesc; iDesc++ )
    {
        const uint32_t *pData = aData[ iDesc ];
        const char     *pDesc = aDesc[ iDesc ];

        printf( "%s\n", pDesc );

        for( int iFunc = 0; iFunc < nFunc; iFunc++ )
        {
            int isShiftR  = (iFunc >> 2) & 1;
            int isRevData = (iFunc >> 1) & 1;
            int isRevCRC  = (iFunc >> 0) & 1;

            crc = aFunc[ iFunc ]( pData, length, text ); 
            printf( "   Shift: %s, Rev. Data: %d, Rev. CRC: %d, 0x%08X  %s\n"
                , isShiftR
                  ? "Right"
                  : "Left "
                , isRevData
                , isRevCRC
                , crc
                , crc == check
                    ? "crc32b"
                    : crc == CRC32A
                        ? "crc32a"
                        : "no"
            );
        }

        printf( "\n" );
    }

    return 0;
}

