/*

"CRC32 Demystified"
https://github.com/Michaelangel007/crc32

Michaelangel007
Copyleft (C) 2017

Brute-force searching for bytes of length N for when CRC32 generates zero.

*/

// Defines
    #define USE_OMP 1
#if _WIN32
    #define _CRT_SECURE_NO_WARNINGS 1
#endif

// Includes
    #include "common.cpp"

// BEGIN OMP
#if USE_OMP
    #include <omp.h>
#endif
    #include "util_threads.h"
    #include "util_timer.h"
// END OMP

// Macros
    // NEEDS variables: LENGTH, RANGE, total
    #define HEADER   printf( "// Searching length %d...\n", LENGTH )
    #define FOOTER   printf( "// Found %d of length %d in ", found, LENGTH ); return RANGE

// Vars
    Crc32Func gpCRC32   = crc32_reverse;
    bool bSearchCRC32B  = true;
    char cSearchCRC32   = 'b';
    bool bShowProgress  = false;

typedef size_t (*FuncPtr)();
void Measure( FuncPtr pSearchLenFunc )
{
    Timer timer;
    timer.Start();
        pSearchLenFunc();
    timer.Stop();
    timer.Print();
    printf( "\n" );
}

// ========================================================================
void Hexdump(uint64_t data, const size_t length, char* text )
{
    if (!text) return;

    for( int offset = 0; offset < length; offset++ )
        sprintf( &text[offset*3], "%02llX ", (data >> (8 * offset)) & 0xFF );
}

// v1
void Printable(unsigned char* data, const size_t length, char* text)
{
    if (!data) return;
    if (!text) return;

    for( int offset = 0; offset < length; offset++ )
             if (data[ offset ] <= 0x20) text[offset] = '.';
        else if (data[ offset ] >= 0x7F) text[offset] = '?';
        else                             text[offset] = data[offset];

    text[length] = 0;
}

// v2
inline void Printable2(const size_t length, uint64_t crc, char* text)
{
    for( int offset = 0; offset < length; offset++ )
    {
        uint8_t byte = crc & 0xFF;
             if (byte <= 0x20) text[offset] = '.';
        else if (byte >= 0x7F) text[offset] = '?';
        else                   text[offset] = byte;
        crc >>= 8;
    }
    text[length] = 0;
}


// "................"
// "????????????????"
// "真真真真真真真真"
// "\xA8\xA8\xA8\xA8\xA8\xA8\xA8\xA8\xA8\xA8\xA8\xA8\xA8\xA8\xA8\xA8\xA8"
const char BYTE_TO_ASCII[] =
  // 0123456789ABCDEF
    "................" // [00]
    "................" // [10]
    " !\"#$%&'()*+,-./"// [20]
    "0123456789:;<=>?" // [30]
    "@ABCDEFGHIJKLMNO" // [40]
    "PQRSTUVWXYZ[\\]^_"// [50]
    "`abcdefghijklmno" // [60]
    "pqrstuvwxyz{|}~?" // [70]
    "????????????????" // [80]
    "????????????????" // [90]
    "????????????????" // [A0]
    "????????????????" // [B0]
    "????????????????" // [C0]
    "????????????????" // [D0]
    "????????????????" // [E0]
    "????????????????" // [F0]
;
// ========================================================================
inline void Printable3(const size_t length, uint64_t crc, char* text)
{
    for( int offset = 0; offset < length; crc >>= 8, offset++ )
        text[offset] = BYTE_TO_ASCII[ crc & 0xFF ];
}

// ========================================================================
size_t SearchLen1()
{
          size_t progress   = 0;
          double percent    = 0.0;
          int    found      = 0;

    const int    LENGTH     = 1;
    const int    BITS_TOTAL = (8 * LENGTH);
    const int    BITS_WORDS = (BITS_TOTAL < 32) ? BITS_TOTAL : 32;
    const int    BITS_PAGES = BITS_TOTAL - BITS_WORDS;

    const size_t RANGE      = 1ull << BITS_TOTAL;
    const size_t WORDS      = 1ull << BITS_WORDS;
    const size_t PAGES      = 1ull << BITS_PAGES;
    HEADER;

    for( int iPage = 0; iPage < PAGES; iPage++ )
    {
        const int      iThread = 0;
              char     keytext[8] = { 0, 0, 0, 0, 0, 0, 0, 0 };
              uint64_t Haystack   = ((size_t)iPage) << BITS_WORDS;
              uint64_t iWord      = WORDS;
              uint32_t crc;

        while (iWord --> 0)
        {
            unsigned char *pNeedle  = (unsigned char*) &Haystack; // Printable v1
            crc = gpCRC32( LENGTH, pNeedle );
            if (crc == 0)
            {
                Printable3( LENGTH, Haystack, keytext );
                printf( ", 0x%0*llX // %s  [#%02d, Page: %0*X] %6.2f%%\n", LENGTH*2, Haystack, keytext, iThread, (BITS_PAGES+3)/4, iPage, percent );
                found++;
            }

            Haystack++;
        }
        progress++;
        percent = (100.0 * progress) / (double)PAGES;
    }

    FOOTER;
}

// ========================================================================
size_t SearchLen2()
{
          size_t progress   = 0;
          double percent    = 0.0;
          int    found      = 0;

    const int    LENGTH     = 2;
    const int    BITS_TOTAL = (8 * LENGTH);
    const int    BITS_WORDS = (BITS_TOTAL < 32) ? BITS_TOTAL : 32;
    const int    BITS_PAGES = BITS_TOTAL - BITS_WORDS;

    const size_t RANGE      = 1ull << BITS_TOTAL; // 2 bytes = 0x10000
    const size_t WORDS      = 1ull << BITS_WORDS;
    const size_t PAGES      = 1ull << BITS_PAGES;
    HEADER;

    for (int bytes2 = 0; bytes2 < RANGE; bytes2++)
    {
        unsigned char data2[2];
        *((uint16_t *)data2) = bytes2;
        uint32_t crc = gpCRC32( LENGTH, data2 );
        if (crc == 0)
        {
            printf( ", 0x%08X\n", bytes2 );
            found++;
        }
    }

    FOOTER;
}

