# CRC32 Demystified
A.K.A. Why does almost everyone bugger up CRC32 and how to fix it.

Version 1, February 2, 2017
By Michaelangel007

# Table of Contents

* Introduction
* Checksum
* CRC32 Implementation
  * Formulaic CRC
  * Table-Lookup CRC
     * CRC32 Tables
     * All CRC32 Permutations
* TL:DR; CRC32 Summary
* CRC32 Hashing Confusion
  * Bad Code
  * Why two forms?
  * Bad Data
* CRC32 or CRC33?



# Introduction

How many different ways can one implement a
[Cyclic redundancy check](https://en.wikipedia.org/wiki/Cyclic_redundancy_check)
algorithm? Specifically, where the polynomial is 32-bits, aka CRC32?

Let me count the ways.

The CRC algorithm can be implemented in one of two general methods:

* Formulaic
* Table-Lookup

For each of these methods there are various options we can choose from:

First, for each CRC we can use one of two polynomials:

* a "forward" polynomial constant, or
* a "reverse" polynomial constant, that is, the forward polynomial bit reversed

Second, when we initialize the table we can shift bits to the left or right.

Third, when we calculate the CRC value we can shift bits to left or right.
Ross Williams mentions in his guide:
[A PAINLESS GUIDE TO CRC ERROR DETECTION ALGORITHMS](http://www.ross.net/crc/download/crc_v3.txt)

> "There are really only two forms: normal and reflected. Normal
> shifts to the left and covers the case of algorithms with Refin=FALSE
> and Refot=FALSE. Reflected shifts to the right and covers algorithms
> with both those parameters true."

And gives these two formulas:

```c
    Normal:  crc = table[ ((crc>>24) ^ *data++) & 0xFF ] ^ (crc << 8);
    Reflect: crc = table[ (crc       ^ *data++) & 0xFF ] ^ (crc >> 8);
```

Note: that while the latter "reflected" formula is correct,
_the former "normal" formula is incorrect_ -- one needs to reverse **both** the:

* data bits, and
* final CRC value.

That gives rise to the Forth and Fifth options, respectively.
The fourth -- as we read each byte of the data we are calculating the CRC for
we optionally bit-reverse the the bytes as they get applied into the CRC algorithm.
The fifth -- we optionally bit-reverse the final CRC value.

What this all means is that depending on:

* which form (Normal or Reflected) and
* which polynomial (Forward or Reverse)

is used _a naive user won't realize there _at least_ a total of **4 different outcomes!**_

 Two of them are correct; the other two are incorrect.

 * Normal    form (Shift Left)  with Forward polynomial (valid)
 * Reflected form (Shift Right) with Forward polynomial
 * Normal    form (Shift Left)  with Reverse polynomial
 * Reflected form (Shift Right) with Reverse polynomial (valid)


# Checksum

Ross also mentions a _checksum_ called "CHECK":

> CHECK:  ... The field contains the checksum obtained when the
> ASCII string "123456789" is fed through the specified algorithm
> (i.e. 313233... (hexadecimal)).

For CRC32 a popular forward polynomial is `0x04C11DB7.` Reversing the bits
we get the reference polynomial of `0xEDB88320`.

|Polynomial| Binary                                |
|---------:|--------------------------------------:|
|0x04C11DB7| `00000100_11000001_00011101_10110111` |
|0xEDB88320| `11101101-10111000_10000011_00100000` |

(Note: See CRC32 vs CRC33)


They generate this _checksum:_

|Form     |Polynomial| CRC32 checksum |
|:--------|:---------|:------------|
|Normal   |0x04C11DB7| 0xCBF43926  |
|Reflected|0xEDB88320| 0xCBF43926  |

Notice how the **two checksum values are the same** even though the
polynomials are different!  We'll examine why that is in a _bit_ (**groan**)
but for now we can utilize this knowledge to verify that we have 
implemented the CRC32 algorithm correctly regardless if we are using:

 * a formulaic method,
 * a table-lookup method,
 * a normal polynomial, or
 * a reflected polynomial.

These are not the only CRC32 polynomials used.
Wikipedia has a table of [popular CRC polynomials](https://en.wikipedia.org/wiki/Cyclic_redundancy_check#table)
For example, if you enter in "123456789" in password searching utilities:

* https://decryptpassword.com/encrypt/
* https://www.integers.co/questions-answers/what-are-the-different-hash-algorithm-outputs-for-123456789.html
* http://php.net/manual/en/function.hash-file.php

They will list:

```
Encrypting 123456789 with CRC32: 181989fc
Encrypting 123456789 with CRC32B: cbf43926
```

**NOTE:** To prevent ambiguity I'll call the former `CRC32A`.

The former `CRC32A` is the ITU I.363.5 algorithm popularised by BZIP2.
The latter `CRC32B` is the ITU V.42 algorithm used in Ethernet and popularised by PKZip)

Why are two different values when **both** of these are
generated with the _same_ CRC32 polynomial 0x04C11DB7 on the same input?
We're getting ahead of ourselves but they can be summarize like this:

|Polynomial|Shift| Reverse Data|Reverse CRC|Checksum  | Name   |
|:---------|:---:|:-----------:|:---------:|:---------|-------:|
|0x04C11DB7|Left | No          |No         |0xFC891918| crc32a |
|0x04C11DB7|Left | Yes         |Yes        |0xCBF43926| crc32b |
|0xEDB88320|Right| No          |No         |0xCBF43926| crc32b |
|0xEDB88320|Right| Yes         |Yes        |0xFC891918| crc32a |

You can see [enum_crc32.cpp](src/enum_crc32.cpp) for more details.

For this document we will focus mainly on CRC32B.

On *nix machines we can use the built-in `cksum` utility.

```bash
    echo -n "123456789" > crctest.txt
    cksum -o 3 crctest.txt
```

Produces this output:

```
    3421780262 9 crctest.txt
```

Converting to hex via [Basic Calculator](https://en.wikipedia.org/wiki/Bc_(programming_language\))

```bash
     echo "obase=16; 3421780262;" | bc
```

```
CBF43926
```

Which matches the expected checksum value.


# CRC32 Implementation

So how do we actually _implement_ CRC32?

* Initialize the CRC value, typically zero, but by convention we usually invert the bits, that is, -1.
* Iterate over each byte we wish to calculate the CRC checksum for
  * For each byte we exclusive-or (XOR) it with the current CRC one bit at a time
* By convention for the final CRC value we invert the bits

Sounds simple, right?

It is, except for some minor, but important implementation details:

* When we XOR the data byte into the current CRC value do we start at the top or bottom bits?
* Which direction do we shift the CRC bits?
* How do we convert this formula into a table where we handle all 8-bits at once?

We'll start with the formulaic version.


## Formulaic CRC

If we are using a formulaic method, we can generalize the 4 permutations with 2 algorithms:

* a) Formulaic "Normal" CRC32

```C++
    // ========================================================================
    uint32_t crc32_formula_normal( size_t len, const void *data, const uint32_t POLY = 0x04C11DB7 )
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
```

 Notes:

  * The `reverse` is a table of byte values bit-reversed.
  * Likewise `reflect32()` bit reverses a 32-bit value.

 There are few key things to note:

 * `Normal` = Shift Left
 * We reverse bytes in the buffer before XOR'ing them into the CRC value
 * The data bytes are fed into the **top** 8 bits of the CRC value
 * We test the _top-bit_ of the CRC value
 * We invert the final CRC value
 * We bit reverse the final CRC value

 What were to happen if we _didn't_ reverse any of the bits?

```C++
    // ========================================================================
    uint32_t crc32_formula_normal_noreverse( size_t len, const void *data, const uint32_t POLY = 0x04C11DB7 )
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
```

 Running it on "123456789" produces the CRC32 value of 0xFC891918 -- notice
 how this is the little endian form of the big endian value 0x181989FC mentioned above! 
 We would have `CRC32A`.

* b) Formulaic "Reflected" CRC32

  If we want to remove all that bit-reversing shenanigans we end up the simpler version:

```C++
    // ========================================================================
    uint32_t crc32_formula_reflect( size_t len, const void *data, const uint32_t POLY = 0xEDB88320 )
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
```

 The areas to note are:

 * `Reflected` = Shift Right
 * The data bytes are fed into the **bottom** 8 bits of the CRC value
 * We test the _bottom-bit_ of the CRC value
 * We invert the final CRC value

What would happen if we mis-matched the form and the polynomial?
Using the check string this produces the formulaic output of:

```
   Normal   ( 0x04C11DB7 ): 0xCBF43926
   Reflected( 0x04C11DB7 ): 0xFC4F2BE9 invalid!
   Normal   ( 0xEDB88320 ): 0xFC4F2BE9 invalid!
   Reflected( 0xEDB88320 ): 0xCBF43926
```

 We can now understand _why_ often times you'll find people posting on the internet
 that they are using a (forward polynomial) value of `0x04C11DB7` and are asking for
 help to understand why they aren't getting the correct CRC calculation.
 The typical knee-jerk solution is someone tells the original parent to use the other
 (reverse) polynomial, `0x0xEDB88320`,
 _without both parties understanding that **both polynomials are correct!**_

 The _real_ problem is a MISMATCH of the bit shuffling used in the
 CRC32 table Initialization _and_ CRC32 Calculation.


## Table-Lookup CRC

 There is one big annoyance with the formulaic approach:

 * We need to test the CRC one bit a time

 Can we test 8 bits at once? Yes, by using a pre-computed table.
 The table lookup approach the CRC algorithm is broken down into two phases:

 * Initialization of the table
 * Calculation of crc using the table

 As we saw in the formulaic discussion above there are 4 different permutions.

 * Normal    form (Shift Left)  with Forward polynomial (valid)
 * Reflected form (Shift Right) with Forward polynomial
 * Normal    form (Shift Left)  with Reverse polynomial
 * Reflected form (Shift Right) with Reverse polynomial (valid)

 The calculation phase of table-lookup adds 8 more permutations.

| Phase          | Permutations |
|:---------------|-------------:|
| Initialization |          2^2 |
| Calculation    |          2^3 |

 Thus there are a total of 2^2 * 2^3 = 4 * 8 = 32 permutations.

 Of these 32 different permutations only 4 are correct, or should I say _standardized._
 The _other 28 are broken implementations_ due to someone not understanding
 the theory and incorrectly applying it.

 No wonder most people are confused about CRC32!  It is _so_ easy to go wrong!!

 Here are the details:

  a) How can the table _initialization_ be implemented in 4 different ways?

