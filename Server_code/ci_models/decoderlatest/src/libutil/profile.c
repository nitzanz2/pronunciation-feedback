/*
 * profile.c -- For timing and event counting.
 *
 * **********************************************
 * CMU ARPA Speech Project
 *
 * Copyright (c) 1996 Carnegie Mellon University.
 * ALL RIGHTS RESERVED.
 * **********************************************
 * 
 * HISTORY
 * 
 * 01-Aug-96	M K Ravishankar (rkm@cs.cmu.edu) at Carnegie Mellon University
 * 		Changed timer_ names to cyctimer_ (for cycle counter).
 * 		Added timing_ structures and functions using system calls for timing.
 * 
 * 13-Dec-95	M K Ravishankar (rkm@cs.cmu.edu) at Carnegie Mellon University
 * 		Added ifdefs around cyctimer_print_all and cyctimer_print_all_norm.
 * 
 * 27-Nov-95	M K Ravishankar (rkm@cs.cmu.edu) at Carnegie Mellon University
 * 		Created from Sphinx-II version.
 */


#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#if (! WIN32)
#include <sys/time.h>
#include <sys/resource.h>
#else
#include <windows.h>
#include <time.h>
#endif

#include "profile.h"
#include "err.h"
#include "ckd_alloc.h"


#if (ALPHA_OSF1)
extern uint32 rpcc();		/* On an alpha, use the RPCC instruction */
static int32 mhz = 0;		/* Guess regarding clock rate on machine (in MHz) */
#endif

typedef struct {
    char *name;
    uint32 start_time;
    float accum_time;
} cyctimer_t;
static cyctimer_t *timer = NULL;
static int32 n_timer = 0;
#define MAX_TIMER		30
#define ACCUM_SCALE		0.001

typedef struct {
    char *name;
    uint32 count;
} ctr_t;
static ctr_t *ctr = NULL;
static int32 n_ctr = 0;

#define MAX_CTR			30


int32 counter_new (char *name)
{
    if (! ctr)
	ctr = (ctr_t *) ckd_calloc (MAX_CTR, sizeof(ctr_t));
    if (n_ctr >= MAX_CTR) {
	E_WARN("#counters (%d) exceeded\n", MAX_CTR);
	return -1;
    }
    ctr[n_ctr].name = (char *) ckd_salloc (name);
    ctr[n_ctr].count = 0;
    
    return (n_ctr++);
}


void counter_increment (int32 id, int32 inc)
{
    if ((id < 0) || (id >= MAX_CTR))
	return;
    
    ctr[id].count += inc;
}


void counter_reset (int32 id)
{
    if ((id < 0) || (id >= MAX_CTR))
	return;
    
    ctr[id].count = 0;
}


void counter_reset_all ( void )
{
    int32 i;
    
    for (i = 0; i < n_ctr; i++)
	counter_reset (i);
}


static int32 get_namelen ( void )
{
    int32 i, len;

    len = 0;
    for (i = 0; i < n_ctr; i++)
	if (len < strlen(ctr[i].name))
	    len = strlen(ctr[i].name);
    for (i = 0; i < n_timer; i++)
	if (len < strlen(timer[i].name))
	    len = strlen(timer[i].name);

    return (len);
}


void counter_print_all (FILE *fp)
{
    int32 i;
    
    if (n_ctr > 0) {
	fprintf (fp, "CTR:");
	for (i = 0; i < n_ctr; i++)
	    fprintf (fp, "[%s %10d]", ctr[i].name, ctr[i].count);
	fprintf (fp, "\n");
    }
}


#if (ALPHA_OSF1)
static int32 clock_speed (int32 dummy)
{
    int32 i, j, k, besti, bestj, diff;
    uint32 rpcc_start, rpcc_end;
    struct rusage start, stop;
    float64 t;
    
    getrusage (RUSAGE_SELF, &start);
    rpcc_start = rpcc();
    for (i = 0; i < 100000000; i++)
	if (i > dummy)
	    return (i);
    rpcc_end = rpcc();
    getrusage (RUSAGE_SELF, &stop);
    
    t = (stop.ru_utime.tv_sec - start.ru_utime.tv_sec) + 
	((stop.ru_utime.tv_usec - start.ru_utime.tv_usec) * 0.000001);
    mhz = ((rpcc_end - rpcc_start) / t) * 0.000001 + 0.5;
    diff = (int32)0x7fffffff;
    for (i = 100; i <= 1000; i += 100) {
	for (j = 1; j <= 10; j++) {
	    k = i/j - mhz;
	    if (k < 0)
		k = -k;
	    if (k < diff) {
		diff = k;
		besti = i;
		bestj = j;
	    }
	}
    }
    mhz = besti/bestj;
    E_INFO("%d ticks in %.3f sec; machine clock rate = %d MHz\n",
	   rpcc_end - rpcc_start, t, mhz);
    
    return 0;
}
#endif


int32 cyctimer_new (char *name)
{
#if (ALPHA_OSF1)
    {
	int32 dummy;
	
	dummy = name[0] | ((int32) 0x70000000);
	if (mhz == 0)
	    clock_speed (dummy);
    }
    
    if (! timer)
	timer = (cyctimer_t *) ckd_calloc (MAX_TIMER, sizeof(cyctimer_t));
    if (n_timer >= MAX_TIMER) {
	E_WARN("#timers (%d) exceeded\n", MAX_TIMER);
	return -1;
    }
    timer[n_timer].name = (char *) ckd_salloc (name);
    timer[n_timer].accum_time = 0.0;

    return (n_timer++);
#else
    return -1;
#endif
}


void cyctimer_resume (int32 id)
{
#if (ALPHA_OSF1)
    if ((id < 0) || (id >= MAX_TIMER))
	return;
    
    timer[id].start_time = rpcc();
#endif
}