// ========================================================================
size_t SearchLen3()
{
          size_t progress   = 0;
          double percent    = 0.0;
          int    found      = 0;

    const int    LENGTH     = 3;
    const int    BITS_TOTAL = (8 * LENGTH);
    const int    BITS_WORDS = (BITS_TOTAL < 32) ? BITS_TOTAL : 32;
    const int    BITS_PAGES = BITS_TOTAL - BITS_WORDS;

    const size_t RANGE      = 1ull << BITS_TOTAL; // 3 bytes = 0x1000000
    const size_t WORDS      = 1ull << BITS_WORDS;
    const size_t PAGES      = 1ull << BITS_PAGES;
    HEADER;

    for (int bytes3 = 0; bytes3 < RANGE; bytes3++)
    {
        unsigned char data3[LENGTH+1];
        *((uint32_t *)data3) = bytes3;

        uint32_t crc = gpCRC32( LENGTH, data3 );
        if (crc == 0)
        {
            printf( ", 0x%08X\n", bytes3 );
            found++;
        }
    }

    FOOTER;
}

// ========================================================================
size_t SearchLen4()
{
          size_t progress   = 0;
          double percent    = 0.0;
          int    found      = 0;

    const int    LENGTH     = 4;
    const int    BITS_TOTAL = (8 * LENGTH);
    const int    BITS_WORDS = 24; // (BITS_TOTAL < 32) ? BITS_TOTAL : 32;
    const int    BITS_PAGES = BITS_TOTAL - BITS_WORDS;

    const size_t RANGE      = 1ull << BITS_TOTAL;
    const size_t WORDS      = 1ull << BITS_WORDS; // 0x1000000; // 3 bytes
    const size_t PAGES      = 1ull << BITS_PAGES; // 0x100
    HEADER;

#if _DEBUG
    printf( "// LENGTH: %d\n", LENGTH );
    printf( "// RANGE : %016llX  (%d bits)\n", RANGE, BITS_TOTAL );
    printf( "// PAGES :       %010llX  (%d bits)\n", PAGES, BITS_PAGES );
    printf( "// WORDS :       %010llX  (%d bits)\n", WORDS, BITS_WORDS );
#endif

#pragma omp parallel for
    for( int iPage = 0; iPage < PAGES; iPage++ )
    {
#if USE_OMP
        const int      iThread = omp_get_thread_num(); // Get Thread Index: 0 .. nCores-1
#else
        const int      iThread = 0;
#endif
              char     keytext[8] = { 0, 0, 0, 0, 0, 0, 0, 0 };
              uint32_t Haystack   = ((uint32_t)iPage    ) << BITS_WORDS;
              uint32_t Remain     = WORDS;

        while( Remain --> 0 )
        {
            // https://stackoverflow.com/questions/25573743/can-crc32c-ever-return-to-0
            // 4-byte: ab 9b e0 9b
            // 5-byte: DYB|O
            // Which poly is this???  B69B50B9

            // 6DD90A9D
            unsigned char *pNeedle  = (unsigned char*) &Haystack;
            uint32_t crc = gpCRC32( LENGTH, pNeedle );
            if (crc == 0)
            {
                //Printable( pNeedle, LENGTH, keytext );
                Printable3( LENGTH, Haystack, keytext );
                printf( ", 0x%08X // %s  [#%02d, Page: %02X] %6.2f%%\n", Haystack, keytext, iThread, iPage, percent );
#pragma omp atomic
                found++;
            }
            Haystack++;
        }
#pragma omp atomic
        progress++;
        percent = (100.0 * progress) / (double)PAGES;
    }

    FOOTER;
}

// ========================================================================
size_t SearchLen5()
{
          size_t progress   = 0;
          double percent    = 0.0;
          int    found      = 0;

    const int    LENGTH     = 5;
    const int    BITS_TOTAL = (8 * LENGTH);
    const int    BITS_WORDS = (BITS_TOTAL < 32) ? BITS_TOTAL : 32; // MIN( 32, BITS_TOTAL );
    const int    BITS_PAGES = BITS_TOTAL - BITS_WORDS;

    const size_t RANGE      = 1ull << BITS_TOTAL;
    const size_t WORDS      = 1ull << BITS_WORDS;
    const size_t PAGES      = 1ull << BITS_PAGES;
    HEADER;

#if _DEBUG
    printf( "// LENGTH: %d\n", LENGTH );
    printf( "// RANGE : %016llX  (%d bits)\n", RANGE, BITS_TOTAL );
    printf( "// PAGES :       %010llX  (%d bits)\n", PAGES, BITS_PAGES );
    printf( "// WORDS :       %010llX  (%d bits)\n", WORDS, BITS_WORDS );
#endif

#pragma omp parallel for
    for( int iPage = 0; iPage < PAGES; iPage++ )
    {
#if USE_OMP
        const int      iThread = omp_get_thread_num(); // Get Thread Index: 0 .. nCores-1
#else
        const int      iThread = 0;
#endif
              char     keytext[8] = { 0, 0, 0, 0, 0, 0, 0, 0 };
              uint64_t Haystack   = ((uint64_t)iPage) << BITS_WORDS;
              uint64_t Remain     = WORDS;

        if (bSearchCRC32B)
        {
            while (Remain --> 0)
            {
                unsigned char *pNeedle  = (unsigned char*) &Haystack; // Printable v1
                uint32_t crc = crc32_reverse( LENGTH, pNeedle );
                if (crc == 0)
                {
                    Printable3( LENGTH, Haystack, keytext );
                    printf( ", 0x%010llX // %s  [#%02d, Page: %02X] %6.2f%%\n", Haystack, keytext, iThread, iPage, percent );
    #pragma omp atomic
                    found++;
                }
                Haystack++;
            }
        }
        else
        {
            while (Remain --> 0)
            {
                unsigned char *pNeedle  = (unsigned char*) &Haystack; // Printable v1
                uint32_t crc = crc32c_reverse( LENGTH, pNeedle );
                if (crc == 0)
                {
                    Printable3( LENGTH, Haystack, keytext );
                    printf( ", 0x%010llX // %s  [#%02d, Page: %02X] %6.2f%%\n", Haystack, keytext, iThread, iPage, percent );
    #pragma omp atomic
                    found++;
                }
                Haystack++;
            }
        }

#pragma omp atomic
        progress++;
        percent = (100.0 * progress) / (double)PAGES;
    }

    FOOTER;
}

