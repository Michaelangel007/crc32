/*

"CRC32 Demystified"
https://github.com/Michaelangel007/crc32

Michaelangel007
Copyleft (C) 2017

History:
* reflect32() is SLOW, replaced with reverse32()
* renamed CRC32_FORWARD -> POLY_FORWARD
* renamed CRC32_REVERSE -> POLY_REVERSE
* renamed reverse -> REVERSE_BITS

*/

// Types

    // Pointer to Formulaic Function
    typedef uint32_t (*Crc32Simple_t)( const uint32_t POLY, size_t len, const void *data );

    // Pointer to Table Function
    typedef uint32_t (*Crc32Func_t)( const uint32_t *CRC32, size_t len, const void *data );

// Consts

    const uint32_t POLY_FORWARD = 0x04C11DB7; // forward = shift left
    const uint32_t POLY_REVERSE = 0xEDB88320; // reverse = shift right = reflect32( POLY_FORWARD );

/* */     uint32_t CRC32_FORWARD[256]; // init with poly = 0x04C11DB7
/* */     uint32_t CRC32_REVERSE[256]; // init with poly = 0xEDB88320, // [0]=0x00000000, [1]=0x77073096,

    const char    *CRC32_CHECK_TXT = "123456789";
    const uint32_t CRC32_CHECK_SUM =  0xCBF43926; // crc32_reverse( "123456789" ) = 0xCBF43926

// ------------------------------------------------------------------------
// Formulaic CRC32
// ------------------------------------------------------------------------

