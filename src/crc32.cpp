/*

"CRC32 Demystified"
https://github.com/Michaelangel007/crc32

Michaelangel007
Copyleft (C) 2017

Compile:

    g++ crc32.cpp -o crc32

Usage:

    crc32 [text]

    text defaults to "123456789"
*/

// Includes
    #include <stdio.h>
    #include <string.h>

    #include "common.cpp"

// ========================================================================
int main( const int nArg, const char *aArg[] )
{
    common_init();

    const char *pArg = (nArg > 1)
        ? &aArg[1][0]
        : CRC32_CHECK_TXT
        ;
    const int nLen = (int) strlen( pArg );
    const unsigned char *pData = (const unsigned char*) pArg;

    printf( "CRC32A = 0x%08X              \n", crc32a_formula_normal_noreverse( nLen, pData ) );
    printf( "CRC32B = 0x%08X; // '%s' (%d)\n", crc32b_table_reflect( nLen, pData ), pArg, nLen );
    printf( "forward = 0x%08X             \n", crc32_forward( nLen, pData ) );
    printf( "reverse = 0x%08X             \n", crc32_reverse( nLen, pData ) );

    return 0;
}
