/*
 * dag-main.c -- Driver for DAG search.
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
 * $Log$
 * 
 * 08-Sep-97	M K Ravishankar (rkm@cs.cmu.edu) at Carnegie Mellon University
 * 		Added .Z compression option to lattice files.
 * 
 * 22-Nov-96	M K Ravishankar (rkm@cs.cmu.edu) at Carnegie Mellon University
 * 		Added -maxedge argument to control memory usage.
 * 
 * 21-Nov-96	M K Ravishankar (rkm@cs.cmu.edu) at Carnegie Mellon University
 * 		Added -maxlmop and -maxlpf options to control execution time.
 * 
 * 15-Nov-96	M K Ravishankar (rkm@cs.cmu.edu) at Carnegie Mellon University
 * 		Changed the meaning of -matchsegfn and, correspondingly, log_hypseg().
 * 
 * 08-Nov-96	M K Ravishankar (rkm@cs.cmu.edu) at Carnegie Mellon University
 * 		Added BSTXCT: reporting since that became available from dag_search.
 * 
 * 05-Nov-96	M K Ravishankar (rkm@cs.cmu.edu) at Carnegie Mellon University
 * 		Added -dagfudge and -min_endfr parameter handling.
 *  
 * 16-Oct-96	M K Ravishankar (rkm@cs.cmu.edu) at Carnegie Mellon University
 * 		Added orig_stdout, orig_stderr hack to avoid hanging on exit under Linux.
 *  
 * 11-Oct-96	M K Ravishankar (rkm@cs.cmu.edu) at Carnegie Mellon University
 * 		Added fillpen_init() and removed explicit addition of SILENCE_WORD,
 * 		START_WORD and FINISH_WORD to the dictionary.
 * 
 * 03-Sep-96	M K Ravishankar (rkm@cs.cmu.edu) at Carnegie Mellon University
 * 		Creating.
 */


#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#if (! WIN32)
#include <unistd.h>
#endif
#include <assert.h>

#include <libutil/libutil.h>

#include "s3types.h"
#include "logs3.h"
#include "tmat.h"
#include "mdef.h"
#include "dict.h"
#include "lm.h"
#include "fillpen.h"
#include "search.h"


static mdef_t *mdef;		/* Model definition */

static s3wid_t startwid, finishwid, silwid;

static timing_t *tm_utt;
static int32 tot_nfr;


/*
 * Command line arguments.
 */