// uncommon CRC32A, poly = 0x04C11DB7, "123456789" checksum = 0xFC891918
    // ========================================================================
    uint32_t crc32a_formula_normal_noreverse( size_t len, const void *data, const uint32_t POLY = 0x04C11DB7 )
    {
        const unsigned char *buffer = (const unsigned char*) data;
        uint32_t crc = -1;

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

// common CRC32B, poly = 0x04C11DB7, "123456789", checksum = 0xCBF43926
    // ========================================================================
    uint32_t crc32_formula_normal( const uint32_t POLY, size_t len, const void *data )
    {
        const unsigned char *buffer = (const unsigned char*) data;
        uint32_t crc = -1;

        while( len-- )
        {
            crc = crc ^ (REVERSE_BITS[ *buffer++ ] << 24);
            for( int bit = 0; bit < 8; bit++ )
            {
                if( crc & (1L << 31)) crc = (crc << 1) ^ POLY;
                else                  crc = (crc << 1);
            }
        }
        return reverse32( ~crc );
    }

// common CRC32B, poly = 0xEDB88320, "123456789", checksum = 0xCBF43926
    // ========================================================================
    uint32_t crc32_formula_reflect( const uint32_t POLY, size_t len, const void *data )
    {
        const unsigned char *buffer = (const unsigned char*) data;
        uint32_t crc = -1;

        while( len-- )
        {
            crc = crc ^ *buffer++;
            for( int bit = 0; bit < 8; bit++ )
            {
                if( crc & 1 ) crc = (crc >> 1) ^ POLY;
                else          crc = (crc >> 1);
            }
        }
        return ~crc;
    }

// ------------------------------------------------------------------------
// Table-driven CRC32B
// ------------------------------------------------------------------------

// Table Initialization

    // Normal Polynimal = 0x04C11DB7
    // ========================================================================
    void crc32_init_normal( uint32_t *CRC32, const uint32_t POLY = 0x04C11DB7 )
    {
        for (unsigned int byte = 0; byte <= 0xFF; byte++ )
        {
            uint32_t crc = (byte << 24);

            for (int bit = 0; bit < 8; bit++ )
            {
                if (crc & (1L << 31)) crc = (crc << 1) ^ POLY;
                else                  crc = (crc << 1);
            }
            CRC32[ byte ] = crc;
        }
    }

    // Reflected Polynomial = 0xEDB88320
    // ========================================================================
    void crc32_init_reflect( uint32_t *CRC32, const uint32_t POLY = 0xEDB88320 )
    {
        for ( unsigned int byte = 0; byte <= 0xFF; byte++ )
        {
            uint32_t crc = byte;

            for( char bit = 0; bit < 8; bit++ )
            {
                if (crc & 1) crc = (crc >> 1) ^ POLY;
                else         crc = (crc >> 1);
            }
            CRC32[ byte ] = crc;
        }
    }


// Table CRC Calculation

    // Normal: crc << 8, Data Bits: *buffer, Final CRC: ~crc
    // ========================================================================
    uint32_t crc32_000( const uint32_t *CRC32, size_t len, const void *data )
    {
        const unsigned char *buffer = (const unsigned char*) data;
        uint32_t crc = -1;

        while( len-- )
            crc = CRC32[ (        *buffer++  ^ (crc >> 24)) & 0xFF ] ^ (crc << 8);
        return ~crc;
    }

    // Normal: crc << 8, Data Bits: *buffer, Final CRC: reverse32( ~crc )
    // ========================================================================
    uint32_t crc32_001( const uint32_t *CRC32, size_t len, const void *data )
    {
        const unsigned char *buffer = (const unsigned char*) data;
        uint32_t crc = -1;

        while( len-- )
            crc = CRC32[ (        *buffer++  ^ (crc >> 24)) & 0xFF ] ^ (crc << 8);
        return reverse32( ~crc );
    }

    // Normal: crc << 8, Data Bits: REVERSE_BITS[*buffer], Final CRC: ~crc
    // ========================================================================
    uint32_t crc32_010( const uint32_t *CRC32, size_t len, const void *data )
    {
        const unsigned char *buffer = (const unsigned char*) data;
        uint32_t crc = -1;

        while( len-- )
            crc = CRC32[ (REVERSE_BITS[*buffer++] ^ (crc >> 24)) & 0xFF ] ^ (crc << 8);
        return ~crc;
    }

// Authentic CRC Form: "Normal
    // Normal: crc << 8, Data Bits: REVERSE_BITS[*buffer], Final CRC: reverse32( ~crc )
    // ========================================================================
    uint32_t crc32_011( const uint32_t *CRC32, size_t len, const void *data )
    {
        const unsigned char *buffer = (const unsigned char*) data;
        uint32_t crc = -1;

        while( len-- )
            crc = CRC32[ (REVERSE_BITS[*buffer++] ^ (crc >> 24)) & 0xFF ] ^ (crc << 8);
        return reverse32( ~crc );
    }

// Authentic CRC Form: "Reflected" 

    // Reflected: crc >> 8, Data Bits: *buffer, Final CRC: ~crc
    // ========================================================================
    uint32_t crc32_100( const uint32_t *CRC32, size_t len, const void *data )
    {
        const unsigned char *buffer = (const unsigned char*) data;
        uint32_t crc = -1;

        while( len-- )
            crc = CRC32[ (crc ^         *buffer++ ) & 0xFF ] ^ (crc >> 8);
        return ~crc;
    }

    uint32_t crc32b_table_reflect( int nLength, const unsigned char* pData)
    {
         return crc32_100( CRC32_REVERSE, nLength, pData );
    }

    // Reflected: crc >> 8, Data Bits: REVERSE_BITS[*buffer], Final CRC: ~crc
    // ========================================================================
    uint32_t crc32_110( const uint32_t *CRC32, size_t len, const void *data )
    {
        const unsigned char *buffer = (const unsigned char*) data;
        uint32_t crc = -1;

        while( len-- )
            crc = CRC32[ (crc ^ REVERSE_BITS[*buffer++]) & 0xFF ] ^ (crc >> 8);
        return ~crc;
    }

    // Reflected: crc >> 8, Data Bits: *buffer, Final CRC: reverse32( ~crc )
    // ========================================================================
    uint32_t crc32_101( const uint32_t *CRC32, size_t len, const void *data )
    {
        const unsigned char *buffer = (const unsigned char*) data;
        uint32_t crc = -1;

        while( len-- )
            crc = CRC32[ (crc ^         *buffer++ ) & 0xFF ] ^ (crc >> 8);
        return reverse32( ~crc );
    }

    // Reflected: crc >> 8, Data Bits: REVERSE_BITS[*buffer], Final CRC: reverse32( ~crc )
    // ========================================================================
    uint32_t crc32_111( const uint32_t *CRC32, size_t len, const void *data )
    {
        const unsigned char *buffer = (const unsigned char*) data;
        uint32_t crc = -1;

        while( len-- )
            crc = CRC32[ (crc ^ REVERSE_BITS[*buffer++]) & 0xFF ] ^ (crc >> 8);
        return reverse32( ~crc );
    }

// ========================================================================
void CRC32_Init()
{
    // crc32b
    for( unsigned short byte = 0; byte < 256; byte++ )
    {
        uint32_t crc = (uint32_t) byte; // reflected form
        for( char bit = 0; bit < 8; bit++ )
            if( crc & 1 ) crc = (crc >> 1) ^ POLY_REVERSE; // reverse/reflected Form
            else          crc = (crc >> 1);
        CRC32_REVERSE[ byte ] = crc;
    }

    for( unsigned short byte = 0; byte < 256; byte++ )
    {
        uint32_t crc = (uint32_t) byte << 24;
        for( char bit = 0; bit < 8; bit++ )
            if( crc & (1L << 31)) crc = (crc << 1) ^ POLY_FORWARD; // forward Form
            else                  crc = (crc << 1);
        CRC32_FORWARD[ byte ] = crc;
    }

    if (CRC32_REVERSE[128] !=  POLY_REVERSE      ) printf( "ERROR: CRC#@ Reverse Table not initialzied properly! 0x%08X != 0x%08X\n", CRC32_REVERSE[128], POLY_REVERSE );
    if (CRC32_REVERSE  [8] != (POLY_REVERSE >> 4)) printf( "ERROR: CRC32 Reverse Table not initialized properly! 0x%08X != 0x%08X\n", CRC32_REVERSE  [8], POLY_REVERSE );
    if (CRC32_FORWARD  [1] !=  POLY_FORWARD      ) printf( "ERROR: CRC32 Forward Table not initialized properly! 0x%08x != 0x%08X\n", CRC32_FORWARD  [1], POLY_FORWARD );
    if (CRC32_FORWARD [16] != (POLY_FORWARD << 4)) printf( "ERROR: CRC32 Forward Table not initialized properly! 0x%08x != 0x%08X\n", CRC32_FORWARD [16], POLY_FORWARD );
}

// ========================================================================
unsigned int crc32_forward( size_t nLength, const unsigned char *pData )
{
    unsigned int crc = -1; // CRC32_INIT
    while( nLength --> 0 )
        crc = CRC32_FORWARD[ ((crc >> 24) ^ REVERSE_BITS[*pData++]) & 0xFF ] ^ (crc << 8); // normal form
    return reverse32(~crc); // ^CRC32_DONE
}

// ========================================================================
unsigned int crc32_reverse( size_t nLength, const unsigned char *pData )
{
    unsigned int crc = -1; // CRC32_INIT
    while( nLength --> 0 )
        crc = CRC32_REVERSE[ (crc         ^              *pData++ ) & 0xFF ] ^ (crc >> 8); // reverse/reflected form
    return ~crc; // ^CRC32_DONE
}