| Form      | Bit Check  | Rotate | Polynomial | Valid | Function                             |
|:----------|:-----------|:-------|:----------:|:------|:-------------------------------------|
| Normal    | Top Bit    | Left   | 0x04C11DB7 | Yes   | `crc32_init_normal ( forward poly )` |
| Reflected | Bottom Bit | Right  | 0x04C11DB7 | no !! | `crc32_init_reflect( forward poly )` |
| Normal    | Top Bit    | Left   | 0xEDB88320 | no !! | `crc32_init_normal ( reverse poly )` |
| Reflected | Bottom Bit | Right  | 0xEDB88320 | Yes   | `crc32_init_reflect( reverse poly )` |

### CRC32 Tables

Here are the 4 CRC32 tables:

* Normal    0x04C11DB7: &<<31 [  1] = poly     , [ 16] =     poly << 8 VALID 

```
    00000000, 04C11DB7, 09823B6E, 0D4326D9, 130476DC, 17C56B6B, 1A864DB2, 1E475005,  //   0 [0x00 .. 0x07]
    2608EDB8, 22C9F00F, 2F8AD6D6, 2B4BCB61, 350C9B64, 31CD86D3, 3C8EA00A, 384FBDBD,  //   8 [0x08 .. 0x0F]
    4C11DB70, 48D0C6C7, 4593E01E, 4152FDA9, 5F15ADAC, 5BD4B01B, 569796C2, 52568B75,  //  16 [0x10 .. 0x17]
    6A1936C8, 6ED82B7F, 639B0DA6, 675A1011, 791D4014, 7DDC5DA3, 709F7B7A, 745E66CD,  //  24 [0x18 .. 0x1F]
    9823B6E0, 9CE2AB57, 91A18D8E, 95609039, 8B27C03C, 8FE6DD8B, 82A5FB52, 8664E6E5,  //  32 [0x20 .. 0x27]
    BE2B5B58, BAEA46EF, B7A96036, B3687D81, AD2F2D84, A9EE3033, A4AD16EA, A06C0B5D,  //  40 [0x28 .. 0x2F]
    D4326D90, D0F37027, DDB056FE, D9714B49, C7361B4C, C3F706FB, CEB42022, CA753D95,  //  48 [0x30 .. 0x37]
    F23A8028, F6FB9D9F, FBB8BB46, FF79A6F1, E13EF6F4, E5FFEB43, E8BCCD9A, EC7DD02D,  //  56 [0x38 .. 0x3F]
    34867077, 30476DC0, 3D044B19, 39C556AE, 278206AB, 23431B1C, 2E003DC5, 2AC12072,  //  64 [0x40 .. 0x47]
    128E9DCF, 164F8078, 1B0CA6A1, 1FCDBB16, 018AEB13, 054BF6A4, 0808D07D, 0CC9CDCA,  //  72 [0x48 .. 0x4F]
    7897AB07, 7C56B6B0, 71159069, 75D48DDE, 6B93DDDB, 6F52C06C, 6211E6B5, 66D0FB02,  //  80 [0x50 .. 0x57]
    5E9F46BF, 5A5E5B08, 571D7DD1, 53DC6066, 4D9B3063, 495A2DD4, 44190B0D, 40D816BA,  //  88 [0x58 .. 0x5F]
    ACA5C697, A864DB20, A527FDF9, A1E6E04E, BFA1B04B, BB60ADFC, B6238B25, B2E29692,  //  96 [0x60 .. 0x67]
    8AAD2B2F, 8E6C3698, 832F1041, 87EE0DF6, 99A95DF3, 9D684044, 902B669D, 94EA7B2A,  // 104 [0x68 .. 0x6F]
    E0B41DE7, E4750050, E9362689, EDF73B3E, F3B06B3B, F771768C, FA325055, FEF34DE2,  // 112 [0x70 .. 0x77]
    C6BCF05F, C27DEDE8, CF3ECB31, CBFFD686, D5B88683, D1799B34, DC3ABDED, D8FBA05A,  // 120 [0x78 .. 0x7F]
    690CE0EE, 6DCDFD59, 608EDB80, 644FC637, 7A089632, 7EC98B85, 738AAD5C, 774BB0EB,  // 128 [0x80 .. 0x87]
    4F040D56, 4BC510E1, 46863638, 42472B8F, 5C007B8A, 58C1663D, 558240E4, 51435D53,  // 136 [0x88 .. 0x8F]
    251D3B9E, 21DC2629, 2C9F00F0, 285E1D47, 36194D42, 32D850F5, 3F9B762C, 3B5A6B9B,  // 144 [0x90 .. 0x97]
    0315D626, 07D4CB91, 0A97ED48, 0E56F0FF, 1011A0FA, 14D0BD4D, 19939B94, 1D528623,  // 152 [0x98 .. 0x9F]
    F12F560E, F5EE4BB9, F8AD6D60, FC6C70D7, E22B20D2, E6EA3D65, EBA91BBC, EF68060B,  // 160 [0xA0 .. 0xA7]
    D727BBB6, D3E6A601, DEA580D8, DA649D6F, C423CD6A, C0E2D0DD, CDA1F604, C960EBB3,  // 168 [0xA8 .. 0xAF]
    BD3E8D7E, B9FF90C9, B4BCB610, B07DABA7, AE3AFBA2, AAFBE615, A7B8C0CC, A379DD7B,  // 176 [0xB0 .. 0xB7]
    9B3660C6, 9FF77D71, 92B45BA8, 9675461F, 8832161A, 8CF30BAD, 81B02D74, 857130C3,  // 184 [0xB8 .. 0xBF]
    5D8A9099, 594B8D2E, 5408ABF7, 50C9B640, 4E8EE645, 4A4FFBF2, 470CDD2B, 43CDC09C,  // 192 [0xC0 .. 0xC7]
    7B827D21, 7F436096, 7200464F, 76C15BF8, 68860BFD, 6C47164A, 61043093, 65C52D24,  // 200 [0xC8 .. 0xCF]
    119B4BE9, 155A565E, 18197087, 1CD86D30, 029F3D35, 065E2082, 0B1D065B, 0FDC1BEC,  // 208 [0xD0 .. 0xD7]
    3793A651, 3352BBE6, 3E119D3F, 3AD08088, 2497D08D, 2056CD3A, 2D15EBE3, 29D4F654,  // 216 [0xD8 .. 0xDF]
    C5A92679, C1683BCE, CC2B1D17, C8EA00A0, D6AD50A5, D26C4D12, DF2F6BCB, DBEE767C,  // 224 [0xE0 .. 0xE7]
    E3A1CBC1, E760D676, EA23F0AF, EEE2ED18, F0A5BD1D, F464A0AA, F9278673, FDE69BC4,  // 232 [0xE8 .. 0xEF]
    89B8FD09, 8D79E0BE, 803AC667, 84FBDBD0, 9ABC8BD5, 9E7D9662, 933EB0BB, 97FFAD0C,  // 240 [0xF0 .. 0xF7]
    AFB010B1, AB710D06, A6322BDF, A2F33668, BCB4666D, B8757BDA, B5365D03, B1F740B4,  // 248 [0xF8 .. 0xFF]
```

