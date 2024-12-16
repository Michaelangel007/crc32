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
    // NEEDS variables: LENGTH, RANGE
    #define HEADER   printf( "// Searching length %d...\n", LENGTH )
    #define FOOTER   printf( "// Found %d of length %d in ", total, LENGTH ); return RANGE

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

// ========================================================================
size_t SearchLen1()
{
    const int LENGTH = 1;
    const int RANGE  = 0x100;
          int total  = 0;
    HEADER;

    for( int bytes1 = 0; bytes1 < RANGE; bytes1++ )
    {
        unsigned char data1[1];
        *((uint8_t *)data1) = bytes1;

        unsigned int result1 = crc32_forward( 1, data1 );
        if (result1 == 0)
        {
            printf( ", 0x%08X\n", bytes1 );
            total++;
        }
    }

    FOOTER;
}

size_t SearchLen2()
{
    const int LENGTH = 2;
    const int RANGE  = 0x10000;
          int total  = 0;
    HEADER;

    for (int bytes2 = 0; bytes2 < RANGE; bytes2++)
    {
        unsigned char data2[2];
        *((uint16_t *)data2) = bytes2;
        unsigned int crc = crc32_forward( 2, data2 );
        if (crc == 0)
        {
            printf( ", 0x%08X\n", bytes2 );
            total++;
        }
    }

    FOOTER;
}

size_t SearchLen3()
{
    const int LENGTH = 3;
    const int RANGE  = 0x1000000; // 3 bytes
          int total  = 0;
    HEADER;

    for (int bytes3 = 0; bytes3 < RANGE; bytes3++)
    {
        unsigned char data3[LENGTH+1];
        *((uint32_t *)data3) = bytes3;

        unsigned int crc = crc32_forward( LENGTH, data3 );
        if (crc == 0)
        {
            printf( ", 0x%08X\n", bytes3 );
            total++;
        }
    }

    FOOTER;
}

size_t SearchLen4()
{
    const int    LENGTH    = 4;
    const int    PAGE      = 0x100;
    const int    SUB_RANGE = 0x1000000; // 3 bytes
    const size_t RANGE     = (size_t)PAGE * (size_t)SUB_RANGE;
          int    total     = 0;
    HEADER;

#pragma omp parallel for
    for( int iPage = 0; iPage < PAGE; iPage++ )
    {
#if USE_OMP
        int iThread = omp_get_thread_num(); // Get Thread Index: 0 .. nCores-1
#else
        const int iThread = 0;
#endif
        char crctext[8];

        uint32_t       Haystack = ((uint32_t)iPage    ) << 24;
        uint32_t       Remain   = SUB_RANGE;
        unsigned char *pNeedle  = (unsigned char*) &Haystack;
        unsigned int   crc;

        while( Remain --> 0 )
        {
            // https://stackoverflow.com/questions/25573743/can-crc32c-ever-return-to-0
            // 4-byte: ab 9b e0 9b
            // 5-byte: DYB|O
            // Which poly is this???  B69B50B9

            // 6DD90A9D
            crc = crc32_reverse( LENGTH, pNeedle );
            if (crc == 0)
            {
                Printable( pNeedle, LENGTH, crctext );
                printf( ", 0x%08X // %s  [#%02d, Page: %02X]\n", Haystack, crctext, iThread, iPage );
#pragma omp atomic
                total++;
            }
            Haystack++;
        }
    }

    FOOTER;
}

