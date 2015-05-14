# top level makefile
MKDIR    = mkdir -p
INSTALL = /usr/bin/install -c 
LINK = ln -s

all: 
	for dir in libhtmlparser libstream libchseg libmtd libui build ; do \
		cd $$dir && make && cd ..; \
	done

clean:
	for dir in libhtmlparser libstream libchseg libui libmtd build; do \
		cd $$dir && make clean && cd ..; \
	done

install:  
	$(INSTALL) bin/*    			$(DESTDIR)/usr/local/wdget/bin
	$(INSTALL) libmtd/lib*			$(DESTDIR)/usr/local/wdget/lib
	$(INSTALL) libhtmlparser/lib*   $(DESTDIR)/usr/local/wdget/lib
	$(INSTALL) libstream/lib* 		$(DESTDIR)/usr/local/wdget/lib
	$(INSTALL) -m 644 config/*  	$(DESTDIR)/usr/local/wdget/config
	$(INSTALL) -m 644 images/*   	$(DESTDIR)/usr/local/wdget/images
	$(INSTALL) -m 644  lang/*       $(DESTDIR)/usr/local/wdget/lang
	$(INSTALL) -m 644 config/wdget.desktop $(DESTDIR)/usr/share/applications/wdget.desktop