* Reflected 0x04C11DB7: &1 >> [  1] = rev. poly, [ 30] = rev.poly << 8 broken

```
    00000000, 06233697, 05C45641, 03E760D6, 020A97ED, 0429A17A, 07CEC1AC, 01EDF73B,  //   0 [0x00 .. 0x07]
    04152FDA, 0236194D, 01D1799B, 07F24F0C, 061FB837, 003C8EA0, 03DBEE76, 05F8D8E1,  //   8 [0x08 .. 0x0F]
    01A864DB, 078B524C, 046C329A, 024F040D, 03A2F336, 0581C5A1, 0666A577, 004593E0,  //  16 [0x10 .. 0x17]
    05BD4B01, 039E7D96, 00791D40, 065A2BD7, 07B7DCEC, 0194EA7B, 02738AAD, 0450BC3A,  //  24 [0x18 .. 0x1F]
    0350C9B6, 0573FF21, 06949FF7, 00B7A960, 015A5E5B, 077968CC, 049E081A, 02BD3E8D,  //  32 [0x20 .. 0x27]
    0745E66C, 0166D0FB, 0281B02D, 04A286BA, 054F7181, 036C4716, 008B27C0, 06A81157,  //  40 [0x28 .. 0x2F]
    02F8AD6D, 04DB9BFA, 073CFB2C, 011FCDBB, 00F23A80, 06D10C17, 05366CC1, 03155A56,  //  48 [0x30 .. 0x37]
    06ED82B7, 00CEB420, 0329D4F6, 050AE261, 04E7155A, 02C423CD, 0123431B, 0700758C,  //  56 [0x38 .. 0x3F]
    06A1936C, 0082A5FB, 0365C52D, 0546F3BA, 04AB0481, 02883216, 016F52C0, 074C6457,  //  64 [0x40 .. 0x47]
    02B4BCB6, 04978A21, 0770EAF7, 0153DC60, 00BE2B5B, 069D1DCC, 057A7D1A, 03594B8D,  //  72 [0x48 .. 0x4F]
    0709F7B7, 012AC120, 02CDA1F6, 04EE9761, 0503605A, 032056CD, 00C7361B, 06E4008C,  //  80 [0x50 .. 0x57]
    031CD86D, 053FEEFA, 06D88E2C, 00FBB8BB, 01164F80, 07357917, 04D219C1, 02F12F56,  //  88 [0x58 .. 0x5F]
    05F15ADA, 03D26C4D, 00350C9B, 06163A0C, 07FBCD37, 01D8FBA0, 023F9B76, 041CADE1,  //  96 [0x60 .. 0x67]
    01E47500, 07C74397, 04202341, 020315D6, 03EEE2ED, 05CDD47A, 062AB4AC, 0009823B,  // 104 [0x68 .. 0x6F]
    04593E01, 027A0896, 019D6840, 07BE5ED7, 0653A9EC, 00709F7B, 0397FFAD, 05B4C93A,  // 112 [0x70 .. 0x77]
    004C11DB, 066F274C, 0588479A, 03AB710D, 02468636, 0465B0A1, 0782D077, 01A1E6E0,  // 120 [0x78 .. 0x7F]
    04C11DB7, 02E22B20, 01054BF6, 07267D61, 06CB8A5A, 00E8BCCD, 030FDC1B, 052CEA8C,  // 128 [0x80 .. 0x87]
    00D4326D, 06F704FA, 0510642C, 033352BB, 02DEA580, 04FD9317, 071AF3C1, 0139C556,  // 136 [0x88 .. 0x8F]
    0569796C, 034A4FFB, 00AD2F2D, 068E19BA, 0763EE81, 0140D816, 02A7B8C0, 04848E57,  // 144 [0x90 .. 0x97]
    017C56B6, 075F6021, 04B800F7, 029B3660, 0376C15B, 0555F7CC, 06B2971A, 0091A18D,  // 152 [0x98 .. 0x9F]
    0791D401, 01B2E296, 02558240, 0476B4D7, 059B43EC, 03B8757B, 005F15AD, 067C233A,  // 160 [0xA0 .. 0xA7]
    0384FBDB, 05A7CD4C, 0640AD9A, 00639B0D, 018E6C36, 07AD5AA1, 044A3A77, 02690CE0,  // 168 [0xA8 .. 0xAF]
    0639B0DA, 001A864D, 03FDE69B, 05DED00C, 04332737, 021011A0, 01F77176, 07D447E1,  // 176 [0xB0 .. 0xB7]
    022C9F00, 040FA997, 07E8C941, 01CBFFD6, 002608ED, 06053E7A, 05E25EAC, 03C1683B,  // 184 [0xB8 .. 0xBF]
    02608EDB, 0443B84C, 07A4D89A, 0187EE0D, 006A1936, 06492FA1, 05AE4F77, 038D79E0,  // 192 [0xC0 .. 0xC7]
    0675A101, 00569796, 03B1F740, 0592C1D7, 047F36EC, 025C007B, 01BB60AD, 0798563A,  // 200 [0xC8 .. 0xCF]
    03C8EA00, 05EBDC97, 060CBC41, 002F8AD6, 01C27DED, 07E14B7A, 04062BAC, 02251D3B,  // 208 [0xD0 .. 0xD7]
    07DDC5DA, 01FEF34D, 0219939B, 043AA50C, 05D75237, 03F464A0, 00130476, 063032E1,  // 216 [0xD8 .. 0xDF]
    0130476D, 071371FA, 04F4112C, 02D727BB, 033AD080, 0519E617, 06FE86C1, 00DDB056,  // 224 [0xE0 .. 0xE7]
    052568B7, 03065E20, 00E13EF6, 06C20861, 072FFF5A, 010CC9CD, 02EBA91B, 04C89F8C,  // 232 [0xE8 .. 0xEF]
    009823B6, 06BB1521, 055C75F7, 037F4360, 0292B45B, 04B182CC, 0756E21A, 0175D48D,  // 240 [0xF0 .. 0xF7]
    048D0C6C, 02AE3AFB, 01495A2D, 076A6CBA, 06879B81, 00A4AD16, 0343CDC0, 0560FB57,  // 248 [0xF8 .. 0xFF]
```