// ========================================================================
size_t SearchLen6()
{
          size_t progress   = 0;
          double percent    = 0.0;
          int    found      = 0;

    const int    LENGTH     = 6;
    const int    BITS_TOTAL = (8 * LENGTH);
    const int    BITS_WORDS = (BITS_TOTAL < 32) ? BITS_TOTAL : 32; // MIN( 32, BITS_TOTAL );
    const int    BITS_PAGES = BITS_TOTAL - BITS_WORDS;

    const size_t RANGE      = 1ull << BITS_TOTAL;
    const size_t WORDS      = 1ull << BITS_WORDS;
    const size_t PAGES      = 1ull << BITS_PAGES;
    HEADER;

#if _DEBUG
    printf( "// LENGTH: %d\n", LENGTH );
    printf( "// RANGE : %016llX  (%d bits)\n", RANGE, BITS_TOTAL );
    printf( "// PAGES :       %010llX  (%d bits)\n", PAGES, BITS_PAGES );
    printf( "// WORDS :       %010llX  (%d bits)\n", WORDS, BITS_WORDS );
#endif

#pragma omp parallel for
    for( int iPage = 0; iPage < PAGES; iPage++ )
    {
#if USE_OMP
        const int      iThread = omp_get_thread_num(); // Get Thread Index: 0 .. nCores-1
#else
        const int      iThread = 0;
#endif
              char     keytext[8] = { 0, 0, 0, 0, 0, 0, 0, 0 };
              uint64_t Haystack   = ((size_t)iPage) << BITS_WORDS;
              uint64_t iWord      = WORDS;
              unsigned char *pNeedle  = (unsigned char*) &Haystack;

        if (bSearchCRC32B)
        {
            while (iWord --> 0)
            {
                uint32_t crc = crc32_reverse( LENGTH, pNeedle );
                if (crc == 0)
                {
                    Printable3( LENGTH, Haystack, keytext );
                    printf( ", 0x%0*llX // %s  [#%02d, Page: %0*X] %6.2f%%\n", LENGTH*2, Haystack, keytext, iThread, (BITS_PAGES+3)/4, iPage, percent );
    #pragma omp atomic
                    found++;
                }
                Haystack++;
            }
        }
        else
        {
            while (iWord --> 0)
            {
                uint32_t crc = crc32c_reverse( LENGTH, pNeedle );
                if (crc == 0)
                {
                    Printable3( LENGTH, Haystack, keytext );
                    printf( ", 0x%0*llX // %s  [#%02d, Page: %0*X] %6.2f%%\n", LENGTH*2, Haystack, keytext, iThread, (BITS_PAGES+3)/4, iPage, percent );
    #pragma omp atomic
                    found++;
                }
                Haystack++;
            }
        }

#pragma omp atomic
        progress++;
        percent = (100.0 * progress) / (double)PAGES;
    }

    FOOTER;
}

    //      2^(8*Len)                        --Zeroes--
    // Len  Bits          Page        Words  Page Words
    //   1  2^ 8           0x1        0x100  0    2
    //   2  2^16           0x1      0x10000  0    4
    //   3  2^24           0x1    0x1000000  0    6
    //   4  2^32           0x1  0x100000000  0    8 <-- single threaded
    //   4  2^32         0x100    0x1000000  2    6 <-- force multi-threading
    //   5  2^40         0x100  0x100000000  2    8
    //   6  2^48       0x10000  0x100000000  4    8
    //   7  2^56     0x1000000  0x100000000  6    8
    //   8  2^64   0x100000000  0x100000000  8    8
// ========================================================================
size_t SearchLenN(const int LENGTH)
{
          size_t progress   = 0;
          double percent    = 0.0;
          int    found      = 0;

    const int    BITS_TOTAL = (8 * LENGTH);
    const int    BITS_WORDS = (BITS_TOTAL < 32) ? BITS_TOTAL : 32; // MIN( 32, BITS_TOTAL );
    const int    BITS_PAGES = BITS_TOTAL - BITS_WORDS;

    const size_t RANGE      = 1ull << BITS_TOTAL;
    const size_t WORDS      = 1ull << BITS_WORDS;
    const size_t PAGES      = 1ull << BITS_PAGES;
    HEADER;

    printf( "// CRC32%c\n", cSearchCRC32 );
    printf( "// LENGTH: %d\n", LENGTH );
    printf( "// RANGE : %016llX  (%d bits)\n", RANGE, BITS_TOTAL );
    printf( "// PAGES :       %010llX  (%d bits)\n", PAGES, BITS_PAGES );
    printf( "// WORDS :       %010llX  (%d bits)\n", WORDS, BITS_WORDS );

#pragma omp parallel for
    for( int iPage = 0; iPage < PAGES; iPage++ )
    {
#if USE_OMP
        const int       iThread = omp_get_thread_num(); // Get Thread Index: 0 .. nCores-1
#else
        const int       iThread = 0;
#endif
              char      keytext[8] = { 0, 0, 0, 0, 0, 0, 0, 0 };
              uint64_t  Haystack   = ((size_t)iPage) << BITS_WORDS;
              uint64_t  iWord      = WORDS;
              uint32_t  crc;

        while (iWord --> 0)
        {
#if 1 // Sans inlining: 3:23
            unsigned char *pNeedle  = (unsigned char*) &Haystack; // Printable v1
            crc = gpCRC32( LENGTH, pNeedle );
#else // With inlining: 3:04
            crc = -1;
            size_t nLength = LENGTH;
            size_t nData = Haystack;
            while (nLength -- > 0)
            {
                unsigned char bits = nData & 0xFF;
                crc = CRC32_REVERSE[ (crc ^ bits) & 0xFF ] ^ (crc >> 8); // reverse/reflected form
                nData >>= 8;
            }
            crc = ~crc;
#endif

            if (crc == 0)
            {
                Printable3( LENGTH, Haystack, keytext );
                printf( ", 0x%0*llX // %s  [#%02d, Page: %0*X] %5.2f%%\n", LENGTH*2, Haystack, keytext, iThread, (BITS_PAGES+3)/4, iPage, percent );
#pragma omp atomic
                found++;
            }
            Haystack++;
        }
#pragma omp atomic
        progress++;
        percent = 100.0 * (double)progress / (double)PAGES;
    }

    FOOTER;
}

// ========================================================================
void SetString(uint64_t key, size_t length, unsigned char* data)
{
    for (int byte = 0; byte < length; byte++)
    {
        data[ byte ] = key & 0xFF;
        key >>= 8;
    }
}

