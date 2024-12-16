    int       gnThreadsMaximum = 0 ;
    int       gnThreadsActive  = 0 ; // 0 = auto detect; > 0 use manual # of threads

    const int MAX_THREADS      = 256; // Threadripper 3990X

// ========================================================================
void Threads_Default()
{
    gnThreadsMaximum = omp_get_num_procs();
    if( gnThreadsMaximum > MAX_THREADS )
        gnThreadsMaximum = MAX_THREADS;
}

// ========================================================================
void Threads_Set()
{
   if(!gnThreadsActive) // user didn't specify how many threads to use, default to all of them
        gnThreadsActive = gnThreadsMaximum;

    omp_set_num_threads( gnThreadsActive );
}
