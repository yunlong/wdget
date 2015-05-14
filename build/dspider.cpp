#define USE_TIMER_H
#define USE_THREAD_H
#define USE_HTTPMSG_H
#define USE_SOCKET_H
#define USE_POLLER_H
#define USE_HASHTABLE_H
#define USE_HTTPCLI_H
#define USE_CRAWLER_H
#define USE_RAWSERVER_H
#include "matrix.h"
#include "getopt.h"

#include "Uri.h"
using namespace htmlcxx;


using namespace matrix;

static void Usage( const char * progname )
{
    printf( "usage: %s [-p port]  [-v]\n", progname);
    printf( "    -v     crowdspider Implementation Version 1.0 (c) 2008 \n" );
    printf( "    -h     Display this help message.\n");
    printf( "    -d     Enable useful debug messages.\n");
}

/* this is used both for graceful shutdown (SIGTERM or SIGINT) and restart (SIGHUP) */
static void shutdownCrowd(int signum)
{
	/* Do a shutdown, it is really up to the module to "Do The Right Thing" here, we cancel, and wait.  */
	fprintf(stdout, "\nshutting down...\n");

	if(g_pApp != NULL)
		g_pApp->stop();

	fprintf(stdout, "waiting for:\n");
	fprintf(stdout, "shutdown complete.\n");

	/* exit normally */
	exit(0);
}

int main( int argc, char** argv )
{
	static struct option longopts[] = {
		{ "help", no_argument, NULL, 'h' },
		{ "version", no_argument, NULL, 'v' },
		{ "daemon", no_argument, NULL, 'd' },
		{ NULL, 0, NULL, '\0' },
	};

	char c;
	while ((c = getopt_long(argc, argv, "hvd", longopts, NULL)) != EOF) 
	{
		switch (c) 
		{
		case 'h':
        	Usage( argv[0] );
			exit(0);

		case 'v':
        	Usage( argv[0] );
			exit(0);

		case 'd':
        	Usage( argv[0] );
			exit(0);

		default:
        	Usage( argv[0] );
			exit(0);
		}
	}

	/* setup some signal handlers, may want to do this earlier in main() */

	{
		struct sigaction    act;
		sigemptyset(&act.sa_mask);
		/* now our graceful shutdown & restart handler */
		act.sa_handler = shutdownCrowd;
		act.sa_flags = 0;
		sigaction(SIGTERM, &act, NULL);
		sigaction(SIGINT, &act, NULL); /* deal with those knee-jerk Ctrl-C's when running from the shell */
		/* you can still forcably quit with Ctrl-\, SIGQUIT. */
		sigaction(SIGHUP, &act, NULL);
	}

    TRawServer RawServer( argc, argv );
    RawServer.start();

    for(;;)
        pause();

}





