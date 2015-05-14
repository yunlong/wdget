#include "ParserDom.h"
#include "utils.h"
#include <fstream>
#include <stdexcept>
#include <iostream>
#include <cstdlib>
#include <cstdio>
#include <vector>

#define VERSION "0.6"

using namespace std;
using namespace htmlcxx;

typedef struct{
	int offset;
	int len;
}TagInfo;	

void usage(string prg) 
{
	cerr << "usage:\t" << prg << " [-h] [-V] file.html [file.css]" << endl;
	return;
}

void usage_long(string prg) 
{
	usage(prg);
	cerr << "  -V\t print version number and exit" << endl;
	cerr << "  -h\t print this help text" << endl;
	cerr << "  -C\t disable css parsing" << endl;
	return;
}

int main(int argc, char **argv) 
{
	tree<HTML::Node> dom;
	string css_code;
	try 
	{
		while (1) 
		{
			signed char c = getopt(argc, argv, "hVC");	
			if(c == -1) break;
			switch(c) 
			{
			case 'h':
				usage_long(argv[0]);
				exit(0);
				break;
			case 'V':
				cerr << VERSION << endl;
				exit(0);
			default:
				usage(argv[0]);
				exit(1);
				break;
			}
		}

		if (argc != optind + 1 && argc != optind + 2) 
		{
			usage(argv[0]);
			exit(1);
		}

		ifstream file(argv[optind]);
		if (!file.is_open()) 
		{
			cerr << "Unable to open file " << argv[optind] << endl;
			exit(1);
		}
		string html;

		while (1)
		{
			char buf[BUFSIZ];
			file.read(buf, BUFSIZ);
			if(file.gcount() == 0) 
			{
				break;
			}
			html.append(buf, file.gcount());
		}
		file.close();

		HTML::ParserDom parser;
		parser.parse(html);
		dom = parser.getTree();
//		cout << dom << endl;

		tree<HTML::Node>::iterator it = dom.begin();
  		tree<HTML::Node>::iterator end = dom.end();
  		for (; it != end; ++it)
  		{
  			if (it->tagName() == "id")
  			{
				it++;
  				if ((!it->isTag()) && (!it->isComment()))
  					cout << it->text() << endl;
  			}	
  		}

	/***
		int len = 0;
		std::vector<TagInfo> vt;
  		for (; it != end; ++it)
  		{
  			if ( it->tagName() == "script" || it->tagName() == "style" )
  			{
			//	cout << (++it)->text() << endl;
				TagInfo ti;
				ti.offset = it->offset();
				ti.len = it->length();
				vt.push_back(ti);
  			}	
  		}
	***/

	/****
		string htmlText;
		int start = 0;
		for(int i = 0; i < vt.size(); i++)
		{
		//	cout << vt[i].offset << ":" << vt[i].len << endl;
			htmlText.append(html, start, vt[i].offset - start);
			start = vt[i].offset + vt[i].len;
		}
		if(start < html.size())
			htmlText.append(html, start, html.size() - start);
		
		cout << htmlText << endl;
	***/

	/***
		parser.parse(htmlText);
		dom = parser.getTree();

		it = dom.begin();
 		end = dom.end();
  		for (; it != end; ++it)
  		{
  			if (it->tagName() == "a")
  			{
  				it->parseAttributes();
				if(it->attribute("href").second.substr(0, 10) == "javascript") continue; 
				if(it->attribute("href").second.substr(0, 6) == "mailto") continue; 
				if(it->attribute("href").second.substr(0, 5) == "https") continue; 
  				cout << it->attribute("href").second << endl;
  			}	
  		}
	***/
  		//Dump all text of the document
		/***
		it = dom.begin();
 		end = dom.end();
  		for (; it != end; ++it)
  		{
  			if ((!it->isTag()) && (!it->isComment()))
  				cout << it->text();
  		}
		***/
	} 
	catch (exception &e) 
	{
		cerr << "Exception " << e.what() << " caught" << endl;
		exit(1);
	} 
	catch (...) 
	{
		cerr << "Unknow exception caught " << endl;
	}
}