static arg_def_t defn[] = {
    { "-logbase",
      CMD_LN_FLOAT32,
      CMD_LN_NO_VALIDATION,
      "1.0001",
      "Base in which all log values calculated" },
    { "-mdeffn",
      CMD_LN_STRING,
      CMD_LN_NO_VALIDATION,
      CMD_LN_NO_DEFAULT,
      "Model definition input file: triphone -> senones/tmat tying" },
    { "-dictfn",
      CMD_LN_STRING,
      CMD_LN_NO_VALIDATION,
      CMD_LN_NO_DEFAULT,
      "Main pronunciation dictionary (lexicon) input file" },
    { "-fdictfn",
      CMD_LN_STRING,
      CMD_LN_NO_VALIDATION,
      CMD_LN_NO_DEFAULT,
      "Optional filler word (noise word) pronunciation dictionary input file" },
    { "-lmfn",
      CMD_LN_STRING,
      CMD_LN_NO_VALIDATION,
      CMD_LN_NO_DEFAULT,
      "Language model input file (precompiled .DMP file)" },
    { "-langwt",
      CMD_LN_FLOAT32,
      CMD_LN_NO_VALIDATION,
      "9.5",
      "Language weight: empirical exponent applied to LM probabilty" },
    { "-ugwt",
      CMD_LN_FLOAT32,
      CMD_LN_NO_VALIDATION,
      "0.7",
      "LM unigram weight: unigram probs interpolated with uniform distribution with this weight" },
    { "-inspen",
      CMD_LN_FLOAT32,
      CMD_LN_NO_VALIDATION,
      "0.65",
      "Word insertion penalty" },
    { "-ctlfn",
      CMD_LN_STRING,
      CMD_LN_NO_VALIDATION,
      CMD_LN_NO_DEFAULT,
      "Input control file listing utterances to be decoded" },
    { "-ctloffset",
      CMD_LN_INT32,
      CMD_LN_NO_VALIDATION,
      "0",
      "No. of utterances at the beginning of -ctlfn file to be skipped" },
    { "-ctlcount",
      CMD_LN_INT32,
      CMD_LN_NO_VALIDATION,
      CMD_LN_NO_DEFAULT,
      "No. of utterances in -ctlfn file to be processed (after -ctloffset).  Default: Until EOF" },
    { "-silpen",
      CMD_LN_FLOAT32,
      CMD_LN_NO_VALIDATION,
      "0.1",
      "Language model 'probability' of silence word" },
    { "-noisepen",
      CMD_LN_FLOAT32,
      CMD_LN_NO_VALIDATION,
      "0.05",
      "Language model 'probability' of each non-silence filler word" },
    { "-fillpenfn",
      CMD_LN_STRING,
      CMD_LN_NO_VALIDATION,
      CMD_LN_NO_DEFAULT,
      "Filler word probabilities input file (used in place of -silpen and -noisepen)" },
    { "-min_endfr",
      CMD_LN_INT32,
      CMD_LN_NO_VALIDATION,
      "3",
      "Nodes ignored during search if they persist for fewer than so many end frames" },
    { "-dagfudge",
      CMD_LN_INT32,
      CMD_LN_NO_VALIDATION,
      "2",
      "(0..2); 1 or 2: add edge if endframe == startframe; 2: if start == end-1" },
    { "-maxlpf",
      CMD_LN_INT32,
      CMD_LN_NO_VALIDATION,
      "40000",
      "Max LMops/frame after which utterance aborted; controls CPU use (see maxlmop)" },
    { "-maxlmop",
      CMD_LN_INT32,
      CMD_LN_NO_VALIDATION,
      "100000000",
      "Max LMops in utterance after which it is aborted; controls CPU use (see maxlpf)" },
    { "-maxedge",
      CMD_LN_INT32,
      CMD_LN_NO_VALIDATION,
      "2000000",
      "Max DAG edges allowed in utterance; aborted if exceeded; controls memory usage" },
    { "-inlatdir",
      CMD_LN_STRING,
      CMD_LN_NO_VALIDATION,
      CMD_LN_NO_DEFAULT,
      "Input word-lattice directory with per-utt files for restricting words searched" },
    { "-latext",
      CMD_LN_STRING,
      CMD_LN_NO_VALIDATION,
      "lat.gz",
      "Word-lattice filename extension (.gz or .Z extension for compression)" },
    { "-matchfn",
      CMD_LN_STRING,
      CMD_LN_NO_VALIDATION,
      CMD_LN_NO_DEFAULT,
      "Recognition result output file (old NIST format; pre Nov95)" },
    { "-matchsegfn",
      CMD_LN_STRING,
      CMD_LN_NO_VALIDATION,
      CMD_LN_NO_DEFAULT,
      "Exact recognition result file with word segmentations and scores" },
    { "-logfn",
      CMD_LN_STRING,
      CMD_LN_NO_VALIDATION,
      CMD_LN_NO_DEFAULT,
      "Log file (default stdout/stderr)" },
    { "-backtrace",
      CMD_LN_INT32,
      CMD_LN_NO_VALIDATION,
      "1",
      "Whether detailed backtrace information (word segmentation/scores) shown in log" },
    
    { NULL, CMD_LN_UNDEF, CMD_LN_NO_VALIDATION, CMD_LN_NO_DEFAULT, NULL }
};


/* Hacks to avoid hanging problem under Linux */
static FILE orig_stdout, orig_stderr;
static FILE *logfp;

