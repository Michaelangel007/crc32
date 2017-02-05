/*

"CRC32 Demystified"
https://github.com/Michaelangel007/crc32

Michaelangel007
Copyleft (C) 2017

*/

// Types

    // Pointer to Formulaic Function
    typedef uint32_t (*Crc32Simple_t)( const uint32_t POLY, size_t len, const void *data );

    // Pointer to Table Function
    typedef uint32_t (*Crc32Func_t)( const uint32_t *CRC32, size_t len, const void *data );

// Consts

    const uint32_t CRC32_FORWARD = 0x04C11DB7;
    const uint32_t CRC32_REVERSE = 0xEDB88320;

    const char    *CRC32_CHECK_TXT = "123456789";
    const uint32_t CRC32_CHECK_SUM =  0xCBF43926; // data = "123456789", crc32 = 0xCBF43926

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
            crc = crc ^ (reverse[ *buffer++ ] << 24);
            for( int bit = 0; bit < 8; bit++ )
            {
                if( crc & (1L << 31)) crc = (crc << 1) ^ POLY;
                else                  crc = (crc << 1);
            }
        }
        return reflect32( ~crc );
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

    // Normal: crc << 8, Data Bits: *buffer, Final CRC: reflect32( ~crc )
    // ========================================================================
    uint32_t crc32_001( const uint32_t *CRC32, size_t len, const void *data )
    {
        const unsigned char *buffer = (const unsigned char*) data;
        uint32_t crc = -1;

        while( len-- )
            crc = CRC32[ (        *buffer++  ^ (crc >> 24)) & 0xFF ] ^ (crc << 8);
        return reflect32( ~crc );
    }

    // Normal: crc << 8, Data Bits: reverse[*buffer], Final CRC: ~crc
    // ========================================================================
    uint32_t crc32_010( const uint32_t *CRC32, size_t len, const void *data )
    {
        const unsigned char *buffer = (const unsigned char*) data;
        uint32_t crc = -1;

        while( len-- )
            crc = CRC32[ (reverse[*buffer++] ^ (crc >> 24)) & 0xFF ] ^ (crc << 8);
        return ~crc;
    }

// Authentic CRC Form: "Normal
    // Normal: crc << 8, Data Bits: reverse[*buffer], Final CRC: reflect32( ~crc )
    // ========================================================================
    uint32_t crc32_011( const uint32_t *CRC32, size_t len, const void *data )
    {
        const unsigned char *buffer = (const unsigned char*) data;
        uint32_t crc = -1;

        while( len-- )
            crc = CRC32[ (reverse[*buffer++] ^ (crc >> 24)) & 0xFF ] ^ (crc << 8);
        return reflect32( ~crc );
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

    // Reflected: crc >> 8, Data Bits: reverse[*buffer], Final CRC: ~crc
    // ========================================================================
    uint32_t crc32_110( const uint32_t *CRC32, size_t len, const void *data )
    {
        const unsigned char *buffer = (const unsigned char*) data;
        uint32_t crc = -1;

        while( len-- )
            crc = CRC32[ (crc ^ reverse[*buffer++]) & 0xFF ] ^ (crc >> 8);
        return ~crc;
    }

    // Reflected: crc >> 8, Data Bits: *buffer, Final CRC: reflect32( ~crc )
    // ========================================================================
    uint32_t crc32_101( const uint32_t *CRC32, size_t len, const void *data )
    {
        const unsigned char *buffer = (const unsigned char*) data;
        uint32_t crc = -1;

        while( len-- )
            crc = CRC32[ (crc ^         *buffer++ ) & 0xFF ] ^ (crc >> 8);
        return reflect32( ~crc );
    }

    // Reflected: crc >> 8, Data Bits: reverse[*buffer], Final CRC: reflect32( ~crc )
    // ========================================================================
    uint32_t crc32_111( const uint32_t *CRC32, size_t len, const void *data )
    {
        const unsigned char *buffer = (const unsigned char*) data;
        uint32_t crc = -1;

        while( len-- )
            crc = CRC32[ (crc ^ reverse[*buffer++]) & 0xFF ] ^ (crc >> 8);
        return reflect32( ~crc );
    }