// ========================================================================
int Usage()
{
    printf(
"Find CRC32 zeroes\n"
"Verion 1.1\n"
"https://github.com/Michaelangel007/crc32\n"
"\n"
"Usage:\n"
"    -?   Display usage\n"
"    -1   Search byte sequence length 1\n"
"    -2   Search byte sequence length 2\n"
"    -3   Search byte sequence length 3\n"
"    -4   Search byte sequence length 4\n"
"    -5   Search byte sequence length 5\n"
"    -6   Search byte sequence length 6\n"
"    -b   Use CRC32B (default)\n"
"    -c   Use CRC32C\n"
"    -j#  Use # threads\n"
"    -v   Verify CRC32B or CRC32C that generate 0\n"
"\n"
"Examples:\n"
"\n"
"Search input lengths 5 for CRC32B\n"
"    ./find_zero -b -5\n"
"Search input lengths 5 for CRC32C\n"
"    ./find_zero -c -5\n"
"Verify CRC32B\n"
"    ./find_zero -v\n"
"Verify CRC32C\n"
"    ./find_zero -c -v\n"
    );

    return 0;
}

// ========================================================================
void Verify(size_t lengths, uint64_t **tables)
{
    // Verification
    //
    // This SO (Stack Overflow) mentions the following CRC32C input bytes returns zero:
    //   https://stackoverflow.com/questions/25573743/can-crc32c-ever-return-to-0
    //           Hex              Text    CRC32C
    //   4-byte: AB 9B E0 9B      ????    0x73CBAF29
    //   5-byte: 44 59 42 7C 4F   DYB|O   0xCBA37AE9
    // I was able to verify that:
    //   There is only one 4-byte input sequence that generate a CRC32C() zero,
    //   There are exactly 256 4-byte input sequence that generates a CRC32C() zero.
    // One can cross-verify with the md5calc site to verify the ASCII strings.
    //
    // I also inspected the CRC32B input bytes that returns zero.
    //                            Text    CRC32B
    //   4-byte: 9D 0A D9 6D      ???m    0x00000000
    //   5-byte: 5B 6C 6D 62 43   [lmbC   0x00000000
    // I was able to verify that:
    //   There is only one 4-byte input squence that generates a CRC32B() zero,
    //   There are exactly 256 4-byte input sequence that generates a CRC32B() zero.
    // One can cross-verify with md5calc site to verify the ASCII strings along with
    // "On-line CRC calculation and free library" linked below.
    //
    // See:
    //   https://www.lammertbies.nl/comm/info/crc-calculation
    //   https://md5calc.com/hash/crc32c
    unsigned char data   [16]; //                   raw input bytes
             char keytext[16]; // "?????"           5 ascii chars
             char crctext[16]; // "00 00 00 00 00 " hexdump

    for (size_t length = 1; length < lengths; length++)
    {
        printf( "- %d -\n", (int)length );
        uint64_t *keys = tables[ length ];
        if (keys)
            for (size_t entry = 0; true; entry++)
            {
                uint64_t key = keys[ entry ];
                if (!key) break;

                SetString( key, length, data );
                uint32_t crc = gpCRC32( length, data );

                //sprintf( (char*)crctext, "%08X", val );
                Hexdump  ( key , length, keytext );
                Printable( data, length, crctext );
                printf( "CRC32%c( 0x%010llX ) = %08X // %s = %s", cSearchCRC32, key, crc, crctext, keytext );
                if (crc != 0)
                    printf( " ERROR: CRC32 not 0!" );
                printf( "\n" );
            }
    }
}