static int32 cmdline_parse (int argc, char *argv[])
{
    int32 i;
    char *logfile;
    
    E_INFO("Parsing command line:\n");
    for (i = 0; i < argc; i++) {
	if (argv[i][0] == '-')
	    printf ("\\\n\t");
	printf ("%s ", argv[i]);
    }
    printf ("\n\n");
    fflush (stdout);

    cmd_ln_parse (argc, argv);

    if (cmd_ln_validate() == FALSE) {
	E_FATAL("Unable to validate command line arguments\n");
    }

    logfp = NULL;
    if ((logfile = (char *)cmd_ln_access("-logfn")) != NULL) {
	if ((logfp = fopen(logfile, "w")) == NULL) {
	    E_ERROR("fopen(%s,w) failed; logging to stdout/stderr\n");
	} else {
	    orig_stdout = *stdout;	/* Hack!! To avoid hanging problem under Linux */
	    orig_stderr = *stderr;	/* Hack!! To avoid hanging problem under Linux */
	    *stdout = *logfp;
	    *stderr = *logfp;
	    
	    E_INFO("Command line:\n");
	    for (i = 0; i < argc; i++) {
		if (argv[i][0] == '-')
		    printf ("\\\n\t");
		printf ("%s ", argv[i]);
	    }
	    printf ("\n\n");
	    fflush (stdout);
	}
    }
    
    E_INFO("Configuration in effect:\n");
    cmd_ln_print_configuration();
    printf ("\n");
    
    return 0;
}


/*
 * Load and cross-check all models (acoustic/lexical/linguistic).
 */
static void models_init ( void )
{
    dict_t *dict;
    
    /* HMM model definition */
    mdef = mdef_init ((char *) cmd_ln_access("-mdeffn"));

    /* Dictionary */
    dict = dict_init ((char *) cmd_ln_access("-dictfn"),
		      (char *) cmd_ln_access("-fdictfn"));

    /* HACK!! Make sure SILENCE_WORD, START_WORD and FINISH_WORD are in dictionary */
    silwid = dict_wordid (SILENCE_WORD);
    startwid = dict_wordid (START_WORD);
    finishwid = dict_wordid (FINISH_WORD);
    if (NOT_WID(silwid) || NOT_WID(startwid) || NOT_WID(finishwid)) {
	E_FATAL("%s, %s, or %s missing from dictionary\n",
		SILENCE_WORD, START_WORD, FINISH_WORD);
    }
    if ((dict->filler_start > dict->filler_end) || (! dict_filler_word (silwid)))
	E_FATAL("%s must occur (only) in filler dictionary\n", SILENCE_WORD);
    /* No check that alternative pronunciations for filler words are in filler range!! */

    /* LM */
    lm_read ((char *) cmd_ln_access("-lmfn"), "");

    /* Filler penalties */
    fillpen_init ((char *) cmd_ln_access("-fillpenfn"),
		  dict->filler_start, dict->filler_end);
}


/*
 * Write exact hypothesis.  Format:
 *   <id> T <scr> A <ascr> L <lscr> {<sf> <wascr> <wlscr> <word>}... <ef>
 * where:
 *   scr = ascr + (lscr*lw+N*wip), where N = #words excluding <s>
 *   ascr = scaled acoustic score for entire utterance
 *   lscr = LM score (without lw or wip) for entire utterance
 *   sf = start frame for word
 *   wascr = scaled acoustic score for word
 *   wlscr = LM score (without lw or wip) for word
 *   ef = end frame for utterance.
 */
static void log_hypseg (char *uttid,
			FILE *fp,	/* Out: output file */
			hyp_t *hypptr,	/* In: Hypothesis */
			int32 nfrm)	/* In: #frames in utterance */
{
    hyp_t *h;
    int32 ascr, lscr, tscr;
    
    ascr = lscr = tscr = 0;
    for (h = hypptr; h; h = h->next) {
	ascr += h->ascr;
	if (dict_basewid(h->wid) != startwid) {
	    lscr += lm_rawscore (h->lscr, 1.0);
	} else {
	    assert (h->lscr == 0);
	}
	tscr += h->ascr + h->lscr;
    }

    fprintf (fp, "%s T %d A %d L %d", uttid, tscr, ascr, lscr);
    
    if (! hypptr)	/* HACK!! */
	fprintf (fp, " (null)\n");
    else {
	for (h = hypptr; h; h = h->next) {
	    lscr = (dict_basewid(h->wid) != startwid) ? lm_rawscore (h->lscr, 1.0) : 0;
	    fprintf (fp, " %d %d %d %s", h->sf, h->ascr, lscr, dict_wordstr (h->wid));
	}
	fprintf (fp, " %d\n", nfrm);
    }
    
    fflush (fp);
}


