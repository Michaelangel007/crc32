/*

"CRC32 Demystified"
https://github.com/Michaelangel007/crc32

Michaelangel007
Copyleft (C) 2017

*/

// Global
    // Before using must first call:
    //     reverse_init()
    // NOTE: If you're thrashing the L1 cache, change to uint8_t
    uint32_t reverse[ 256 ]; // 8-bit reverse bit look-up table

// Utility

/**
    Formulaic Reverse 32 bits

    In:

        +--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+
        |31:30:29:28:27:26:25:24:23:22:21:20:19:18:17:16:15:14:13:12:11:10: 9: 8: 7: 6: 5: 4: 3: 2: 1: 0|
        +--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+

    Out:

        +--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+
        | 0: 1: 2: 3: 4: 5: 6: 7: 8: 9:10:11:12:13:14:15:16:17:18:19:20:21:22:23:24:25:26:27:28:29:30:31|
        +--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+--+

   @param  {uint32_t} x - value to bit-reverse
   @return {uint32_t}   - value bit reversed
*/
    // ========================================================================
    uint32_t reflect32( const uint32_t x )
    {
        uint32_t bits = 0;
        uint32_t mask = x;

        for( int i = 0; i < sizeof(x)*8; i++ )
        {
            bits <<= 1;
            if( mask & 1 )
                bits |= 1;
            mask >>= 1;
        }

        return bits;
    }

    /** Table-Lookup
     * @param  {uint32_t} x - value to bit-reverse
     * @return {uint32_t}     value bit reversed
     */
    // ========================================================================
    uint32_t reverse32( const uint32_t x )
    {
        return 0
        | reverse[ (x >> 24) & 0xFF ] <<  0L
        | reverse[ (x >> 16) & 0xFF ] <<  8L
        | reverse[ (x >>  8) & 0xFF ] << 16L
        | reverse[ (x >>  0) & 0xFF ] << 24L;
    }

    // Table-Lookup
    // ========================================================================
    void reverse_init()
    {
        for( int byte = 0; byte < 256; byte++ )
            reverse[ byte ] = (reflect32( byte ) >> 24) & 0xFF;

//      for( int byte = 0; byte < 256; byte++ )
//          printf( "Byte: %02X -> %08X -> %02X\n", byte, reflect32( byte ), (reflect32( byte ) >> 24) & 0xFF );
    }

