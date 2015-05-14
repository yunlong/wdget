#include <getopt.h>
#include <string>
#include <cstdlib>
#include <iostream>
#include <vector>

#include "Uri.h"

using namespace std;
using namespace htmlcxx;

//#define DEBUG
#include "debug.h"

#define myassert(x) \
do {\
	if(!(x)) {\
		fprintf(stderr, "Test at %s:%d failed!\n", __FILE__, __LINE__);\
		exit(1);\
	}\
} while(0)

void usage(const char *prg) 
{
	cerr << "usage: " <<  prg << " [-d maxdepth] uri1 [uri2 ...]"  << endl;
}
int main(int argc, char **argv) 
{

	try 
	{
		int flags = Uri::REMOVE_WWW_PREFIX | Uri::REMOVE_TRAILING_BAR | Uri::REMOVE_DEFAULT_FILENAMES | Uri::REMOVE_SCHEME;

		unsigned int maxDepth = UINT_MAX;
		vector<string> tests;
//		tests.push_back("http://longtails@163.com:123456@www.slashdot.org");
//		tests.push_back("http://www.embratel.net.br:80/Embratel02/cda/portal/0,2297,RE_P_371,00.html");
		tests.push_back("Https://www.amazon.cn/mn/detailApp?ref=BR&uid=478-7900914-8125608&prodid=bkbk852094");
//		tests.push_back("http://longtails@163.com:123456@www.embratel.net.br:80/joão.html");
//		tests.push_back("http://www.embratel.net.br:80/superjoão! .html");
		//tests.push_back("ftp://ftp.kernel.org/pub/linux/kernel/v1.0/linux-1.0.tar.gz");
		tests.push_back("ftp://ftp.kernel.org/");
		while(1) 
		{
			signed char c = getopt(argc, argv, "hd:");
			if(c == -1) break;
			switch(c) 
			{
			case 'd':
				{ 
					char *end;
					maxDepth = strtoul(optarg, &end, 10);
					if (*end != 0) 
					{
						usage(argv[0]);
						exit(1);
					}
				}
				break;
			case 'h':
				usage(argv[0]);
				exit(0);
			default:
				usage(argv[0]);
				exit(1);
			}
		}

		/*
		if(optind < argc) 
		{
			for(int i = optind; i < argc; ++i) 
			{
				Uri uri(argv[i]);
				DEBUGP("Created uri object for %s\n", argv[i]);
				cerr << uri.unparse(flags) << endl;
				cerr << " -> " << uri.canonicalHostname(maxDepth) <<  endl;
			}
		} 
		else 
		{
			
			for(unsigned int i = 0; i < tests.size(); ++i) 
			{
				Uri uri(tests[i].c_str());
				uri.user();
				DEBUGP("Created uri object\n");
				cerr << uri.unparse(flags) << " -> " << uri.canonicalHostname() <<  endl;
			}
			for (unsigned int i = 0; i < tests.size(); ++i)
			{
				std::string e = Uri::encode(tests[i]);
				cerr << "Encoded: " << e << endl;
				cerr << "Decoded: " << Uri::decode(e) << endl;
				myassert(Uri::decode(e) == tests[i]);
			}
		}
		*/

		for(unsigned int i = 0; i < tests.size(); ++i) 
		{
//			std::string e = Uri::encode(tests[i]);
//			Uri uri(e);
			Uri uri(tests[i]);

			cout <<uri.scheme() << "\n" << uri.hostname() << '\n' << uri.port() << '\n' <<  uri.path() << '\n' << uri.query() << '\n' << uri.fragment() << endl;
		//	cout <<uri.user() << "\n" << uri.password() << '\n' << endl; 
			cerr << uri.unparse(flags) << " -> " << uri.canonicalHostname() <<  endl;
		}
		
	} 
	catch (exception &e) 
	{
		cerr << e.what() << endl;
		exit(1);
	} 
	catch (...) 
	{
		cerr << "Unknow object thrown" << endl;
	}
	exit(0);
}