* Normal    0xEDB88320: &<<31 [128] = rev. poly, [120] = rev.poly >> 8 broken

```
    00000000, EDB88320, 36C98560, DB710640, 6D930AC0, 802B89E0, 5B5A8FA0, B6E20C80,  //   0 [0x00 .. 0x07]
    DB261580, 369E96A0, EDEF90E0, 005713C0, B6B51F40, 5B0D9C60, 807C9A20, 6DC41900,  //   8 [0x08 .. 0x0F]
    5BF4A820, B64C2B00, 6D3D2D40, 8085AE60, 3667A2E0, DBDF21C0, 00AE2780, ED16A4A0,  //  16 [0x10 .. 0x17]
    80D2BDA0, 6D6A3E80, B61B38C0, 5BA3BBE0, ED41B760, 00F93440, DB883200, 3630B120,  //  24 [0x18 .. 0x1F]
    B7E95040, 5A51D360, 8120D520, 6C985600, DA7A5A80, 37C2D9A0, ECB3DFE0, 010B5CC0,  //  32 [0x20 .. 0x27]
    6CCF45C0, 8177C6E0, 5A06C0A0, B7BE4380, 015C4F00, ECE4CC20, 3795CA60, DA2D4940,  //  40 [0x28 .. 0x2F]
    EC1DF860, 01A57B40, DAD47D00, 376CFE20, 818EF2A0, 6C367180, B74777C0, 5AFFF4E0,  //  48 [0x30 .. 0x37]
    373BEDE0, DA836EC0, 01F26880, EC4AEBA0, 5AA8E720, B7106400, 6C616240, 81D9E160,  //  56 [0x38 .. 0x3F]
    826A23A0, 6FD2A080, B4A3A6C0, 591B25E0, EFF92960, 0241AA40, D930AC00, 34882F20,  //  64 [0x40 .. 0x47]
    594C3620, B4F4B500, 6F85B340, 823D3060, 34DF3CE0, D967BFC0, 0216B980, EFAE3AA0,  //  72 [0x48 .. 0x4F]
    D99E8B80, 342608A0, EF570EE0, 02EF8DC0, B40D8140, 59B50260, 82C40420, 6F7C8700,  //  80 [0x50 .. 0x57]
    02B89E00, EF001D20, 34711B60, D9C99840, 6F2B94C0, 829317E0, 59E211A0, B45A9280,  //  88 [0x58 .. 0x5F]
    358373E0, D83BF0C0, 034AF680, EEF275A0, 58107920, B5A8FA00, 6ED9FC40, 83617F60,  //  96 [0x60 .. 0x67]
    EEA56660, 031DE540, D86CE300, 35D46020, 83366CA0, 6E8EEF80, B5FFE9C0, 58476AE0,  // 104 [0x68 .. 0x6F]
    6E77DBC0, 83CF58E0, 58BE5EA0, B506DD80, 03E4D100, EE5C5220, 352D5460, D895D740,  // 112 [0x70 .. 0x77]
    B551CE40, 58E94D60, 83984B20, 6E20C800, D8C2C480, 357A47A0, EE0B41E0, 03B3C2C0,  // 120 [0x78 .. 0x7F]
    E96CC460, 04D44740, DFA54100, 321DC220, 84FFCEA0, 69474D80, B2364BC0, 5F8EC8E0,  // 128 [0x80 .. 0x87]
    324AD1E0, DFF252C0, 04835480, E93BD7A0, 5FD9DB20, B2615800, 69105E40, 84A8DD60,  // 136 [0x88 .. 0x8F]
    B2986C40, 5F20EF60, 8451E920, 69E96A00, DF0B6680, 32B3E5A0, E9C2E3E0, 047A60C0,  // 144 [0x90 .. 0x97]
    69BE79C0, 8406FAE0, 5F77FCA0, B2CF7F80, 042D7300, E995F020, 32E4F660, DF5C7540,  // 152 [0x98 .. 0x9F]
    5E859420, B33D1700, 684C1140, 85F49260, 33169EE0, DEAE1DC0, 05DF1B80, E86798A0,  // 160 [0xA0 .. 0xA7]
    85A381A0, 681B0280, B36A04C0, 5ED287E0, E8308B60, 05880840, DEF90E00, 33418D20,  // 168 [0xA8 .. 0xAF]
    05713C00, E8C9BF20, 33B8B960, DE003A40, 68E236C0, 855AB5E0, 5E2BB3A0, B3933080,  // 176 [0xB0 .. 0xB7]
    DE572980, 33EFAAA0, E89EACE0, 05262FC0, B3C42340, 5E7CA060, 850DA620, 68B52500,  // 184 [0xB8 .. 0xBF]
    6B06E7C0, 86BE64E0, 5DCF62A0, B077E180, 0695ED00, EB2D6E20, 305C6860, DDE4EB40,  // 192 [0xC0 .. 0xC7]
    B020F240, 5D987160, 86E97720, 6B51F400, DDB3F880, 300B7BA0, EB7A7DE0, 06C2FEC0,  // 200 [0xC8 .. 0xCF]
    30F24FE0, DD4ACCC0, 063BCA80, EB8349A0, 5D614520, B0D9C600, 6BA8C040, 86104360,  // 208 [0xD0 .. 0xD7]
    EBD45A60, 066CD940, DD1DDF00, 30A55C20, 864750A0, 6BFFD380, B08ED5C0, 5D3656E0,  // 216 [0xD8 .. 0xDF]
    DCEFB780, 315734A0, EA2632E0, 079EB1C0, B17CBD40, 5CC43E60, 87B53820, 6A0DBB00,  // 224 [0xE0 .. 0xE7]
    07C9A200, EA712120, 31002760, DCB8A440, 6A5AA8C0, 87E22BE0, 5C932DA0, B12BAE80,  // 232 [0xE8 .. 0xEF]
    871B1FA0, 6AA39C80, B1D29AC0, 5C6A19E0, EA881560, 07309640, DC419000, 31F91320,  // 240 [0xF0 .. 0xF7]
    5C3D0A20, B1858900, 6AF48F40, 874C0C60, 31AE00E0, DC1683C0, 07678580, EADF06A0,  // 248 [0xF8 .. 0xFF]
```

