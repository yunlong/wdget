all: htmlparser tests uritests
htmlparser: htmlparser.cc 
	g++ -g -o htmlparser htmlparser.cc -L../ -lhtmlparser -I../ -D_UNIX 
uritests: uritests.cc 
	g++ -g -o uritests uritests.cc -L../ -lhtmlparser -I../ -D_UNIX
tests: tests.cc 
	g++ -g -o tests tests.cc -L../ -lhtmlparser -I../ -D_UNIX 
clean:
	rm -f htmlparser uritests tests 
