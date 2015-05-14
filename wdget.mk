# top level makefile
MKDIR    = mkdir -p
INSTALL = /usr/bin/install -c 
LINK = ln -s
INSTALL_DIR = /usr/local/wdget

all: 
	for dir in libhtmlparser libstream libchseg libmtd libui build ; do \
		cd $$dir && make && cd ..; \
	done

clean:
	for dir in libhtmlparser libstream libchseg libui libmtd build; do \
		cd $$dir && make clean && cd ..; \
	done

install:  
	$(MKDIR) $(INSTALL_DIR)/bin
	$(MKDIR) $(INSTALL_DIR)/lib
	$(MKDIR) $(INSTALL_DIR)/config
	$(MKDIR) $(INSTALL_DIR)/images
	$(MKDIR) $(INSTALL_DIR)/lang
	$(MKDIR) $(INSTALL_DIR)/doc

	$(INSTALL) bin/*    $(INSTALL_DIR)/bin
	$(INSTALL) config/* $(INSTALL_DIR)/config
	$(INSTALL) -m 644 images/* $(INSTALL_DIR)/images
	$(INSTALL) -m 644  lang/*   $(INSTALL_DIR)/lang
	$(INSTALL) libmtd/lib* $(INSTALL_DIR)/lib
	$(INSTALL) libhtmlparser/lib* $(INSTALL_DIR)/lib
	$(INSTALL) libstream/lib* $(INSTALL_DIR)/lib
	$(INSTALL) -m 644 config/wdget.desktop /usr/share/applications/wdget.desktop
	grep -v "/usr/local/wdget/lib" /etc/ld.so.conf > /tmp/.ld.so.conf
	echo "/usr/local/wdget/lib" >> /tmp/.ld.so.conf
	mv /tmp/.ld.so.conf /etc/ld.so.conf
	ldconfig