* Reflected 0xEDB88320: &1 >> [128] = poly     , [  8] =     poly >> 8 VALID 

```
    00000000, 77073096, EE0E612C, 990951BA, 076DC419, 706AF48F, E963A535, 9E6495A3,  //   0 [0x00 .. 0x07]
    0EDB8832, 79DCB8A4, E0D5E91E, 97D2D988, 09B64C2B, 7EB17CBD, E7B82D07, 90BF1D91,  //   8 [0x08 .. 0x0F]
    1DB71064, 6AB020F2, F3B97148, 84BE41DE, 1ADAD47D, 6DDDE4EB, F4D4B551, 83D385C7,  //  16 [0x10 .. 0x17]
    136C9856, 646BA8C0, FD62F97A, 8A65C9EC, 14015C4F, 63066CD9, FA0F3D63, 8D080DF5,  //  24 [0x18 .. 0x1F]
    3B6E20C8, 4C69105E, D56041E4, A2677172, 3C03E4D1, 4B04D447, D20D85FD, A50AB56B,  //  32 [0x20 .. 0x27]
    35B5A8FA, 42B2986C, DBBBC9D6, ACBCF940, 32D86CE3, 45DF5C75, DCD60DCF, ABD13D59,  //  40 [0x28 .. 0x2F]
    26D930AC, 51DE003A, C8D75180, BFD06116, 21B4F4B5, 56B3C423, CFBA9599, B8BDA50F,  //  48 [0x30 .. 0x37]
    2802B89E, 5F058808, C60CD9B2, B10BE924, 2F6F7C87, 58684C11, C1611DAB, B6662D3D,  //  56 [0x38 .. 0x3F]
    76DC4190, 01DB7106, 98D220BC, EFD5102A, 71B18589, 06B6B51F, 9FBFE4A5, E8B8D433,  //  64 [0x40 .. 0x47]
    7807C9A2, 0F00F934, 9609A88E, E10E9818, 7F6A0DBB, 086D3D2D, 91646C97, E6635C01,  //  72 [0x48 .. 0x4F]
    6B6B51F4, 1C6C6162, 856530D8, F262004E, 6C0695ED, 1B01A57B, 8208F4C1, F50FC457,  //  80 [0x50 .. 0x57]
    65B0D9C6, 12B7E950, 8BBEB8EA, FCB9887C, 62DD1DDF, 15DA2D49, 8CD37CF3, FBD44C65,  //  88 [0x58 .. 0x5F]
    4DB26158, 3AB551CE, A3BC0074, D4BB30E2, 4ADFA541, 3DD895D7, A4D1C46D, D3D6F4FB,  //  96 [0x60 .. 0x67]
    4369E96A, 346ED9FC, AD678846, DA60B8D0, 44042D73, 33031DE5, AA0A4C5F, DD0D7CC9,  // 104 [0x68 .. 0x6F]
    5005713C, 270241AA, BE0B1010, C90C2086, 5768B525, 206F85B3, B966D409, CE61E49F,  // 112 [0x70 .. 0x77]
    5EDEF90E, 29D9C998, B0D09822, C7D7A8B4, 59B33D17, 2EB40D81, B7BD5C3B, C0BA6CAD,  // 120 [0x78 .. 0x7F]
    EDB88320, 9ABFB3B6, 03B6E20C, 74B1D29A, EAD54739, 9DD277AF, 04DB2615, 73DC1683,  // 128 [0x80 .. 0x87]
    E3630B12, 94643B84, 0D6D6A3E, 7A6A5AA8, E40ECF0B, 9309FF9D, 0A00AE27, 7D079EB1,  // 136 [0x88 .. 0x8F]
    F00F9344, 8708A3D2, 1E01F268, 6906C2FE, F762575D, 806567CB, 196C3671, 6E6B06E7,  // 144 [0x90 .. 0x97]
    FED41B76, 89D32BE0, 10DA7A5A, 67DD4ACC, F9B9DF6F, 8EBEEFF9, 17B7BE43, 60B08ED5,  // 152 [0x98 .. 0x9F]
    D6D6A3E8, A1D1937E, 38D8C2C4, 4FDFF252, D1BB67F1, A6BC5767, 3FB506DD, 48B2364B,  // 160 [0xA0 .. 0xA7]
    D80D2BDA, AF0A1B4C, 36034AF6, 41047A60, DF60EFC3, A867DF55, 316E8EEF, 4669BE79,  // 168 [0xA8 .. 0xAF]
    CB61B38C, BC66831A, 256FD2A0, 5268E236, CC0C7795, BB0B4703, 220216B9, 5505262F,  // 176 [0xB0 .. 0xB7]
    C5BA3BBE, B2BD0B28, 2BB45A92, 5CB36A04, C2D7FFA7, B5D0CF31, 2CD99E8B, 5BDEAE1D,  // 184 [0xB8 .. 0xBF]
    9B64C2B0, EC63F226, 756AA39C, 026D930A, 9C0906A9, EB0E363F, 72076785, 05005713,  // 192 [0xC0 .. 0xC7]
    95BF4A82, E2B87A14, 7BB12BAE, 0CB61B38, 92D28E9B, E5D5BE0D, 7CDCEFB7, 0BDBDF21,  // 200 [0xC8 .. 0xCF]
    86D3D2D4, F1D4E242, 68DDB3F8, 1FDA836E, 81BE16CD, F6B9265B, 6FB077E1, 18B74777,  // 208 [0xD0 .. 0xD7]
    88085AE6, FF0F6A70, 66063BCA, 11010B5C, 8F659EFF, F862AE69, 616BFFD3, 166CCF45,  // 216 [0xD8 .. 0xDF]
    A00AE278, D70DD2EE, 4E048354, 3903B3C2, A7672661, D06016F7, 4969474D, 3E6E77DB,  // 224 [0xE0 .. 0xE7]
    AED16A4A, D9D65ADC, 40DF0B66, 37D83BF0, A9BCAE53, DEBB9EC5, 47B2CF7F, 30B5FFE9,  // 232 [0xE8 .. 0xEF]
    BDBDF21C, CABAC28A, 53B39330, 24B4A3A6, BAD03605, CDD70693, 54DE5729, 23D967BF,  // 240 [0xF0 .. 0xF7]
    B3667A2E, C4614AB8, 5D681B02, 2A6F2B94, B40BBE37, C30C8EA1, 5A05DF1B, 2D02EF8D,  // 248 [0xF8 .. 0xFF]
```

 Thus by inspecting table[1] and table[128] entries we can tell:

  * Which form you used,
  * Which polynomial you used, and
  * Which shift direction you should be using.

  b) How can the CRC _calculation_ be implemented in 8 different ways?

| Shift CRC  | Data Bits reversed? | Final CRC Reversed? |
|:-----------|:--------------------|:--------------------|
| Left       | No                  | No                  |
| Right      | Yes                 | Yes                 |

 The data bits are reversed depending on which form we are in:

|Form     | Data bits                                                       |
|:--------|:----------------------------------------------------------------|
|Normal   | `crc32 = table[ (crc32 >> 24) ^ reverse[ *data ] ^ (crc << 8);` |
|Reflected| `crc32 = table[ (crc32      ) ^          *data ] ^ (crc >> 8);` |

 The final CRC value is reversed depending on which form we are in:

