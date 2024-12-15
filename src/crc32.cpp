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

// Consts
    const unsigned int  CRC32_REVERSE   = 0xEDB88320; // reflect32( CRC32_FORWARD );
    const          char*CRC32_CHECK_TXT ="123456789";
    const unsigned int  CRC32_CHECK_SUM = 0xCBF43926; // "123456789" -> 0xCBF43926
    /* */ unsigned int  CRC32_TABLE[256]            ; // i.e. 0x00000000, 0x77073096, 

// ========================================================================
unsigned int crc32a_formula_normal_noreverse( size_t len, const void *data, const unsigned int POLY = 0x04C11DB7 )
{
    const unsigned char *buffer = (const unsigned char*) data;
    unsigned int crc = -1;

    while( len-- )
    {
        crc = crc ^ (*buffer++ << 24);
        for( int bit = 0; bit < 8; bit++ )
        {
            if( crc & (1L << 31)) crc = (crc << 1) ^ POLY;
            else                  crc = (crc << 1);
        }
    }
    return ~crc;
}

// ========================================================================
void crc32b_init()
{
    for( int byte = 0; byte < 256; byte++ )
    {
        unsigned int crc = (unsigned int) byte; // reflected form

        for( char bit = 0; bit < 8; bit++ )
            if( crc & 1 ) crc = (crc >> 1) ^ CRC32_REVERSE; // reflected Form
            else          crc = (crc >> 1);

        CRC32_TABLE[ byte ] = crc;
    }

    if( CRC32_TABLE[128] != CRC32_REVERSE )
        printf("ERROR: CRC32 Table not initialized properly!\n");
}

// ========================================================================
unsigned int crc32b_table_reflect( int nLength, const unsigned char *pData )
{
   unsigned int crc = -1 ; // Optimization: crc = CRC32_INIT;

   while( nLength-- > 0 )
       crc = CRC32_TABLE[ (crc ^ *pData++) & 0xFF ] ^ (crc >> 8); // reflect: >> 8

   return ~crc; // Optimization: crc ^= CRC32_DONE
} 

// ========================================================================
int main( const int nArg, const char *aArg[] )
{
    crc32b_init();

    const char *pArg = (nArg > 1)
        ? &aArg[1][0]
        : CRC32_CHECK_TXT
        ;
    const int nLen = (int) strlen( pArg );
    const unsigned char *pData = (const unsigned char*) pArg;

    printf( "CRC32A = 0x%08X             \n", crc32a_formula_normal_noreverse( nLen, pData ) );
    printf( "CRC32B = 0x%08X; // '%s' (%d)\n", crc32b_table_reflect( nLen, pData ), pArg, nLen );
    return 0;
}