size_t SearchLen5()
{
    const int    LENGTH    = 5;
    const int    PAGE      = 0x100;
    const size_t SUB_RANGE = 0x100000000; // 4 bytes
    const size_t RANGE     = (size_t)PAGE * (size_t)SUB_RANGE;
                 int total = 0;
    HEADER;

#pragma omp parallel for
    for( int iPage = 0; iPage < PAGE; iPage++ )
    {
#if USE_OMP
        const int iThread = omp_get_thread_num(); // Get Thread Index: 0 .. nCores-1
#else
        const int iThread = 0;
#endif
        char           crctext[8];
        uint64_t       Haystack = ((uint64_t)iPage    ) << 32;
        uint64_t       Remain   = SUB_RANGE;
        unsigned int   crc;

        while( Remain --> 0 )
        {
#if 1 // Sans inlining: 2:19
            unsigned char *pNeedle  = (unsigned char*) &Haystack; // Printable v1
            crc = crc32_reverse( LENGTH, pNeedle );
#else // With inlining: 2:27
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
                Printable( pNeedle, LENGTH, crctext );
                printf( ", 0x%010llX // %s  [#%02d, Page: %02X]\n", Haystack, crctext, iThread, iPage );
#pragma omp atomic
                total++;
            }
            Haystack++;
        }
    }

    FOOTER;
}

void SetString(uint64_t key, size_t length, unsigned char* data)
{
    for (int byte = 0; byte < length; byte++)
    {
        data[ byte ] = key & 0xFF;
        key >>= 8;
    }
}

int Usage()
{
    printf(
"-?   Display usage\n"
"-1   Search byte sequence length 1\n"
"-2   Search byte sequence length 1\n"
"-3   Search byte sequence length 1\n"
"-4   Search byte sequence length 1\n"
"-5   Search byte sequence length 1\n"
"-j#  Use # threads\n"
"-v   Verify CRC32s that generate 0\n"
    );

    return 0;
}