|Form     | Final CRC32                    |
|:--------|:-------------------------------|
|Normal   | `crc32 =            ~crc32  ;` |
|Reflected| `crc32 = reflect32( ~crc32 );` |

 Enumerating the 8 calculation permutations:

| Right Shift | Reverse<br>Data | Reverse<br>CRC | Function      |
|:-----------:|:---------------:|:--------------:|:--------------|
| 0           | 0               | 0              | `crc32_000()` |
| 0           | 0               | 1              | `crc32_001()` |
| 0           | 1               | 0              | `crc32_010()` |
| 0           | 1               | 1              | `crc32_011()` |
| 1           | 0               | 0              | `crc32_100()` |
| 1           | 0               | 1              | `crc32_101()` |
| 1           | 1               | 0              | `crc32_110()` |
| 1           | 1               | 1              | `crc32_111()` |

### All CRC32 Permutations

Enumerating all 32 initialization and calculation permutations:

* See [enum_crc32.cpp](enum_crc32.cpp)

|Polynomial|Bit| Init    |Shift<br>CRC>|Reverse<br>Data|Reverse<br>CRC| Function      |Valid|
|:---------|--:|--------------------|:----:|:----------:|:-----------:|:--------------|----:|
|0x04C11DB7| 31|`crc32_init_normal` |Left  | 0          | 0           | `crc32_000()` |crc32a|
|0x04C11DB7| 31|`crc32_init_normal` |Left  | 0          | 1           | `crc32_001()` |  no  |
|0x04C11DB7| 31|`crc32_init_normal` |Left  | 1          | 0           | `crc32_010()` |  no  |
|0x04C11DB7| 31|`crc32_init_normal` |Left  | 1          | 1           | `crc32_011()` |crc32b|
|0x04C11DB7| 31|`crc32_init_normal` |Right | 0          | 0           | `crc32_100()` |  no  |
|0x04C11DB7| 31|`crc32_init_normal` |Right | 0          | 1           | `crc32_101()` |  no  |
|0x04C11DB7| 31|`crc32_init_normal` |Right | 1          | 0           | `crc32_110()` |  no  |
|0x04C11DB7| 31|`crc32_init_normal` |Right | 1          | 1           | `crc32_111()` |  no  |
|0x04C11DB7|  1|`crc32_init_reflect`|Left  | 0          | 0           | `crc32_000()` |  no  |
|0x04C11DB7|  1|`crc32_init_reflect`|Left  | 0          | 1           | `crc32_001()` |  no  |
|0x04C11DB7|  1|`crc32_init_reflect`|Left  | 1          | 0           | `crc32_010()` |  no  |
|0x04C11DB7|  1|`crc32_init_reflect`|Left  | 1          | 1           | `crc32_011()` |  no  |
|0x04C11DB7|  1|`crc32_init_reflect`|Right | 0          | 0           | `crc32_100()` |  no  |
|0x04C11DB7|  1|`crc32_init_reflect`|Right | 0          | 1           | `crc32_101()` |  no  |
|0x04C11DB7|  1|`crc32_init_reflect`|Right | 1          | 0           | `crc32_110()` |  no  |
|0x04C11DB7|  1|`crc32_init_reflect`|Right | 1          | 1           | `crc32_111()` |  no  |
|0xEDB88320| 31|`crc32_init_normal` |Left  | 0          | 0           | `crc32_000()` |  no  |
|0xEDB88320| 31|`crc32_init_normal` |Left  | 0          | 1           | `crc32_001()` |  no  |
|0xEDB88320| 31|`crc32_init_normal` |Left  | 1          | 0           | `crc32_010()` |  no  |
|0xEDB88320| 31|`crc32_init_normal` |Left  | 1          | 1           | `crc32_011()` |  no  |
|0xEDB88320| 31|`crc32_init_normal` |Right | 0          | 0           | `crc32_100()` |  no  |
|0xEDB88320| 31|`crc32_init_normal` |Right | 0          | 1           | `crc32_101()` |  no  |
|0xEDB88320| 31|`crc32_init_normal` |Right | 1          | 0           | `crc32_110()` |  no  |
|0xEDB88320| 31|`crc32_init_normal` |Right | 1          | 1           | `crc32_111()` |  no  |
|0xEDB88320|  1|`crc32_init_reflect`|Left  | 0          | 0           | `crc32_000()` |  no  |
|0xEDB88320|  1|`crc32_init_reflect`|Left  | 0          | 1           | `crc32_001()` |  no  |
|0xEDB88320|  1|`crc32_init_reflect`|Left  | 1          | 0           | `crc32_010()` |  no  |
|0xEDB88320|  1|`crc32_init_reflect`|Left  | 1          | 1           | `crc32_011()` |  no  |
|0xEDB88320|  1|`crc32_init_reflect`|Right | 0          | 0           | `crc32_100()` |crc32b|
|0xEDB88320|  1|`crc32_init_reflect`|Right | 0          | 1           | `crc32_101()` |  no  |
|0xEDB88320|  1|`crc32_init_reflect`|Right | 1          | 0           | `crc32_110()` |  no  |
|0xEDB88320|  1|`crc32_init_reflect`|Right | 1          | 1           | `crc32_111()` |crc32a|

Legend:

 * Bit = Which bit is tested during table initialization
 * Shift CRC = Which direction the CRC value is shifted during calculation.
   Note that this is _independent_ of which shift direction was used during initialization!


# TL:DR; CRC32 Summary

Here is a summary of the two forms of CRC32:

| Description                     | Normal            | Reflected          |
|:--------------------------------|:-----------------:|:------------------:|
| Polynomial bits reversed?       | No                | Yes                |
| Polynomial value                | 0x04C11DB7        | 0xEDB88320         |
| Polynomial nomenclature         | Forward           | Reverse            |
| Table initialization bit        | Top-bit           | Low-bit            |
| Table initialization bit test   | 31                | 1                  |
| Table initialization bit shift  | Left              | Right              |
| Data bits reversed?             | Yes               | No                 |
| CRC calculation shift           | Left              | Right              |
| Final CRC bits reversed?        | Yes               | No                 |
| Correct Initialization Function |`crc32_init_normal`|`crc32_init_reflect`|
| Correct Calculation Function    |`crc32_011()`      |`crc32_100()`       |
| CRC32 Checksum "123456789"      |`0xCBF43926`       |`CBF43926`          |

# CRC32 Hashing Confusion

Due to _one_ person completely failing to understand CRC32 this misled
_other_ people to falsely conclude that CRC32 wouldn't make a good hash.

> CRC32 was never intended for hash table use. There is really no good reason to use it for this purpose, and I recommend that you avoid doing so.
> If you decide to use CRC32, it's critical that you use the hash bits from the end opposite to that in which the key octets are fed in. Which end this is depends on the specific CRC32 implementation. Do not treat CRC32 as a "black box" hash function, and do not use it as a general purpose hash. Be sure to test each application of it for suitability.

Ironically, Bret Mulvey implemented a _broken_ version of CRC32!

Original code:

```C#
public class CRC32 : HashFunction
{
    uint[] tab;

    public CRC32()
    {
        Init(0x04c11db7);
    }

    public CRC32(uint poly)
    {
        Init(poly);
    }

    void Init(uint poly)
    {
        tab = new uint[256];
        for (uint i=0; i<256; i++)
        {
            uint t = i;
            for (int j=0; j<8; j++)
                if ((t & 1) == 0)
                    t >>= 1;
                else
                    t = (t >> 1) ^ poly;
            tab[i] = t;
        }
    }

    public override uint ComputeHash(byte[] data)
    {
        uint hash = 0xFFFFFFFF;
        foreach (byte b in data)
            hash = (hash << 8) 
                ^ tab[b ^ (hash >> 24)];
        return ~hash;
    }
}
```