// ========================================================================
void Verify_CRC32B()
{
    // NOTE: In Little Endian format!
    uint64_t length4[] =
    {
          0x6DD90A9D // ?.?m
        , 0
    };

    uint64_t length5[] =
    {
          0x000B840633 // 3.?..
        , 0x01D0F50072 // r.??.
        , 0x0266170CF0 // ?..f.
        , 0x03BD660AB1 // ?.f?.
        , 0x04D0A213B5 // ?.??.
        , 0x050BD315F4 // ?.?..
        , 0x06BD311976 // v.1?.
        , 0x0766401F37 // 7.@f.
        , 0x0866B92B7E // ~+?f.
        , 0x09BDC82D3F // ?-??.
        , 0x0A0B2A21BD // ?!*..
        , 0x0BD05B27FC // ?'[?.
        , 0x0CBD9F3EF8 // ?>??.
        , 0x0D66EE38B9 // ?8?f.
        , 0x0ED00C343B // ;4.?.
        , 0x0F0B7D327A // z2}..
        , 0x10D1FE5CA9 // ?\??.
        , 0x110A8F5AE8 // ?Z?..
        , 0x12BC6D566A // jVm?.
        , 0x13671C502B // +P.g.
        , 0x140AD8492F // /I?..
        , 0x15D1A94F6E // nO??.
        , 0x16674B43EC // ?CKg.
        , 0x17BC3A45AD // ?E:?.
        , 0x18BCC371E4 // ?q??.
        , 0x1967B277A5 // ?w?g.
        , 0x1AD1507B27 // '{P?.
        , 0x1B0A217D66 // f}!..
        , 0x1C67E56462 // bd?g.
        , 0x1DBC946223 // #b??.
        , 0x1E0A766EA1 // ?nv..
        , 0x1FD10768E0 // ?h.?.
        , 0x206401B546 // F?.d.
        , 0x21BF70B307 // .?p?!
        , 0x220992BF85 // ???."
        , 0x23D2E3B9C4 // ????#
        , 0x24BF27A0C0 // ??'?$
        , 0x256456A681 // ??Vd%
        , 0x26D2B4AA03 // .???&
        , 0x2709C5AC42 // B??.'
        , 0x28093C980B // .?<.(
        , 0x29D24D9E4A // J?M?)
        , 0x2A64AF92C8 // ???d*
        , 0x2BBFDE9489 // ????+
        , 0x2CD21A8D8D // ??.?,
        , 0x2D096B8BCC // ??k.-
        , 0x2EBF89874E // N???.
        , 0x2F64F8810F // .??d/
        , 0x30BE7BEFDC // ??{?0
        , 0x31650AE99D // ??.e1
        , 0x32D3E8E51F // .???2
        , 0x330899E35E // ^??.3
        , 0x34655DFA5A // Z?]e4
        , 0x35BE2CFC1B // .?,?5
        , 0x3608CEF099 // ???.6
        , 0x37D3BFF6D8 // ????7
        , 0x38D346C291 // ??F?8
        , 0x390837C4D0 // ??7.9
        , 0x3ABED5C852 // R???:
        , 0x3B65A4CE13 // .??e;
        , 0x3C0860D717 // .?`.<
        , 0x3DD311D156 // V?.?=
        , 0x3E65F3DDD4 // ???e>
        , 0x3FBE82DB95 // ?????
        , 0x40D48F60D9 // ?`??@
        , 0x410FFE6698 // ?f?.A
        , 0x42B91C6A1A // .j.?B
        , 0x43626D6C5B // [lmbC
        , 0x440FA9755F // _u?.D
        , 0x45D4D8731E // .s??E
        , 0x46623A7F9C // ??:bF
        , 0x47B94B79DD // ?yK?G
        , 0x48B9B24D94 // ?M??H
        , 0x4962C34BD5 // ?K?bI
        , 0x4AD4214757 // WG!?J
        , 0x4B0F504116 // .AP.K
        , 0x4C62945812 // .X?bL
        , 0x4DB9E55E53 // S^??M
        , 0x4E0F0752D1 // ?R..N
        , 0x4FD4765490 // ?Tv?O
        , 0x500EF53A43 // C:?.P
        , 0x51D5843C02 // .<??Q
        , 0x5263663080 // ?0fcR
        , 0x53B81736C1 // ?6.?S
        , 0x54D5D32FC5 // ?/??T
        , 0x550EA22984 // ?)?.U
        , 0x56B8402506 // .%@?V
        , 0x5763312347 // G#1cW
        , 0x5863C8170E // ..?cX
        , 0x59B8B9114F // O.??Y
        , 0x5A0E5B1DCD // ?.[.Z
        , 0x5BD52A1B8C // ?.*?[
        , 0x5CB8EE0288 // ?.??\ //
        , 0x5D639F04C9 // ?.?c]
        , 0x5ED57D084B // K.}?^
        , 0x5F0E0C0E0A // ...._
        , 0x60BB0AD3AC // ??.?`
        , 0x61607BD5ED // ??{`a
        , 0x62D699D96F // o???b
        , 0x630DE8DF2E // .??.c
        , 0x64602CC62A // *?,`d
        , 0x65BB5DC06B // k?]?e
        , 0x660DBFCCE9 // ???.f
        , 0x67D6CECAA8 // ????g
        , 0x68D637FEE1 // ??7?h
        , 0x690D46F8A0 // ??F.i
        , 0x6ABBA4F422 // "???j
        , 0x6B60D5F263 // c??`k
        , 0x6C0D11EB67 // g?..l
        , 0x6DD660ED26 // &?`?m
        , 0x6E6082E1A4 // ???`n
        , 0x6FBBF3E7E5 // ????o
        , 0x7061708936 // 6?pap
        , 0x71BA018F77 // w?.?q
        , 0x720CE383F5 // ???.r
        , 0x73D79285B4 // ????s
        , 0x74BA569CB0 // ??V?t
        , 0x7561279AF1 // ??'au
        , 0x76D7C59673 // s???v
        , 0x770CB49032 // 2??.w
        , 0x780C4DA47B // {?M.x
        , 0x79D73CA23A // :?<?y
        , 0x7A61DEAEB8 // ???az
        , 0x7BBAAFA8F9 // ????{
        , 0x7CD76BB1FD // ??k?|
        , 0x7D0C1AB7BC // ??..}
        , 0x7EBAF8BB3E // >???~
        , 0x7F6189BD7F // ???a?
        , 0x806EE3CDA6 // ???n?
        , 0x81B592CBE7 // ?????
        , 0x820370C765 // e?p.?
        , 0x83D801C124 // $?.??
        , 0x84B5C5D820 // .????
        , 0x856EB4DE61 // a??n?
        , 0x86D856D2E3 // ??V??
        , 0x870327D4A2 // ??'.?
        , 0x8803DEE0EB // ???.?
        , 0x89D8AFE6AA // ?????
        , 0x8A6E4DEA28 // (?Mn?
        , 0x8BB53CEC69 // i?<??
        , 0x8CD8F8F56D // m????
        , 0x8D0389F32C // ,??.?
        , 0x8EB56BFFAE // ??k??
        , 0x8F6E1AF9EF // ??.n?
        , 0x90B499973C // <????
        , 0x916FE8917D // }??o?
        , 0x92D90A9DFF // ??.??
        , 0x93027B9BBE // ??{.?
        , 0x946FBF82BA // ???o?
        , 0x95B4CE84FB // ?????
        , 0x96022C8879 // y?,.?
        , 0x97D95D8E38 // 8?]??
        , 0x98D9A4BA71 // q????
        , 0x9902D5BC30 // 0??.?
        , 0x9AB437B0B2 // ??7??
        , 0x9B6F46B6F3 // ??Fo?
        , 0x9C0282AFF7 // ???.?
        , 0x9DD9F3A9B6 // ?????
        , 0x9E6F11A534 // 4?.o?
        , 0x9FB460A375 // u?`??
        , 0xA001667ED3 // ?~f.?
        , 0xA1DA177892 // ?x.??
        , 0xA26CF57410 // .t?l?
        , 0xA3B7847251 // Qr???
        , 0xA4DA406B55 // Uk@??
        , 0xA501316D14 // .m1.?
        , 0xA6B7D36196 // ?a???
        , 0xA76CA267D7 // ?g?l?
        , 0xA86C5B539E // ?S[l?
        , 0xA9B72A55DF // ?U*??
        , 0xAA01C8595D // ]Y?.?
        , 0xABDAB95F1C // ._???
        , 0xACB77D4618 // .F}??
        , 0xAD6C0C4059 // Y@.l?
        , 0xAEDAEE4CDB // ?L???
        , 0xAF019F4A9A // ?J?.?
        , 0xB0DB1C2449 // I$.??
        , 0xB1006D2208 // ."m.?
        , 0xB2B68F2E8A // ?.???
        , 0xB36DFE28CB // ?(?m?
        , 0xB4003A31CF // ?1:.?
        , 0xB5DB4B378E // ?7K??
        , 0xB66DA93B0C // .;?m?
        , 0xB7B6D83D4D // M=???
        , 0xB8B6210904 // ..!??
        , 0xB96D500F45 // E.Pm?
        , 0xBADBB203C7 // ?.???
        , 0xBB00C30586 // ?.?.?
        , 0xBC6D071C82 // ?..m?
        , 0xBDB6761AC3 // ?.v??
        , 0xBE00941641 // A.?.?
        , 0xBFDBE51000 // ..???
        , 0xC0B1E8AB4C // L????
        , 0xC16A99AD0D // .??j?
        , 0xC2DC7BA18F // ??{??
        , 0xC3070AA7CE // ??..?
        , 0xC46ACEBECA // ???j?
        , 0xC5B1BFB88B // ?????
        , 0xC6075DB409 // .?].?
        , 0xC7DC2CB248 // H?,??
        , 0xC8DCD58601 // .????
        , 0xC907A48040 // @??.?
        , 0xCAB1468CC2 // ??F??
        , 0xCB6A378A83 // ??7j?
        , 0xCC07F39387 // ???.?
        , 0xCDDC8295C6 // ?????
        , 0xCE6A609944 // D?`j?
        , 0xCFB1119F05 // .?.??
        , 0xD06B92F1D6 // ???k?
        , 0xD1B0E3F797 // ?????
        , 0xD20601FB15 // .?..?
        , 0xD3DD70FD54 // T?p??
        , 0xD4B0B4E450 // P????
        , 0xD56BC5E211 // .??k?
        , 0xD6DD27EE93 // ??'??
        , 0xD70656E8D2 // ??V.?
        , 0xD806AFDC9B // ???.?
        , 0xD9DDDEDADA // ?????
        , 0xDA6B3CD658 // X?<k?
        , 0xDBB04DD019 // .?M??
        , 0xDCDD89C91D // .????
        , 0xDD06F8CF5C // \??.?
        , 0xDEB01AC3DE // ??.??
        , 0xDF6B6BC59F // ??kk?
        , 0xE0DE6D1839 // 9.m??
        , 0xE1051C1E78 // x...?
        , 0xE2B3FE12FA // ?.???
        , 0xE3688F14BB // ?.?h?
        , 0xE4054B0DBF // ?.K.?
        , 0xE5DE3A0BFE // ?.:??
        , 0xE668D8077C // |.?h?
        , 0xE7B3A9013D // =.???
        , 0xE8B3503574 // t5P??
        , 0xE968213335 // 53!h?
        , 0xEADEC33FB7 // ?????
        , 0xEB05B239F6 // ?9?.?
        , 0xEC687620F2 // ?.vh?
        , 0xEDB30726B3 // ?&.??
        , 0xEE05E52A31 // 1*?.?
        , 0xEFDE942C70 // p,???
        , 0xF0041742A3 // ?B..?
        , 0xF1DF6644E2 // ?Df??
        , 0xF269844860 // `H?i?
        , 0xF3B2F54E21 // !N???
        , 0xF4DF315725 // %W1??
        , 0xF504405164 // dQ@.?
        , 0xF6B2A25DE6 // ?]???
        , 0xF769D35BA7 // ?[?i?
        , 0xF8692A6FEE // ?o*i?
        , 0xF9B25B69AF // ?i[??
        , 0xFA04B9652D // -e?.?
        , 0xFBDFC8636C // lc???
        , 0xFCB20C7A68 // hz.??
        , 0xFD697D7C29 // )|}i?
        , 0xFEDF9F70AB // ?p???
        , 0xFF04EE76EA // ?v?.?
        , 0 // Found 256 of length 5 in 00:02:30.510
    };

    uint64_t *lengths[]
    {
          nullptr // 0
        , nullptr // 1
        , nullptr // 2
        , nullptr // 3
        , length4 // 4
        , length5 // 5
    };

    size_t entries = sizeof(lengths) / sizeof(uint64_t*);
    Verify( entries, lengths );
}

