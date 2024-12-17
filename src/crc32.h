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
    typedef uint32_t (*Crc32Table_t)( const uint32_t *CRC32, size_t len, const void *data );

    typedef uint32_t (*Crc32Func)( size_t length, const unsigned char *data);

// Consts

    const uint32_t POLY_FORWARD = 0x04C11DB7; // forward = shift left
    const uint32_t POLY_REVERSE = 0xEDB88320; // reverse = shift right = reflect32( POLY_FORWARD );

    // Popular ones include:
    //   0x04C11DB7 CRC32B "Ethernet"
    //   0x1EDC6F41 CRC32C "Optimization of cyclic redundancy-check codes with 24 and 32 parity bits"
    //   0x741B8CD7 CRC32K
    //   0x814141AB CRC32Q
    //   0x82608EDB
    //   0x82F63B78 CRC32C Intel slicing, reversed 0x1EDC6F41
    //   0xEDB88320
    //   0xDB710641

/* */     uint32_t CRC32_FORWARD[256]; // init with poly = 0x04C11DB7
/* */     uint32_t CRC32_REVERSE[256]; // init with poly = 0xEDB88320, // [0]=0x00000000, [1]=0x77073096,

    const uint32_t CRC32C_POLY_REVERSE = 0x82F63B78;
    const uint32_t CRC32C_REVERSED[256] =
    {
        0x00000000, 0xF26B8303, 0xE13B70F7, 0x1350F3F4, 0xC79A971F, 0x35F1141C, 0x26A1E7E8, 0xD4CA64EB, // [00]
        0x8AD958CF, 0x78B2DBCC, 0x6BE22838, 0x9989AB3B, 0x4D43CFD0, 0xBF284CD3, 0xAC78BF27, 0x5E133C24, // [08]
        0x105EC76F, 0xE235446C, 0xF165B798, 0x030E349B, 0xD7C45070, 0x25AFD373, 0x36FF2087, 0xC494A384, // [10]
        0x9A879FA0, 0x68EC1CA3, 0x7BBCEF57, 0x89D76C54, 0x5D1D08BF, 0xAF768BBC, 0xBC267848, 0x4E4DFB4B, // [18]
        0x20BD8EDE, 0xD2D60DDD, 0xC186FE29, 0x33ED7D2A, 0xE72719C1, 0x154C9AC2, 0x061C6936, 0xF477EA35, // [20]
        0xAA64D611, 0x580F5512, 0x4B5FA6E6, 0xB93425E5, 0x6DFE410E, 0x9F95C20D, 0x8CC531F9, 0x7EAEB2FA, // [28]
        0x30E349B1, 0xC288CAB2, 0xD1D83946, 0x23B3BA45, 0xF779DEAE, 0x05125DAD, 0x1642AE59, 0xE4292D5A, // [30]
        0xBA3A117E, 0x4851927D, 0x5B016189, 0xA96AE28A, 0x7DA08661, 0x8FCB0562, 0x9C9BF696, 0x6EF07595, // [38]
        0x417B1DBC, 0xB3109EBF, 0xA0406D4B, 0x522BEE48, 0x86E18AA3, 0x748A09A0, 0x67DAFA54, 0x95B17957, // [40]
        0xCBA24573, 0x39C9C670, 0x2A993584, 0xD8F2B687, 0x0C38D26C, 0xFE53516F, 0xED03A29B, 0x1F682198, // [48]
        0x5125DAD3, 0xA34E59D0, 0xB01EAA24, 0x42752927, 0x96BF4DCC, 0x64D4CECF, 0x77843D3B, 0x85EFBE38, // [50]
        0xDBFC821C, 0x2997011F, 0x3AC7F2EB, 0xC8AC71E8, 0x1C661503, 0xEE0D9600, 0xFD5D65F4, 0x0F36E6F7, // [58]
        0x61C69362, 0x93AD1061, 0x80FDE395, 0x72966096, 0xA65C047D, 0x5437877E, 0x4767748A, 0xB50CF789, // [60]
        0xEB1FCBAD, 0x197448AE, 0x0A24BB5A, 0xF84F3859, 0x2C855CB2, 0xDEEEDFB1, 0xCDBE2C45, 0x3FD5AF46, // [68]
        0x7198540D, 0x83F3D70E, 0x90A324FA, 0x62C8A7F9, 0xB602C312, 0x44694011, 0x5739B3E5, 0xA55230E6, // [70]
        0xFB410CC2, 0x092A8FC1, 0x1A7A7C35, 0xE811FF36, 0x3CDB9BDD, 0xCEB018DE, 0xDDE0EB2A, 0x2F8B6829, // [78]
        0x82F63B78, 0x709DB87B, 0x63CD4B8F, 0x91A6C88C, 0x456CAC67, 0xB7072F64, 0xA457DC90, 0x563C5F93, // [80] Polynomial used
        0x082F63B7, 0xFA44E0B4, 0xE9141340, 0x1B7F9043, 0xCFB5F4A8, 0x3DDE77AB, 0x2E8E845F, 0xDCE5075C, // [88]
        0x92A8FC17, 0x60C37F14, 0x73938CE0, 0x81F80FE3, 0x55326B08, 0xA759E80B, 0xB4091BFF, 0x466298FC, // [90]
        0x1871A4D8, 0xEA1A27DB, 0xF94AD42F, 0x0B21572C, 0xDFEB33C7, 0x2D80B0C4, 0x3ED04330, 0xCCBBC033, // [98]
        0xA24BB5A6, 0x502036A5, 0x4370C551, 0xB11B4652, 0x65D122B9, 0x97BAA1BA, 0x84EA524E, 0x7681D14D, // [A0]
        0x2892ED69, 0xDAF96E6A, 0xC9A99D9E, 0x3BC21E9D, 0xEF087A76, 0x1D63F975, 0x0E330A81, 0xFC588982, // [A8]
        0xB21572C9, 0x407EF1CA, 0x532E023E, 0xA145813D, 0x758FE5D6, 0x87E466D5, 0x94B49521, 0x66DF1622, // [B0]
        0x38CC2A06, 0xCAA7A905, 0xD9F75AF1, 0x2B9CD9F2, 0xFF56BD19, 0x0D3D3E1A, 0x1E6DCDEE, 0xEC064EED, // [B8]
        0xC38D26C4, 0x31E6A5C7, 0x22B65633, 0xD0DDD530, 0x0417B1DB, 0xF67C32D8, 0xE52CC12C, 0x1747422F, // [C0]
        0x49547E0B, 0xBB3FFD08, 0xA86F0EFC, 0x5A048DFF, 0x8ECEE914, 0x7CA56A17, 0x6FF599E3, 0x9D9E1AE0, // [C8]
        0xD3D3E1AB, 0x21B862A8, 0x32E8915C, 0xC083125F, 0x144976B4, 0xE622F5B7, 0xF5720643, 0x07198540, // [D0]
        0x590AB964, 0xAB613A67, 0xB831C993, 0x4A5A4A90, 0x9E902E7B, 0x6CFBAD78, 0x7FAB5E8C, 0x8DC0DD8F, // [D8]
        0xE330A81A, 0x115B2B19, 0x020BD8ED, 0xF0605BEE, 0x24AA3F05, 0xD6C1BC06, 0xC5914FF2, 0x37FACCF1, // [E0]
        0x69E9F0D5, 0x9B8273D6, 0x88D28022, 0x7AB90321, 0xAE7367CA, 0x5C18E4C9, 0x4F48173D, 0xBD23943E, // [E8]
        0xF36E6F75, 0x0105EC76, 0x12551F82, 0xE03E9C81, 0x34F4F86A, 0xC69F7B69, 0xD5CF889D, 0x27A40B9E, // [F0]
        0x79B737BA, 0x8BDCB4B9, 0x988C474D, 0x6AE7C44E, 0xBE2DA0A5, 0x4C4623A6, 0x5F16D052, 0xAD7D5351  // [F8]
    };

    const char    *CRC32_CHECK_TXT = "123456789";
    const uint32_t CRC32_CHECK_SUM =  0xCBF43926; // crc32_reverse( "123456789" ) = 0xCBF43926

    const char    *CRC32C_CHECK_TXT = "hello world";
    const uint32_t CRC32C_CHECK_SUM = 0xC99465AA;

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

// ========================================================================
uint32_t crc32c_reverse( size_t nLength, const unsigned char *pData )
{
    unsigned int crc = ~0UL;
    while( nLength --> 0 )
        crc = CRC32C_REVERSED[ (crc ^ *pData++ ) & 0xFF ] ^ (crc >> 8); // reverse/reflected form
    return ~crc;
}