While Bret's original hashing page is now dead ...

* http://home.comcast.net/~bretm/hash/8.html

... Fortunately there are mirrors available:

* https://web.archive.org/web/20130420172816/http://home.comcast.net/~bretm/hash/8.html

NOTE: Bret has a new page but _he siliently omits CRC32_ due to his misunderstanding of CRC32.

* http://papa.bretmulvey.com/post/124027987928/hash-functions

This Stack Overflow question:

* http://stackoverflow.com/questions/10953958/can-crc32-be-used-as-a-hash-function

mentions Bret didn't implement CRC32 correctly,
but no one actually says WHAT the bug(s) are!

We can inspect the:

* code
* data

to determine _where_ Bret made his bugs.


## Bad Code

Bret mismatched the table _initialization_ with the CRC _calculation._

1. Incorrect table initialization.

 Pardon the pun, but to re-hash:

 CRC32 has two _polynomials_:

  * The _forward_ polynomial, `0x04C11DB7`,
  * The _reverse_ polynomial, `0xEDB88320`, where the bits are reversed.

 The CRC algorith comes in two **forms**:

   * Normal initialization checks the top bit and shifts left,
   * Reflected initialiation checks the bottom bit and shifts right.

 Bret's `Init()` used a _forward_ polynomial mismatched with a 'reflected' bottom bit initialization.


2. Incorrect CRC calculation

  Bret's `ComputeHash()` shifts left HOWEVER he didn't reverse the bits in both the data and final CRC value!

If we enumerate the possible ways someone could initialize the table with the
forward polynomial 0x04C11DB7
we discover there are 8 crc values on the text "123456789". **NONE of these are correct!**

```
Reflected 0x04C11DB7: &1 >> [  1] = rev. poly, [ 30] = rev.poly << 8 broken
   Shift: Left , Rev. Data: 0, Rev. CRC: 0, 0xC9A0B7E5  no
   Shift: Left , Rev. Data: 0, Rev. CRC: 1, 0xA7ED0593  no
   Shift: Left , Rev. Data: 1, Rev. CRC: 0, 0x9D594C04  no
   Shift: Left , Rev. Data: 1, Rev. CRC: 1, 0x20329AB9  no
   Shift: Right, Rev. Data: 0, Rev. CRC: 0, 0xFC4F2BE9  no
   Shift: Right, Rev. Data: 0, Rev. CRC: 1, 0x97D4F23F  no
   Shift: Right, Rev. Data: 1, Rev. CRC: 0, 0xFDEFB72E  no
   Shift: Right, Rev. Data: 1, Rev. CRC: 1, 0x74EDF7BF  no
```

Why is that?

## Why two forms?

You may be wondering why are there even two forms in the first place?

Back in the day when CRC was being first designed / implemented
the hardware guys would shift bits out right-to-left using
a barrel shifter.

When CRC was implemented in software someone noticed all this
bit-reversal was unneccessary if they:

* reversed the polynomial
* reversed the reversed bytes
* reversed the reversed final CRC value

If we output a trace of _how_ the crc32 is calculated using the tables ...

* Proper table for 0x04C11DB7
* Proper table for 0xEDB88320

... we get this output:


```c
    ========== Bytes: 9 ==========
    [ 31, 32, 33, 34, 35, 36, 37, 38, 39, ]

    ---------- Normal  ----------
    crc32=FFFFFFFF
    ^buf[0000]: 31 -> 8C bits reversed
       = crc32[ 73 ]: 07BE5ED7 ^ FFFFFF__
    crc32=1208C43E
    ^buf[0001]: 32 -> 4C bits reversed
       = crc32[ 5E ]: 04D219C1 ^ 08C43E__
    crc32=4CDD350D
    ^buf[0002]: 33 -> CC bits reversed
       = crc32[ 80 ]: 04C11DB7 ^ DD350D__
    crc32=B439EDEE
    ^buf[0003]: 34 -> 2C bits reversed
       = crc32[ 98 ]: 017C56B6 ^ 39EDEE__
    crc32=3AF83826
    ^buf[0004]: 35 -> AC bits reversed
       = crc32[ 96 ]: 02A7B8C0 ^ F83826__
    crc32=C7A3502C
    ^buf[0005]: 36 -> 6C bits reversed
       = crc32[ AB ]: 00639B0D ^ A3502C__
    crc32=7934B16F
    ^buf[0006]: 37 -> EC bits reversed
       = crc32[ 95 ]: 0140D816 ^ 34B16F__
    crc32=06693FF5
    ^buf[0007]: 38 -> 1C bits reversed
       = crc32[ 1A ]: 00791D40 ^ 693FF5__
    crc32=0AA4F8A6
    ^buf[0008]: 39 -> 9C bits reversed
       = crc32[ 96 ]: 02A7B8C0 ^ A4F8A6__
    crc32=9B63D02C
        ~=649C2FD3
         =CBF43926 bits reversed
    pass

    ---------- Reflect ==========
    crc32=FFFFFFFF
    ^buf[0000]: 31
       = crc32[ CE ]: 61043093 ^ __FFFFFF
    crc32=7C231048
    ^buf[0001]: 32
       = crc32[ 7A ]: CF3ECB31 ^ __7C2310
    crc32=B0ACBB32
    ^buf[0002]: 33
       = crc32[ 01 ]: 04C11DB7 ^ __B0ACBB
    crc32=77B79C2D
    ^buf[0003]: 34
       = crc32[ 19 ]: 6ED82B7F ^ __77B79C
    crc32=641C1F5C
    ^buf[0004]: 35
       = crc32[ 69 ]: 8E6C3698 ^ __641C1F
    crc32=340AC5E3
    ^buf[0005]: 36
       = crc32[ D5 ]: 065E2082 ^ __340AC5
    crc32=F68D2C9E
    ^buf[0006]: 37
       = crc32[ A9 ]: D3E6A601 ^ __F68D2C
    crc32=AFFC9660
    ^buf[0007]: 38
       = crc32[ 58 ]: 5E9F46BF ^ __AFFC96
    crc32=651F2550
    ^buf[0008]: 39
       = crc32[ 69 ]: 8E6C3698 ^ __651F25
    crc32=340BC6D9
        ~=CBF43926
    pass
```


## Bad Data

Bret's code generates this _bogus_ CRC table:

```
    00000000, 06233697, 05C45641, 03E760D6, 020A97ED, 0429A17A, 07CEC1AC, 01EDF73B, 
    04152FDA, 0236194D, 01D1799B, 07F24F0C, 061FB837, 003C8EA0, 03DBEE76, 05F8D8E1, 
    01A864DB, 078B524C, 046C329A, 024F040D, 03A2F336, 0581C5A1, 0666A577, 004593E0, 
    :
    052568B7, 03065E20, 00E13EF6, 06C20861, 072FFF5A, 010CC9CD, 02EBA91B, 04C89F8C, 
    009823B6, 06BB1521, 055C75F7, 037F4360, 0292B45B, 04B182CC, 0756E21A, 0175D48D, 
    048D0C6C, 02AE3AFB, 01495A2D, 076A6CBA, 06879B81, 00A4AD16, 0343CDC0, 0560FB57, 
```

