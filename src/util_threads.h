    int       gnThreadsMaximum = 0 ;
    int       gnThreadsActive  = 0 ; // 0 = auto detect; > 0 manual # of threads

    const int MAX_THREADS      = 256; // Threadripper 3990X
    uint64_t  aHaystack[ MAX_THREADS ];
