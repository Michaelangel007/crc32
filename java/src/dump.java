public class util
{
    public static void dump( int[] array )
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
}