/* Write hypothesis in old (pre-Nov95) NIST format */
static void log_hypstr (FILE *fp, hyp_t *hypptr, char *uttid, int32 scr)
{
    hyp_t *h;
    s3wid_t w;

    if (! hypptr)	/* HACK!! */
	fprintf (fp, "(null)");
    
    for (h = hypptr; h; h = h->next) {
	w = dict_basewid (h->wid);
	if ((w != startwid) && (w != finishwid) && (! dict_filler_word (w)))
	    fprintf (fp, "%s ", dict_wordstr(w));
    }

    if (scr != 0)
	fprintf (fp, " (%s %d)\n", uttid, scr);
    else
	fprintf (fp, " (%s)\n", uttid);

    fflush (fp);
}


/* Log hypothesis in detail with word segmentations, acoustic and LM scores  */
static void log_hyp_detailed (FILE *fp, hyp_t *hypptr, char *uttid, char *LBL, char *lbl)
{
    hyp_t *h;
    int32 ascr_norm, lscr;

    ascr_norm = 0;
    lscr = 0;
    
    fprintf (fp, "%s:%s> %20s %5s %5s %11s %10s\n", LBL, uttid,
	     "WORD", "SFrm", "EFrm", "AScr(Norm)", "LMScore");
    
    for (h = hypptr; h; h = h->next) {
	fprintf (fp, "%s:%s> %20s %5d %5d %11d %10d\n", lbl, uttid,
		 h->word, h->sf, h->ef, h->ascr, h->lscr);

	ascr_norm += h->ascr;
	lscr += h->lscr;
    }

    fprintf (fp, "%s:%s> %20s %5s %5s %11d %10d\n", LBL, uttid,
	     "TOTAL", "", "", ascr_norm, lscr);
}


/* Decode the given mfc file and write result to matchfp and matchsegfp */
static void decode_utt (char *uttid, FILE *matchfp, FILE *matchsegfp)
{
    char dagfile[1024];
    hyp_t *h, *hyp;
    char *latdir, *latext;
    int32 nfrm, ascr, lscr;

    timing_reset (tm_utt);
    timing_start (tm_utt);
    
    latdir = (char *) cmd_ln_access ("-inlatdir");
    latext = (char *) cmd_ln_access ("-latext");
    if (latdir)
	sprintf (dagfile, "%s/%s.%s", latdir, uttid, latext);
    else
	sprintf (dagfile, "%s.%s", uttid, latext);
    
    if ((nfrm = dag_load (dagfile)) >= 0) {
	hyp = dag_search (uttid);

	if ( *((int32 *) cmd_ln_access("-backtrace")) )
	    log_hyp_detailed (stdout, hyp, uttid, "BP", "bp");
	
	/* Total scaled acoustic score and LM score */
	ascr = lscr = 0;
	for (h = hyp; h; h = h->next) {
	    ascr += h->ascr;
	    lscr += h->lscr;
	}
	
	printf ("BSTPTH: ");
	log_hypstr (stdout, hyp, uttid, ascr+lscr);

	printf ("BSTXCT: ");
	log_hypseg (uttid, stdout, hyp, nfrm);

	lm_cache_stats_dump ();
	lm_cache_reset ();
    } else {
	E_ERROR("DAG search (%s) failed\n", uttid);
	hyp = NULL;
    }
    
    /* Log recognition output to the standard match and matchseg files */
    if (matchfp)
	log_hypstr (matchfp, hyp, uttid, 0);
    if (matchsegfp)
	log_hypseg (uttid, matchsegfp, hyp, nfrm);
    
    dag_destroy ();

    timing_stop (tm_utt);
    
    printf ("%s: TMR: %5d Frm", uttid, nfrm);
    if (nfrm > 0) {
	printf (" %6.2f xEl", tm_utt->t_elapsed * 100.0 / nfrm);
	printf (" %6.2f xCPU", tm_utt->t_cpu * 100.0 / nfrm);
    }
    printf ("\n");
    fflush (stdout);

    tot_nfr += nfrm;
}