// ========================================================================
void Verify_CRC32C()
{
    uint64_t length4[] = {
          0x9BE09BAB // ????
        , 0
    };

    uint64_t length5[] =
    {
          0x00345564AA // ?dU4.
        , 0x0131B9125B // [.?1.
        , 0x023F8D8948 // H???.
        , 0x033A61FFB9 // ??a:.
        , 0x0423E4BF6E // n??#.
        , 0x052608C99F // ??.&.
        , 0x06283C528C // ?R<(.
        , 0x072DD0247D // }$?-.
        , 0x081B36D322 // "?6..
        , 0x091EDAA5D3 // ???..
        , 0x0A10EE3EC0 // ?>?..
        , 0x0B15024831 // 1H...
        , 0x0C0C8708E6 // ?.?..
        , 0x0D096B7E17 // .~k..
        , 0x0E075FE504 // .?_..
        , 0x0F02B393F5 // ???..
        , 0x106A920BBA // ?.?j.
        , 0x116F7E7D4B // K}~o.
        , 0x12614AE658 // X?Ja.
        , 0x1364A690A9 // ???d.
        , 0x147D23D07E // ~?#}.
        , 0x1578CFA68F // ???x.
        , 0x1676FB3D9C // ?=?v.
        , 0x1773174B6D // mK.s.
        , 0x1845F1BC32 // 2??E.
        , 0x19401DCAC3 // ??.@.
        , 0x1A4E2951D0 // ?Q)N.
        , 0x1B4BC52721 // !'?K.
        , 0x1C524067F6 // ?g@R.
        , 0x1D57AC1107 // ..?W.
        , 0x1E59988A14 // .??Y.
        , 0x1F5C74FCE5 // ??t\.
        , 0x2089DBBA8A // ???? 
        , 0x218C37CC7B // {?7?!
        , 0x2282035768 // hW.?"
        , 0x2387EF2199 // ?!??#
        , 0x249E6A614E // Naj?$
        , 0x259B8617BF // ?.??%
        , 0x2695B28CAC // ????&
        , 0x27905EFA5D // ]?^?'
        , 0x28A6B80D02 // ..??(
        , 0x29A3547BF3 // ?{T?)
        , 0x2AAD60E0E0 // ??`?*
        , 0x2BA88C9611 // .???+
        , 0x2CB109D6C6 // ??.?,
        , 0x2DB4E5A037 // 7???-
        , 0x2EBAD13B24 // $;??.
        , 0x2FBF3D4DD5 // ?M=?/
        , 0x30D71CD59A // ??.?0
        , 0x31D2F0A36B // k???1
        , 0x32DCC43878 // x8??2
        , 0x33D9284E89 // ?N(?3
        , 0x34C0AD0E5E // ^.??4
        , 0x35C54178AF // ?xA?5
        , 0x36CB75E3BC // ??u?6
        , 0x37CE99954D // M???7
        , 0x38F87F6212 // .b??8
        , 0x39FD9314E3 // ?.??9
        , 0x3AF3A78FF0 // ????:
        , 0x3BF64BF901 // .?K?;
        , 0x3CEFCEB9D6 // ????<
        , 0x3DEA22CF27 // '?"?=
        , 0x3EE4165434 // 4T.?>
        , 0x3FE1FA22C5 // ?"???
        , 0x404AA4AE1B // .??J@
        , 0x414F48D8EA // ??HOA
        , 0x42417C43F9 // ?C|AB
        , 0x4344903508 // .5?DC
        , 0x445D1575DF // ?u.]D
        , 0x4558F9032E // ..?XE
        , 0x4656CD983D // =??VF
        , 0x475321EECC // ??!SG
        , 0x4865C71993 // ?.?eH
        , 0x49602B6F62 // bo+`I
        , 0x4A6E1FF471 // q?.nJ
        , 0x4B6BF38280 // ???kK
        , 0x4C7276C257 // W?vrL
        , 0x4D779AB4A6 // ???wM
        , 0x4E79AE2FB5 // ?/?yN
        , 0x4F7C425944 // DYB|O
        , 0x501463C10B // .?c.P
        , 0x51118FB7FA // ???.Q
        , 0x521FBB2CE9 // ?,?.R
        , 0x531A575A18 // .ZW.S
        , 0x5403D21ACF // ?.?.T
        , 0x55063E6C3E // >l>.U
        , 0x56080AF72D // -?..V
        , 0x570DE681DC // ???.W
        , 0x583B007683 // ?v.;X
        , 0x593EEC0072 // r.?>Y
        , 0x5A30D89B61 // a??0Z
        , 0x5B3534ED90 // ??45[
        , 0x5C2CB1AD47 // G??,\ //
        , 0x5D295DDBB6 // ??])]
        , 0x5E276940A5 // ?@i'^
        , 0x5F22853654 // T6?"_
        , 0x60F72A703B // ;p*?`
        , 0x61F2C606CA // ?.??a
        , 0x62FCF29DD9 // ????b
        , 0x63F91EEB28 // (?.?c
        , 0x64E09BABFF // ????d
        , 0x65E577DD0E // .?w?e
        , 0x66EB43461D // .FC?f
        , 0x67EEAF30EC // ?0??g
        , 0x68D849C7B3 // ??I?h
        , 0x69DDA5B142 // B???i
        , 0x6AD3912A51 // Q*??j
        , 0x6BD67D5CA0 // ?\}?k
        , 0x6CCFF81C77 // w.??l
        , 0x6DCA146A86 // ?j.?m
        , 0x6EC420F195 // ?? ?n
        , 0x6FC1CC8764 // d???o
        , 0x70A9ED1F2B // +.??p
        , 0x71AC0169DA // ?i.?q
        , 0x72A235F2C9 // ??5?r
        , 0x73A7D98438 // 8???s
        , 0x74BE5CC4EF // ??\?t
        , 0x75BBB0B21E // .???u
        , 0x76B584290D // .)??v
        , 0x77B0685FFC // ?_h?w
        , 0x78868EA8A3 // ????x
        , 0x798362DE52 // R?b?y
        , 0x7A8D564541 // AEV?z
        , 0x7B88BA33B0 // ?3??{
        , 0x7C913F7367 // gs??|
        , 0x7D94D30596 // ?.??}
        , 0x7E9AE79E85 // ????~
        , 0x7F9F0BE874 // t?.?? 
        , 0x80C9B6F1C8 // ?????
        , 0x81CC5A8739 // 9?Z??
        , 0x82C26E1C2A // *.n??
        , 0x83C7826ADB // ?j???
        , 0x84DE072A0C // .*.??
        , 0x85DBEB5CFD // ?\???
        , 0x86D5DFC7EE // ?????
        , 0x87D033B11F // .?3??
        , 0x88E6D54640 // @F???
        , 0x89E33930B1 // ?09??
        , 0x8AED0DABA2 // ??.??
        , 0x8BE8E1DD53 // S????
        , 0x8CF1649D84 // ??d??
        , 0x8DF488EB75 // u????
        , 0x8EFABC7066 // fp???
        , 0x8FFF500697 // ?.P??
        , 0x9097719ED8 // ??q??
        , 0x91929DE829 // )????
        , 0x929CA9733A // :s???
        , 0x93994505CB // ?.E??
        , 0x9480C0451C // .E???
        , 0x95852C33ED // ?3,??
        , 0x968B18A8FE // ??.??
        , 0x978EF4DE0F // .????
        , 0x98B8122950 // P).??
        , 0x99BDFE5FA1 // ?_???
        , 0x9AB3CAC4B2 // ?????
        , 0x9BB626B243 // C?&??
        , 0x9CAFA3F294 // ?????
        , 0x9DAA4F8465 // e?O??
        , 0x9EA47B1F76 // v.{??
        , 0x9FA1976987 // ?i???
        , 0xA074382FE8 // ?/8t?
        , 0xA171D45919 // .Y?q?
        , 0xA27FE0C20A // .????
        , 0xA37A0CB4FB // ??.z?
        , 0xA46389F42C // ,??c?
        , 0xA5666582DD // ??ef?
        , 0xA6685119CE // ?.Qh?
        , 0xA76DBD6F3F // ?o?m?
        , 0xA85B5B9860 // `?[[?
        , 0xA95EB7EE91 // ???^?
        , 0xAA50837582 // ?u?P?
        , 0xAB556F0373 // s.oU?
        , 0xAC4CEA43A4 // ?C?L?
        , 0xAD49063555 // U5.I?
        , 0xAE4732AE46 // F?2G?
        , 0xAF42DED8B7 // ???B?
        , 0xB02AFF40F8 // ?@?*?
        , 0xB12F133609 // .6./?
        , 0xB22127AD1A // .?'!?
        , 0xB324CBDBEB // ???$?
        , 0xB43D4E9B3C // <?N=?
        , 0xB538A2EDCD // ???8?
        , 0xB6369676DE // ?v?6?
        , 0xB7337A002F // /.z3?
        , 0xB8059CF770 // p??.?
        , 0xB900708181 // ??p.?
        , 0xBA0E441A92 // ?.D.?
        , 0xBB0BA86C63 // cl?.?
        , 0xBC122D2CB4 // ?,-.?
        , 0xBD17C15A45 // EZ?.?
        , 0xBE19F5C156 // V??.?
        , 0xBF1C19B7A7 // ??..?
        , 0xC0B7473B79 // y;G??
        , 0xC1B2AB4D88 // ?M???
        , 0xC2BC9FD69B // ?????
        , 0xC3B973A06A // j?s??
        , 0xC4A0F6E0BD // ?????
        , 0xC5A51A964C // L?.??
        , 0xC6AB2E0D5F // _..??
        , 0xC7AEC27BAE // ?{???
        , 0xC898248CF1 // ??$??
        , 0xC99DC8FA00 // .????
        , 0xCA93FC6113 // .a???
        , 0xCB961017E2 // ?..??
        , 0xCC8F955735 // 5W???
        , 0xCD8A7921C4 // ?!y??
        , 0xCE844DBAD7 // ??M??
        , 0xCF81A1CC26 // &????
        , 0xD0E9805469 // iT???
        , 0xD1EC6C2298 // ?"l??
        , 0xD2E258B98B // ??X??
        , 0xD3E7B4CF7A // z????
        , 0xD4FE318FAD // ??1??
        , 0xD5FBDDF95C // \????
        , 0xD6F5E9624F // Ob???
        , 0xD7F00514BE // ?..??
        , 0xD8C6E3E3E1 // ?????
        , 0xD9C30F9510 // .?.??
        , 0xDACD3B0E03 // ..;??
        , 0xDBC8D778F2 // ?x???
        , 0xDCD1523825 // %8R??
        , 0xDDD4BE4ED4 // ?N???
        , 0xDEDA8AD5C7 // ?????
        , 0xDFDF66A336 // 6?f??
        , 0xE00AC9E559 // Y??.?
        , 0xE10F2593A8 // ??%.?
        , 0xE2011108BB // ?...?
        , 0xE304FD7E4A // J~?.?
        , 0xE41D783E9D // ?>x.?
        , 0xE51894486C // lH?.?
        , 0xE616A0D37F // ???.?
        , 0xE7134CA58E // ??L.?
        , 0xE825AA52D1 // ?R?%?
        , 0xE920462420 //  $F ?
        , 0xEA2E72BF33 // 3?r.?
        , 0xEB2B9EC9C2 // ???+?
        , 0xEC321B8915 // .?.2?
        , 0xED37F7FFE4 // ???7?
        , 0xEE39C364F7 // ?d?9?
        , 0xEF3C2F1206 // ../<?
        , 0xF0540E8A49 // I?.T?
        , 0xF151E2FCB8 // ???Q?
        , 0xF25FD667AB // ?g?_?
        , 0xF35A3A115A // Z.:Z?
        , 0xF443BF518D // ?Q?C?
        , 0xF54653277C // |'SF?
        , 0xF64867BC6F // o?gH?
        , 0xF74D8BCA9E // ???M?
        , 0xF87B6D3DC1 // ?=m{?
        , 0xF97E814B30 // 0K?~?
        , 0xFA70B5D023 // #??p?
        , 0xFB7559A6D2 // ??Yu?
        , 0xFC6CDCE605 // .??l?
        , 0xFD693090F4 // ??0i?
        , 0xFE67040BE7 // ?..g?
        , 0xFF62E87D16 // .}?b?
        , 0 // Found 256 of length 5 in 00:03:40.721
    };

    uint64_t *lengths[] =
    {
          nullptr // 0
        , nullptr // 1
        , nullptr // 2
        , nullptr // 3
        , length4 // 4
        , length5 // 5
    };

    size_t entries = sizeof(lengths) / sizeof(uint64_t*);
    Verify( entries, lengths );
}