int Verify()
{
    // In Little Endian format!
    uint64_t length4[] = {
//  b3b2b1b0
  0x6DD90A9D // ?.?m  [#18, Page: 6D]
// Found 1 of length 4 in 00:00:00.718
        , 0
    };
    uint64_t length5[] = {
  0x000B840633 // 3.?..  [#00, Page: 00]
, 0x01D0F50072 // r.??.  [#00, Page: 01]
, 0x0266170CF0 // ?..f.  [#00, Page: 02]
, 0x03BD660AB1 // ?.f?.  [#00, Page: 03]
, 0x04D0A213B5 // ?.??.  [#00, Page: 04]
, 0x050BD315F4 // ?.?..  [#00, Page: 05]
, 0x06BD311976 // v.1?.  [#01, Page: 06]
, 0x0766401F37 // 7.@f.  [#01, Page: 07]
, 0x0866B92B7E // ~+?f.  [#01, Page: 08]
, 0x09BDC82D3F // ?-??.  [#01, Page: 09]
, 0x0A0B2A21BD // ?!*..  [#01, Page: 0A]
, 0x0BD05B27FC // ?'[?.  [#01, Page: 0B]
, 0x0CBD9F3EF8 // ?>??.  [#02, Page: 0C]
, 0x0D66EE38B9 // ?8?f.  [#02, Page: 0D]
, 0x0ED00C343B // ;4.?.  [#02, Page: 0E]
, 0x0F0B7D327A // z2}..  [#02, Page: 0F]
, 0x10D1FE5CA9 // ?\??.  [#02, Page: 10]
, 0x110A8F5AE8 // ?Z?..  [#02, Page: 11]
, 0x12BC6D566A // jVm?.  [#03, Page: 12]
, 0x13671C502B // +P.g.  [#03, Page: 13]
, 0x140AD8492F // /I?..  [#03, Page: 14]
, 0x15D1A94F6E // nO??.  [#03, Page: 15]
, 0x16674B43EC // ?CKg.  [#03, Page: 16]
, 0x17BC3A45AD // ?E:?.  [#03, Page: 17]
, 0x18BCC371E4 // ?q??.  [#04, Page: 18]
, 0x1967B277A5 // ?w?g.  [#04, Page: 19]
, 0x1AD1507B27 // '{P?.  [#04, Page: 1A]
, 0x1B0A217D66 // f}!..  [#04, Page: 1B]
, 0x1C67E56462 // bd?g.  [#04, Page: 1C]
, 0x1DBC946223 // #b??.  [#04, Page: 1D]
, 0x1E0A766EA1 // ?nv..  [#05, Page: 1E]
, 0x1FD10768E0 // ?h.?.  [#05, Page: 1F]
, 0x206401B546 // F?.d.  [#05, Page: 20]
, 0x21BF70B307 // .?p?!  [#05, Page: 21]
, 0x220992BF85 // ???."  [#05, Page: 22]
, 0x23D2E3B9C4 // ????#  [#05, Page: 23]
, 0x24BF27A0C0 // ??'?$  [#06, Page: 24]
, 0x256456A681 // ??Vd%  [#06, Page: 25]
, 0x26D2B4AA03 // .???&  [#06, Page: 26]
, 0x2709C5AC42 // B??.'  [#06, Page: 27]
, 0x28093C980B // .?<.(  [#06, Page: 28]
, 0x29D24D9E4A // J?M?)  [#06, Page: 29]
, 0x2A64AF92C8 // ???d*  [#07, Page: 2A]
, 0x2BBFDE9489 // ????+  [#07, Page: 2B]
, 0x2CD21A8D8D // ??.?,  [#07, Page: 2C]
, 0x2D096B8BCC // ??k.-  [#07, Page: 2D]
, 0x2EBF89874E // N???.  [#07, Page: 2E]
, 0x2F64F8810F // .??d/  [#07, Page: 2F]
, 0x30BE7BEFDC // ??{?0  [#08, Page: 30]
, 0x31650AE99D // ??.e1  [#08, Page: 31]
, 0x32D3E8E51F // .???2  [#08, Page: 32]
, 0x330899E35E // ^??.3  [#08, Page: 33]
, 0x34655DFA5A // Z?]e4  [#08, Page: 34]
, 0x35BE2CFC1B // .?,?5  [#08, Page: 35]
, 0x3608CEF099 // ???.6  [#09, Page: 36]
, 0x37D3BFF6D8 // ????7  [#09, Page: 37]
, 0x38D346C291 // ??F?8  [#09, Page: 38]
, 0x390837C4D0 // ??7.9  [#09, Page: 39]
, 0x3ABED5C852 // R???:  [#09, Page: 3A]
, 0x3B65A4CE13 // .??e;  [#09, Page: 3B]
, 0x3C0860D717 // .?`.<  [#10, Page: 3C]
, 0x3DD311D156 // V?.?=  [#10, Page: 3D]
, 0x3E65F3DDD4 // ???e>  [#10, Page: 3E]
, 0x3FBE82DB95 // ?????  [#10, Page: 3F]
, 0x40D48F60D9 // ?`??@  [#10, Page: 40]
, 0x410FFE6698 // ?f?.A  [#10, Page: 41]
, 0x42B91C6A1A // .j.?B  [#11, Page: 42]
, 0x43626D6C5B // [lmbC  [#11, Page: 43]
, 0x440FA9755F // _u?.D  [#11, Page: 44]
, 0x45D4D8731E // .s??E  [#11, Page: 45]
, 0x46623A7F9C // ??:bF  [#11, Page: 46]
, 0x47B94B79DD // ?yK?G  [#11, Page: 47]
, 0x48B9B24D94 // ?M??H  [#12, Page: 48]
, 0x4962C34BD5 // ?K?bI  [#12, Page: 49]
, 0x4AD4214757 // WG!?J  [#12, Page: 4A]
, 0x4B0F504116 // .AP.K  [#12, Page: 4B]
, 0x4C62945812 // .X?bL  [#12, Page: 4C]
, 0x4DB9E55E53 // S^??M  [#12, Page: 4D]
, 0x4E0F0752D1 // ?R..N  [#13, Page: 4E]
, 0x4FD4765490 // ?Tv?O  [#13, Page: 4F]
, 0x500EF53A43 // C:?.P  [#13, Page: 50]
, 0x51D5843C02 // .<??Q  [#13, Page: 51]
, 0x5263663080 // ?0fcR  [#13, Page: 52]
, 0x53B81736C1 // ?6.?S  [#13, Page: 53]
, 0x54D5D32FC5 // ?/??T  [#14, Page: 54]
, 0x550EA22984 // ?)?.U  [#14, Page: 55]
, 0x56B8402506 // .%@?V  [#14, Page: 56]
, 0x5763312347 // G#1cW  [#14, Page: 57]
, 0x5863C8170E // ..?cX  [#14, Page: 58]
, 0x59B8B9114F // O.??Y  [#14, Page: 59]
, 0x5A0E5B1DCD // ?.[.Z  [#15, Page: 5A]
, 0x5BD52A1B8C // ?.*?[  [#15, Page: 5B]
, 0x5CB8EE0288 // ?.??\  [#15, Page: 5C]
, 0x5D639F04C9 // ?.?c]  [#15, Page: 5D]
, 0x5ED57D084B // K.}?^  [#15, Page: 5E]
, 0x5F0E0C0E0A // ...._  [#15, Page: 5F]
, 0x60BB0AD3AC // ??.?`  [#16, Page: 60]
, 0x61607BD5ED // ??{`a  [#16, Page: 61]
, 0x62D699D96F // o???b  [#16, Page: 62]
, 0x630DE8DF2E // .??.c  [#16, Page: 63]
, 0x64602CC62A // *?,`d  [#16, Page: 64]
, 0x65BB5DC06B // k?]?e  [#17, Page: 65]
, 0x660DBFCCE9 // ???.f  [#17, Page: 66]
, 0x67D6CECAA8 // ????g  [#17, Page: 67]
, 0x68D637FEE1 // ??7?h  [#17, Page: 68]
, 0x690D46F8A0 // ??F.i  [#17, Page: 69]
, 0x6ABBA4F422 // "???j  [#18, Page: 6A]
, 0x6B60D5F263 // c??`k  [#18, Page: 6B]
, 0x6C0D11EB67 // g?..l  [#18, Page: 6C]
, 0x6DD660ED26 // &?`?m  [#18, Page: 6D]
, 0x6E6082E1A4 // ???`n  [#18, Page: 6E]
, 0x6FBBF3E7E5 // ????o  [#19, Page: 6F]
, 0x7061708936 // 6?pap  [#19, Page: 70]
, 0x71BA018F77 // w?.?q  [#19, Page: 71]
, 0x720CE383F5 // ???.r  [#19, Page: 72]
, 0x73D79285B4 // ????s  [#19, Page: 73]
, 0x74BA569CB0 // ??V?t  [#20, Page: 74]
, 0x7561279AF1 // ??'au  [#20, Page: 75]
, 0x76D7C59673 // s???v  [#20, Page: 76]
, 0x770CB49032 // 2??.w  [#20, Page: 77]
, 0x780C4DA47B // {?M.x  [#20, Page: 78]
, 0x79D73CA23A // :?<?y  [#21, Page: 79]
, 0x7A61DEAEB8 // ???az  [#21, Page: 7A]
, 0x7BBAAFA8F9 // ????{  [#21, Page: 7B]
, 0x7CD76BB1FD // ??k?|  [#21, Page: 7C]
, 0x7D0C1AB7BC // ??..}  [#21, Page: 7D]
, 0x7EBAF8BB3E // >???~  [#22, Page: 7E]
, 0x7F6189BD7F // ???a?  [#22, Page: 7F]
, 0x806EE3CDA6 // ???n?  [#22, Page: 80]
, 0x81B592CBE7 // ?????  [#22, Page: 81]
, 0x820370C765 // e?p.?  [#22, Page: 82]
, 0x83D801C124 // $?.??  [#23, Page: 83]
, 0x84B5C5D820 // .????  [#23, Page: 84]
, 0x856EB4DE61 // a??n?  [#23, Page: 85]
, 0x86D856D2E3 // ??V??  [#23, Page: 86]
, 0x870327D4A2 // ??'.?  [#23, Page: 87]
, 0x8803DEE0EB // ???.?  [#24, Page: 88]
, 0x89D8AFE6AA // ?????  [#24, Page: 89]
, 0x8A6E4DEA28 // (?Mn?  [#24, Page: 8A]
, 0x8BB53CEC69 // i?<??  [#24, Page: 8B]
, 0x8CD8F8F56D // m????  [#24, Page: 8C]
, 0x8D0389F32C // ,??.?  [#25, Page: 8D]
, 0x8EB56BFFAE // ??k??  [#25, Page: 8E]
, 0x8F6E1AF9EF // ??.n?  [#25, Page: 8F]
, 0x90B499973C // <????  [#25, Page: 90]
, 0x916FE8917D // }??o?  [#25, Page: 91]
, 0x92D90A9DFF // ??.??  [#26, Page: 92]
, 0x93027B9BBE // ??{.?  [#26, Page: 93]
, 0x946FBF82BA // ???o?  [#26, Page: 94]
, 0x95B4CE84FB // ?????  [#26, Page: 95]
, 0x96022C8879 // y?,.?  [#26, Page: 96]
, 0x97D95D8E38 // 8?]??  [#27, Page: 97]
, 0x98D9A4BA71 // q????  [#27, Page: 98]
, 0x9902D5BC30 // 0??.?  [#27, Page: 99]
, 0x9AB437B0B2 // ??7??  [#27, Page: 9A]
, 0x9B6F46B6F3 // ??Fo?  [#27, Page: 9B]
, 0x9C0282AFF7 // ???.?  [#28, Page: 9C]
, 0x9DD9F3A9B6 // ?????  [#28, Page: 9D]
, 0x9E6F11A534 // 4?.o?  [#28, Page: 9E]
, 0x9FB460A375 // u?`??  [#28, Page: 9F]
, 0xA001667ED3 // ?~f.?  [#28, Page: A0]
, 0xA1DA177892 // ?x.??  [#29, Page: A1]
, 0xA26CF57410 // .t?l?  [#29, Page: A2]
, 0xA3B7847251 // Qr???  [#29, Page: A3]
, 0xA4DA406B55 // Uk@??  [#29, Page: A4]
, 0xA501316D14 // .m1.?  [#29, Page: A5]
, 0xA6B7D36196 // ?a???  [#30, Page: A6]
, 0xA76CA267D7 // ?g?l?  [#30, Page: A7]
, 0xA86C5B539E // ?S[l?  [#30, Page: A8]
, 0xA9B72A55DF // ?U*??  [#30, Page: A9]
, 0xAA01C8595D // ]Y?.?  [#30, Page: AA]
, 0xABDAB95F1C // ._???  [#31, Page: AB]
, 0xACB77D4618 // .F}??  [#31, Page: AC]
, 0xAD6C0C4059 // Y@.l?  [#31, Page: AD]
, 0xAEDAEE4CDB // ?L???  [#31, Page: AE]
, 0xAF019F4A9A // ?J?.?  [#31, Page: AF]
, 0xB0DB1C2449 // I$.??  [#32, Page: B0]
, 0xB1006D2208 // ."m.?  [#32, Page: B1]
, 0xB2B68F2E8A // ?.???  [#32, Page: B2]
, 0xB36DFE28CB // ?(?m?  [#32, Page: B3]
, 0xB4003A31CF // ?1:.?  [#32, Page: B4]
, 0xB5DB4B378E // ?7K??  [#33, Page: B5]
, 0xB66DA93B0C // .;?m?  [#33, Page: B6]
, 0xB7B6D83D4D // M=???  [#33, Page: B7]
, 0xB8B6210904 // ..!??  [#33, Page: B8]
, 0xB96D500F45 // E.Pm?  [#33, Page: B9]
, 0xBADBB203C7 // ?.???  [#34, Page: BA]
, 0xBB00C30586 // ?.?.?  [#34, Page: BB]
, 0xBC6D071C82 // ?..m?  [#34, Page: BC]
, 0xBDB6761AC3 // ?.v??  [#34, Page: BD]
, 0xBE00941641 // A.?.?  [#34, Page: BE]
, 0xBFDBE51000 // ..???  [#35, Page: BF]
, 0xC0B1E8AB4C // L????  [#35, Page: C0]
, 0xC16A99AD0D // .??j?  [#35, Page: C1]
, 0xC2DC7BA18F // ??{??  [#35, Page: C2]
, 0xC3070AA7CE // ??..?  [#35, Page: C3]
, 0xC46ACEBECA // ???j?  [#36, Page: C4]
, 0xC5B1BFB88B // ?????  [#36, Page: C5]
, 0xC6075DB409 // .?].?  [#36, Page: C6]
, 0xC7DC2CB248 // H?,??  [#36, Page: C7]
, 0xC8DCD58601 // .????  [#36, Page: C8]
, 0xC907A48040 // @??.?  [#37, Page: C9]
, 0xCAB1468CC2 // ??F??  [#37, Page: CA]
, 0xCB6A378A83 // ??7j?  [#37, Page: CB]
, 0xCC07F39387 // ???.?  [#37, Page: CC]
, 0xCDDC8295C6 // ?????  [#37, Page: CD]
, 0xCE6A609944 // D?`j?  [#38, Page: CE]
, 0xCFB1119F05 // .?.??  [#38, Page: CF]
, 0xD06B92F1D6 // ???k?  [#38, Page: D0]
, 0xD1B0E3F797 // ?????  [#38, Page: D1]
, 0xD20601FB15 // .?..?  [#38, Page: D2]
, 0xD3DD70FD54 // T?p??  [#39, Page: D3]
, 0xD4B0B4E450 // P????  [#39, Page: D4]
, 0xD56BC5E211 // .??k?  [#39, Page: D5]
, 0xD6DD27EE93 // ??'??  [#39, Page: D6]
, 0xD70656E8D2 // ??V.?  [#39, Page: D7]
, 0xD806AFDC9B // ???.?  [#40, Page: D8]
, 0xD9DDDEDADA // ?????  [#40, Page: D9]
, 0xDA6B3CD658 // X?<k?  [#40, Page: DA]
, 0xDBB04DD019 // .?M??  [#40, Page: DB]
, 0xDCDD89C91D // .????  [#40, Page: DC]
, 0xDD06F8CF5C // \??.?  [#41, Page: DD]
, 0xDEB01AC3DE // ??.??  [#41, Page: DE]
, 0xDF6B6BC59F // ??kk?  [#41, Page: DF]
, 0xE0DE6D1839 // 9.m??  [#41, Page: E0]
, 0xE1051C1E78 // x...?  [#41, Page: E1]
, 0xE2B3FE12FA // ?.???  [#42, Page: E2]
, 0xE3688F14BB // ?.?h?  [#42, Page: E3]
, 0xE4054B0DBF // ?.K.?  [#42, Page: E4]
, 0xE5DE3A0BFE // ?.:??  [#42, Page: E5]
, 0xE668D8077C // |.?h?  [#42, Page: E6]
, 0xE7B3A9013D // =.???  [#43, Page: E7]
, 0xE8B3503574 // t5P??  [#43, Page: E8]
, 0xE968213335 // 53!h?  [#43, Page: E9]
, 0xEADEC33FB7 // ?????  [#43, Page: EA]
, 0xEB05B239F6 // ?9?.?  [#43, Page: EB]
, 0xEC687620F2 // ?.vh?  [#44, Page: EC]
, 0xEDB30726B3 // ?&.??  [#44, Page: ED]
, 0xEE05E52A31 // 1*?.?  [#44, Page: EE]
, 0xEFDE942C70 // p,???  [#44, Page: EF]
, 0xF0041742A3 // ?B..?  [#44, Page: F0]
, 0xF1DF6644E2 // ?Df??  [#45, Page: F1]
, 0xF269844860 // `H?i?  [#45, Page: F2]
, 0xF3B2F54E21 // !N???  [#45, Page: F3]
, 0xF4DF315725 // %W1??  [#45, Page: F4]
, 0xF504405164 // dQ@.?  [#45, Page: F5]
, 0xF6B2A25DE6 // ?]???  [#46, Page: F6]
, 0xF769D35BA7 // ?[?i?  [#46, Page: F7]
, 0xF8692A6FEE // ?o*i?  [#46, Page: F8]
, 0xF9B25B69AF // ?i[??  [#46, Page: F9]
, 0xFA04B9652D // -e?.?  [#46, Page: FA]
, 0xFBDFC8636C // lc???  [#47, Page: FB]
, 0xFCB20C7A68 // hz.??  [#47, Page: FC]
, 0xFD697D7C29 // )|}i?  [#47, Page: FD]
, 0xFEDF9F70AB // ?p???  [#47, Page: FE]
, 0xFF04EE76EA // ?v?.?  [#47, Page: FF]
// Found 256 of length 5 in 00:02:30.510
        , 0
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

    // Verification
    //
    // This SO (Stack Overflow) mentions the following CRC32 input returns zero ...
    //   https://stackoverflow.com/questions/25573743/can-crc32c-ever-return-to-0
    //           Hex              Text    CRC32
    //   4-byte: AB 9B E0 9B      ????    0x73CBAF29
    //   5-byte: 44 59 42 7C 4F   DYB|O   0xCBA37AE9
    // .. but I was able to verify that these do NOT generate a CRC32 of zero.
    //
    // However, I WAS able to verify that this sequence of bytes...
    //                            Text    CRC32
    //   4-byte: 9D 0A D9 6D      ???m    0x00000000
    //   5-byte: 8D 8D 1A D2 2C   ????,   0x00000000
    //
    // ... DOES generate a CRC32 of zero. I also able to cross-verify with "On-line CRC calculation and free library".
    //
    // See:
    //   https://www.lammertbies.nl/comm/info/crc-calculation
    unsigned char data   [16]; //                   raw input bytes
             char keytext[16]; // "?????"           5 ascii chars
             char crctext[16]; // "00 00 00 00 00 " hexdump

    size_t entries = sizeof(lengths) / sizeof(uint64_t*);
    for (size_t length = 0; length < entries; length++)
    {
        printf( "- %d -\n", (int)length );
        uint64_t *keys = lengths[ length ];
        if (keys)
            for (size_t entry = 0; true; entry++)
            {
                uint64_t key = keys[ entry ];
                if (!key) break;

                SetString( key, length, data );
                uint32_t crc = crc32_reverse( length, data );

                //sprintf( (char*)crctext, "%08X", val );
                Hexdump  ( key , length, keytext );
                Printable( data, length, crctext );
                printf( "CRC32( 0x%010llX ) = %08X // %s = %s", key, crc, crctext, keytext );
                if (crc != 0)
                    printf( " ERROR: CRC32 not 0!" );
                printf( "\n" );
            }
    }

    return 0;
}

// ========================================================================
int main(int nArg, char *aArg[])
{
    common_init( false );

    bool bSearchLength1 = false;
    bool bSearchLength2 = false;
    bool bSearchLength3 = false;
    bool bSearchLength4 = false;
    bool bSearchLength5 = false;

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
                if (*pArg == '1') bSearchLength1 = true; else
                if (*pArg == '2') bSearchLength2 = true; else
                if (*pArg == '3') bSearchLength3 = true; else
                if (*pArg == '4') bSearchLength4 = true; else
                if (*pArg == '5') bSearchLength5 = true; else

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
                if (*pArg == 'v') return Verify();
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

    printf( "Using: %u / %u threads\n", gnThreadsActive, gnThreadsMaximum );
    printf( "Searching for CRC32() zero hash...\n" );

    if( bSearchLength1 ) Measure( SearchLen1 );
    if( bSearchLength2 ) Measure( SearchLen2 );
    if( bSearchLength3 ) Measure( SearchLen3 );
    if( bSearchLength4 ) Measure( SearchLen4 );
    if( bSearchLength5 ) Measure( SearchLen5 );

    printf( "Done.\n" );

    return 0;
}