/* Process utterances in the control file (-ctlfn argument) */
static void process_ctlfile ( void )
{
    FILE *ctlfp, *matchfp, *matchsegfp;
    char *ctlfile;
    char *matchfile, *matchsegfile;
    char line[1024], ctlspec[1024], uttid[1024];
    int32 ctloffset, ctlcount;
    int32 i, k, sf, ef;
    
    if ((ctlfile = (char *) cmd_ln_access("-ctlfn")) == NULL)
	E_FATAL("No -ctlfn argument\n");
    
    E_INFO("Processing ctl file %s\n", ctlfile);
    
    if ((ctlfp = fopen (ctlfile, "r")) == NULL)
	E_FATAL("fopen(%s,r) failed\n", ctlfile);
    
    if ((matchfile = (char *) cmd_ln_access("-matchfn")) == NULL) {
	E_WARN("No -matchfn argument\n");
	matchfp = NULL;
    } else {
	if ((matchfp = fopen (matchfile, "w")) == NULL)
	    E_ERROR("fopen(%s,w) failed\n", matchfile);
    }
    
    if ((matchsegfile = (char *) cmd_ln_access("-matchsegfn")) == NULL) {
	E_WARN("No -matchsegfn argument\n");
	matchsegfp = NULL;
    } else {
	if ((matchsegfp = fopen (matchsegfile, "w")) == NULL)
	    E_ERROR("fopen(%s,w) failed\n", matchsegfile);
    }
    
    ctloffset = *((int32 *) cmd_ln_access("-ctloffset"));
    if (! cmd_ln_access("-ctlcount"))
	ctlcount = 0x7fffffff;	/* All entries processed if no count specified */
    else
	ctlcount = *((int32 *) cmd_ln_access("-ctlcount"));
    if (ctlcount == 0) {
	E_INFO("-ctlcount argument = 0!!\n");
	fclose (ctlfp);
	return;
    }

    if (ctloffset > 0)
	E_INFO("Skipping %d utterances in the beginning of control file\n",
	       ctloffset);
    while ((ctloffset > 0) && (fgets(line, sizeof(line), ctlfp) != NULL)) {
	if (sscanf (line, "%s", ctlspec) > 0)
	    --ctloffset;
    }
    
    while ((ctlcount > 0) && (fgets(line, sizeof(line), ctlfp) != NULL)) {
	printf ("\n");
	E_INFO("Utterance: %s", line);

	sf = 0;
	ef = (int32)0x7ffffff0;
	if ((k = sscanf (line, "%s %d %d %s", ctlspec, &sf, &ef, uttid)) <= 0)
	    continue;	    /* Empty line */

	if ((k == 2) || ( (k >= 3) && ((sf >= ef) || (sf < 0))) ) {
	    E_ERROR("Error in ctlfile spec; skipped\n");
	    /* What happens to ctlcount??? */
	    continue;
	}
	if (k < 4) {
	    /* Create utt-id from mfc-filename (and sf/ef if specified) */
	    for (i = strlen(ctlspec)-1; (i >= 0) && (ctlspec[i] != '/'); --i);
	    if (k == 3)
		sprintf (uttid, "%s_%d_%d", ctlspec+i+1, sf, ef);
	    else
		strcpy (uttid, ctlspec+i+1);
	}

	decode_utt (uttid, matchfp, matchsegfp);

	--ctlcount;
    }
    printf ("\n");

    if (fscanf (ctlfp, "%s", line) == 1)
	E_INFO("Skipping rest of control file beginning with:\n\t%s\n", line);

    if (matchfp)
	fclose (matchfp);
    if (matchsegfp)
	fclose (matchsegfp);

    fclose (ctlfp);
}


static int32 load_argfile (char *file, char *pgm, char ***argvout)
{
    FILE *fp;
    char line[1024], word[1024], *lp, **argv;
    int32 len, n;

    E_INFO("Reading arguments from %s\n", file);

    if ((fp = fopen (file, "r")) == NULL) {
	E_ERROR("fopen(%s,r) failed\n", file);
	return -1;
    }

    /* Count #arguments */
    n = 1;	/* Including pgm */
    while (fgets (line, sizeof(line), fp) != NULL) {
	if (line[0] == '#')
	    continue;

	lp = line;
	while (sscanf (lp, "%s%n", word, &len) == 1) {
	    lp += len;
	    n++;
	}
    }
    
    /* Allocate space for arguments */
    argv = (char **) ckd_calloc (n+1, sizeof(char *));
    
    /* Create argv list */
    rewind (fp);
    argv[0] = pgm;
    n = 1;
    while (fgets (line, sizeof(line), fp) != NULL) {
	if (line[0] == '#')
	    continue;

	lp = line;
	while (sscanf (lp, "%s%n", word, &len) == 1) {
	    lp += len;
	    argv[n] = ckd_salloc (word);
	    n++;
	}
    }
    argv[n] = NULL;
    *argvout = argv;

    fclose (fp);

    return n;
}