Which generates this "bad checksum" using the _bogus_ CRC table:

```c
    ---------- Mismatched Polynomial and Calculation  ----------
    crc32=FFFFFFFF
    ^buf[0000]: 31
       = crc32[ 73 ]: 07BE5ED7 ^ FFFFFF__
    crc32=FE449FAD
    ^buf[0001]: 32
       = crc32[ B2 ]: 03FDE69B ^ 449FAD__
    crc32=40E09BEC
    ^buf[0002]: 33
       = crc32[ 8C ]: 02DEA580 ^ E09BEC__
    crc32=E725B2D7
    ^buf[0003]: 34
       = crc32[ CB ]: 0592C1D7 ^ 25B2D7__
    crc32=259D5DD6
    ^buf[0004]: 35
       = crc32[ 89 ]: 06F704FA ^ 9D5DD6__
    crc32=9CF5B2DB
    ^buf[0005]: 36
       = crc32[ F0 ]: 009823B6 ^ F5B2DB__
    crc32=F3F2769A
    ^buf[0006]: 37
       = crc32[ 1F ]: 0450BC3A ^ F2769A__
    crc32=F21C8336
    ^buf[0007]: 38
       = crc32[ EE ]: 02EBA91B ^ 1C8336__
    crc32=1F32C140
    ^buf[0008]: 39
       = crc32[ 83 ]: 07267D61 ^ 32C140__
    crc32=365F481A
        ~=C9A0B7E5
    FAIL
```


a) If he had actually _followed his own advice_ ...

> Do not treat CRC32 as a "black box" hash function,

... and _inspected_

* the checksum, and
* the table

to _verify_ they were correct he would have noticed
that _all the high nibbles were set to zero!_
This is dilluting the effectiveness of the CRC32 hash!

Compare and constrast when the tables are _correctly initialized:_

* CRC Polynomial 0x04C11DB7:

```
    00000000, 04C11DB7, 09823B6E, 0D4326D9, 130476DC, 17C56B6B, 1A864DB2, 1E475005,
    2608EDB8, 22C9F00F, 2F8AD6D6, 2B4BCB61, 350C9B64, 31CD86D3, 3C8EA00A, 384FBDBD,
    4C11DB70, 48D0C6C7, 4593E01E, 4152FDA9, 5F15ADAC, 5BD4B01B, 569796C2, 52568B75,
    :
    E3A1CBC1, E760D676, EA23F0AF, EEE2ED18, F0A5BD1D, F464A0AA, F9278673, FDE69BC4,
    89B8FD09, 8D79E0BE, 803AC667, 84FBDBD0, 9ABC8BD5, 9E7D9662, 933EB0BB, 97FFAD0C,
    AFB010B1, AB710D06, A6322BDF, A2F33668, BCB4666D, B8757BDA, B5365D03, B1F740B4,
```

* CRC32 Polynomial 0xEDB88320:

```
    00000000, 77073096, EE0E612C, 990951BA, 076DC419, 706AF48F, E963A535, 9E6495A3, 
    0EDB8832, 79DCB8A4, E0D5E91E, 97D2D988, 09B64C2B, 7EB17CBD, E7B82D07, 90BF1D91, 
    1DB71064, 6AB020F2, F3B97148, 84BE41DE, 1ADAD47D, 6DDDE4EB, F4D4B551, 83D385C7, 
    :
    AED16A4A, D9D65ADC, 40DF0B66, 37D83BF0, A9BCAE53, DEBB9EC5, 47B2CF7F, 30B5FFE9, 
    BDBDF21C, CABAC28A, 53B39330, 24B4A3A6, BAD03605, CDD70693, 54DE5729, 23D967BF, 
    B3667A2E, C4614AB8, 5D681B02, 2A6F2B94, B40BBE37, C30C8EA1, 5A05DF1B, 2D02EF8D, 
```

# Fixing Bret's Code

  There two ways we could fix this depending on which
  polynomial we want to use.

  1. Fix using the reverse polynomial

     a) Table Initialization

     This involves:

     * Use the reverse polynomial

```
        Init( 0xEDB88320 );
```

    b) CRC Calculation

    * Change the incorrect left-shift to right-shift:

```Java
            hash = tab[ (b ^ hash) & 0xFF ] ^ ((hash >> 8) & 0xFFFFFF); // Clamp 'hash >> 8' to 24-bit
```

  2. Fix using the forward polynomial

    a) Table initialization

    This involves fixing the CRC initialization to

    * set the initial crc value
    * if the high-bit is set then shift-left and XOR the polynomial

```
        for( int bite = 0; bite < 256; bite++ )
        {
            int crc = bite << 24;

            for( int bit = 0; bit < 8; bit++ )
            {
                // optimized: if (crc & (1 << 31))
                if (crc < 0)
                {
                    crc <<= 1;
                    crc  ^= POLY;
                }
                else
                {
                    crc <<= 1;
                }
            }

            tab[ bite ] = crc;
        }
```

    b) CRC Calculation

    The data bytes must be bit-reversed

From:

```Java
                hash = (hash << 8) ^ tab[ b ^ (hash >> 24)];
```

To:

```Java
            hash = tab[ (reverse8[ b ] ^ (hash >> 24)) & 0xFF ] ^ (hash << 8);
```


Thus along with verifing the _calculation_ was correct he would have come to a different
conclusion about the validity of CRC32 as a hashing function.

TL:DR; **Don't assume!** VERIFY your code + data.


# CRC32 or CRC33?

Technically, the CRC 32-bit constant `0x04C11DB7` is _really_ a 33-bit constant `0x104C11DB7`
which is classified as an IEEE-802 CRC.
See [RFC 3385](https://tools.ietf.org/html/rfc3385)

|Polynomial | Binary                                  |
|----------:|----------------------------------------:|
| 0x04C11DB7|   `00000100_11000001_00011101_10110111` |
|0x104C11DB7| `1_00000100_11000001_00011101_10110111` |

Since 64-bit computing was hideously expensive at the time of when 
the CRC33 polynomial got truncated down to 32-bits without any loss in generality
even though this gives different results then a pure 33-bit implementation:

|Polynomial | 64-Bit reversed    | 33-Bit reversed |
|----------:|-------------------:|----------------:|
|0x104C11DB7| 0xEDB8832080000000 |     0x1DB710641 |


References:

* "A PAINLESS GUIDE TO CRC ERROR DETECTION ALGORITHMS"
  http://www.ross.net/crc/download/crc_v3.txt

* "Reversing CRC – Theory and Practice."
  http://stigge.org/martin/pub/SAR-PR-2006-05.pdf

* "Hackers Delight", Chapter 14
  http://www.hackersdelight.org/crc.pdf

* Bret Mulvey
  https://web.archive.org/web/20130420172816/http://home.comcast.net/~bretm/hash/8.html
  http://archive.is/6rBZF#selection-251.11-251.21
  http://home.comcast.net/~bretm/hash/8.html

* http://mdfs.net/Info/Comp/Comms/CRC32.htm

* http://programmers.stackexchange.com/questions/49550/which-hashing-algorithm-is-best-for-uniqueness-and-speed
* http://stackoverflow.com/questions/26049150/calculate-a-32-bit-crc-lookup-table-in-c-c
* http://stackoverflow.com/questions/10953958/can-crc32-be-used-as-a-hash-function

* http://wiki.osdev.org/CRC32

* https://decryptpassword.com/encrypt/

