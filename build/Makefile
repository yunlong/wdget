all: httpcli dspider joinblk readLog

httpcli: httpcli.cpp 
	g++ -g -o httpcli httpcli.cpp -L ../libmtd -lmtd  -I ../libmtd -D_UNIX  \
	-L ../libhtmlparser -lhtmlparser -I ../libhtmlparser -lssl 	\
	-L ../libstream	-lstream -I ../libstream -D_FILE_OFFSET_BITS=64
dspider: dspider.cpp
	g++ -g -o dspider dspider.cpp -lmtd -L ../libmtd -I ../libmtd  -D_UNIX \
	-L ../libhtmlparser -lhtmlparser -I ../libhtmlparser -lssl 	\
	-L ../libstream	-lstream -I ../libstream -D_FILE_OFFSET_BITS=64

joinblk: joinblk.c
	g++ -o joinblk joinblk.c -D_FILE_OFFSET_BITS=64
readLog: readLog.c 
	g++ -o readLog readLog.c -D_FILE_OFFSET_BITS=64
clean:
	rm -f httpcli dspider *.o  joinblk readLog dget