main (int32 argc, char *argv[])
{
    char *str;
    
#if 0
    ckd_debug(100000);
#endif
    
    /* Digest command line argument definitions */
    cmd_ln_define (defn);

    if ((argc == 2) && (strcmp (argv[1], "help") == 0)) {
	cmd_ln_print_definitions();
	exit(1); 
    }

    /* Look for default or specified arguments file */
    str = NULL;
    if ((argc == 2) && (argv[1][0] != '-'))
	str = argv[1];
    else if (argc == 1) {
	str = "s3decode.arg";
	E_INFO("Looking for default argument file: %s\n", str);
    }
    if (str) {
	/* Build command line argument list from file */
	if ((argc = load_argfile (str, argv[0], &argv)) < 0) {
	    fprintf (stderr, "Usage:\n");
	    fprintf (stderr, "\t%s argument-list, or\n", argv[0]);
	    fprintf (stderr, "\t%s [argument-file] (default file: s3decode.arg)\n\n",
		     argv[0]);
	    cmd_ln_print_definitions();
	    exit(1);
	}
    }
    
    cmdline_parse (argc, argv);

    /* Remove memory allocation restrictions */
    unlimit ();
    
#if (! WIN32)
    {
	char buf[1024];
	
	gethostname (buf, 1024);
	buf[1023] = '\0';
	E_INFO ("Executing on: %s\n", buf);
    }
#endif

    E_INFO("%s COMPILED ON: %s, AT: %s\n\n", argv[0], __DATE__, __TIME__);
    
    if ((cmd_ln_access("-mdeffn") == NULL) ||
	(cmd_ln_access("-dictfn") == NULL) ||
	(cmd_ln_access("-lmfn") == NULL))
	E_FATAL("Missing -mdeffn, -dictfn, or -lmfn argument\n");
    
    /*
     * Initialize log(S3-base).  All scores (probs...) computed in log domain to avoid
     * underflow.  At the same time, log base = 1.0001 (1+epsilon) to allow log values
     * to be maintained in int32 variables without significant loss of precision.
     */
    if (cmd_ln_access("-logbase") == NULL)
	logs3_init (1.0001);
    else {
	float32 logbase;
    
	logbase = *((float32 *) cmd_ln_access("-logbase"));
	if (logbase <= 1.0)
	    E_FATAL("Illegal log-base: %e; must be > 1.0\n", logbase);
	if (logbase > 1.1)
	    E_WARN("Logbase %e perhaps too large??\n", logbase);
	logs3_init ((float64) logbase);
    }
    
    /* Read in input databases */
    models_init ();

    /* Allocate timing object */
    tm_utt = timing_new ();
    tot_nfr = 0;
    
    /* Initialize forward Viterbi search module */
    dag_init ();
    printf ("\n");
    
    process_ctlfile ();

    printf ("\n");
    printf("TOTAL FRAMES:       %8d\n", tot_nfr);
    if (tot_nfr > 0) {
	printf("TOTAL CPU TIME:     %11.2f sec, %7.2f xRT\n",
	       tm_utt->t_tot_cpu, tm_utt->t_tot_cpu/(tot_nfr*0.01));
	printf("TOTAL ELAPSED TIME: %11.2f sec, %7.2f xRT\n",
	       tm_utt->t_tot_elapsed, tm_utt->t_tot_elapsed/(tot_nfr*0.01));
    }
    fflush (stdout);

#if (! WIN32)
    system ("ps auxwww | grep s3dag");
#endif

    /* Hack!! To avoid hanging problem under Linux */
    if (logfp) {
	fclose (logfp);
	*stdout = orig_stdout;
	*stderr = orig_stderr;
    }

    exit(0);
}