void cyctimer_pause (int32 id)
{
#if (ALPHA_OSF1)
    if ((id < 0) || (id >= MAX_TIMER))
	return;
    
    timer[id].accum_time += (rpcc() - timer[id].start_time) * ACCUM_SCALE;
#endif
}


void cyctimer_reset (int32 id)
{
    if ((id < 0) || (id >= MAX_TIMER))
	return;
    
    timer[id].accum_time = 0.0;
}


void cyctimer_reset_all ( void )
{
    int32 i;
    
    for (i = 0; i < n_timer; i++)
	cyctimer_reset (i);
}


void cyctimer_print_all (FILE *fp)
{
#if (ALPHA_OSF1)
    int32 i;
    char fmtstr[1024];
    
    if (n_timer > 0) {
	fprintf (fp, "PROFILING TIMERS:\n");

	sprintf (fmtstr, "  TMR: %%-%ds %%10d\n", get_namelen());
	for (i = 0; i < n_timer; i++)
	    fprintf (fp, fmtstr, timer[i].name, (int32)(timer[i].accum_time));
    }
#endif
}


float64 cyctimer_get_sec (int32 id)
{
#if (ALPHA_OSF1)
    float64 t;
    
    t = (timer[id].accum_time / ACCUM_SCALE) / (1000000.0 * mhz);	/* Sec */
    return (t);
#else
    return (0.0);
#endif
}


void cyctimer_print_all_norm (FILE *fp, float64 norm_sec, int32 norm_id)
{
#if (ALPHA_OSF1)
    int32 i;
    float64 t;
    
    if (n_timer > 0) {
	fprintf (fp, "TMR:");
	for (i = 0; i < n_timer; i++) {
	    t = (timer[i].accum_time / ACCUM_SCALE) / (1000000.0 * mhz);	/* Sec */

	    fprintf (fp, "[%s %6.1fs %5.1fx %3.0f%%]",
		     timer[i].name, t, t / norm_sec,
		     (timer[i].accum_time * 100.0) / timer[norm_id].accum_time);
	}
	fprintf (fp, "\n");
    }
#endif
}


#if (WIN32)

#define TM_LOWSCALE	1e-7
#define TM_HIGHSCALE	(4294967296.0 * TM_LOWSCALE);

static float64 make_sec (FILETIME *tm)
{
    float64 dt;
    
    dt = tm->dwLowDateTime * TM_LOWSCALE;
    dt += tm->dwHighDateTime * TM_HIGHSCALE;

    return (dt);
}

#else

static float64 make_sec (struct timeval *s)
{
    return (s->tv_sec + s->tv_usec * 0.000001);
}

#endif


/*
 * Obtain and initialize a timing module
 */
timing_t *timing_new ( void )
{
    timing_t *tm;

#if (ALPHA_OSF1)
    {
	int32 dummy;
	
	dummy = (dummy & 0x000000ff) | ((int32) 0x70000000);
	if (mhz == 0)
	    clock_speed (dummy);
    }
#endif

    tm = (timing_t *) ckd_calloc (1, sizeof(timing_t));
    tm->t_elapsed = 0.0;
    tm->t_cpu = 0.0;

    return tm;
}


void timing_start (timing_t *tm)
{
#if (! _SUN4)
#if (! WIN32)
    struct timeval e_start;	/* Elapsed time */
    
#if (! _HPUX_SOURCE)
    struct rusage start;	/* CPU time */
    
    /* Unix but not HPUX */
    getrusage (RUSAGE_SELF, &start);
    tm->start_cpu = make_sec (&start.ru_utime) + make_sec (&start.ru_stime);
#endif
    /* Unix + HP */
    gettimeofday (&e_start, 0);
    tm->start_elapsed = make_sec (&e_start);
#else
    HANDLE pid;
    FILETIME t_create, t_exit, kst, ust;
    
    /* PC */
    pid = GetCurrentProcess();
    GetProcessTimes (pid, &t_create, &t_exit, &kst, &ust);
    tm->start_cpu = make_sec (&ust) + make_sec (&kst);

    tm->start_elapsed = (float64)clock() / CLOCKS_PER_SEC;
#endif
#endif
}


void timing_stop (timing_t *tm)
{
#if (! _SUN4)
    float64 dt_cpu, dt_elapsed;
    
#if (! WIN32)
    struct timeval e_stop;	/* Elapsed time */
    
#if (! _HPUX_SOURCE)
    struct rusage stop;		/* CPU time */
    
    /* Unix but not HPUX */
    getrusage (RUSAGE_SELF, &stop);
    dt_cpu = make_sec (&stop.ru_utime) + make_sec (&stop.ru_stime) - tm->start_cpu;
#else
    dt_cpu = 0.0;
#endif
    /* Unix + HP */
    gettimeofday (&e_stop, 0);
    dt_elapsed = (make_sec (&e_stop) - tm->start_elapsed);
#else
    HANDLE pid;
    FILETIME t_create, t_exit, kst, ust;
    
    /* PC */
    pid = GetCurrentProcess();
    GetProcessTimes (pid, &t_create, &t_exit, &kst, &ust);
    dt_cpu = make_sec (&ust) + make_sec (&kst) - tm->start_cpu;
    dt_elapsed = ((float64)clock() / CLOCKS_PER_SEC) - tm->start_elapsed;
#endif

    tm->t_cpu += dt_cpu;
    tm->t_elapsed += dt_elapsed;

    tm->t_tot_cpu += dt_cpu;
    tm->t_tot_elapsed += dt_elapsed;
#endif
}


void timing_reset (timing_t *tm)
{
    tm->t_cpu = 0.0;
    tm->t_elapsed = 0.0;
}