// ========================================================================
int main(int nArg, char *aArg[])
{
    common_init( false );

    bool bSearchLengths[8] = { 0, 0, 0, 0, 0, 0, 0, 0 };
    bool bSearchGeneric = false;

#if USE_OMP
    Threads_Default();
#endif // USE_OMP

    int   iArg = 0;

    if( nArg > 1 )
    {
        while( iArg < nArg )
        {
            char *pArg = aArg[ iArg + 1 ];
            if(  !pArg )
                break;

            if( pArg[0] == '-' )
            {
                iArg++;
                pArg++; // point to 1st char in option

                if (*pArg == '?') return Usage();
                if((*pArg >= '0') && (*pArg <= '6'))
                    bSearchLengths[ *pArg - '0' ] = true;
                else
                if (*pArg == 'b')
                {
                    bSearchCRC32B = true;
                    cSearchCRC32  = 'b';
                    gpCRC32 = crc32_reverse;
                }
                else
                if (*pArg == 'c')
                {
                    bSearchCRC32B = false;
                    cSearchCRC32  = 'c';
                    gpCRC32 = crc32c_reverse;
                }
                else
#if USE_OMP
                if( *pArg == 'j' )
                {
                    int i = atoi( pArg+1 ); 
                    if( i > 0 )
                        gnThreadsActive = i;
                    if( gnThreadsActive > MAX_THREADS )
                        gnThreadsActive = MAX_THREADS;
                }
#endif // USE_OMP
                else
                if (*pArg == 'n')
                    bSearchGeneric = true;
                else
                if (*pArg == 'p')
                    bShowProgress = true;
                else
                if (*pArg == 'v')
                {
                    if (bSearchCRC32B)
                        Verify_CRC32B();
                    else
                        Verify_CRC32C();
                    return 0;
                }
                else
                    printf( "Unrecognized option: %c\n", *pArg ); 
            }
            else
                break;
        }
    }

#if USE_OMP
    Threads_Set();
#endif // USE_OMP

    gpCRC32 = bSearchCRC32B ? crc32_reverse : crc32c_reverse;

    printf( "Using: %u / %u threads\n", gnThreadsActive, gnThreadsMaximum );
    printf( "Searching for CRC32%c() zero hash...\n", cSearchCRC32 );

    if (!bSearchGeneric)
    {
        if( bSearchLengths[1] ) Measure( SearchLen1 );
        if( bSearchLengths[2] ) Measure( SearchLen2 );
        if( bSearchLengths[3] ) Measure( SearchLen3 );
        if( bSearchLengths[4] ) Measure( SearchLen4 );
        if( bSearchLengths[5] ) Measure( SearchLen5 );
        if( bSearchLengths[6] ) Measure( SearchLen6 );
    }
    else
        for (int length = 1; length < 8; length++)
        {
            if (!bSearchLengths[length])
                continue;

            Timer timer;
            timer.Start();
                SearchLenN( length );
            timer.Stop();
            timer.Print();
        }

    printf( "Done.\n" );

    return 0;
}

