package crc32;

public class reflect
{
    int[] _crc;

// Private

    static void dump( int[] array )
    {
        for( int bite = 0; bite < 256; bite++ )
        {
            if( (bite % 8) == 0 )
                System.out.print( "    " );

            System.out.print( String.format( "%08X, ", array[ bite ] ) );

            if( (bite % 8) == 7 )
                System.out.print( String.format( " // %3d [ 0x%02X ]\n", bite - 7, bite - 7 ) );
        }

        System.out.println();
    }

    void init( int POLY )
    {
        _crc = new int[256];

        for( int bite = 0; bite < 256; bite++ )
        {
            int crc = bite;

            for( int bit = 0; bit < 8; bit++ )
            {
                // if (crc & 1) // Work around retarded Java boolean cast
                if ((crc & 1) == 1)
                {
                    crc >>= 1;
                    crc  &= 0x7FFFFFFF; // Work around retarded pre Java 8 lacking unsigned
                    crc  ^= POLY;
                }
                else
                {
                    crc >>= 1;
                    crc  &= 0x7FFFFFFF; // Work around retarded pre Java 8 lacking unsigned
                }
            }

            _crc[ bite ] = crc;
        }
    }

// Public

    public reflect()
    {
        init( 0xEDB88320 );
    }

    public reflect( int poly )
    {
        init( poly );
    }

    public void Dump()
    {
        dump( _crc );
    }

    public int ComputeHash(byte[] data)
    {
        int hash = 0xFFFFFFFF;

        for( byte bite: data )
            hash = _crc[ (bite ^ hash) & 0xFF ] ^ ((hash >> 8) & 0xFFFFFF); // Clamp 'hash >> 8' to 24-bit

        return ~hash;
    }
}
