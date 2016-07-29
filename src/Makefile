MODULES_DIR=$(DESTDIR)/usr/lib64/httpd/modules

INCLUDES=-I/usr/include/shift
LIBS=-lcastorns -lcastorclient -lcastorrfio

APXS=/usr/sbin/apxs
INSTALL=/usr/bin/install

all: mod_dav_castor.so

mod_dav_castor.so:
	$(APXS) -c -o mod_dav_castor.so $(INCLUDES) *.c $(LIBS)
	cp .libs/mod_dav_castor.so .

install:
	$(INSTALL) -d $(MODULES_DIR)
	$(INSTALL) mod_dav_castor.so $(MODULES_DIR)

clean:
	rm -rf *.o *.lo *.slo *.la *.so .libs

.PHONY: all mod_dav_castor.so install clean
