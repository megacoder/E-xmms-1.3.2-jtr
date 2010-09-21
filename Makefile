CFLAGS = `glib-config --cflags` `xmms-config --cflags` -DEROOT=\"${EROOT}\"
LDFLAGS= `xmms-config --libs` -Wl,'-rpath=/opt/xmms/lib'
LDLIBS = -lxmms -lGL
GCC = gcc -g

mkinstalldirs = $(SHELL) mkinstalldirs
ICONS = \
xmms-icon1.xpm \
xmms-icon2.xpm \
xmms-icon3.xpm \
xmms-icon4.xpm \
xmms-icon5.xpm \
xmms-icon6.xpm \
xmms-icon7.xpm \
xmms-icon8.xpm \
xmms-icon9.xpm \
xmms-icon10.xpm \
xmms-icon11.xpm \
xmms-icon12.xpm \
xmms-icon13.xpm \
xmms-icon14.xpm \
xmms-icon15.xpm \
xmms-icon16.xpm \
xmms-icon17.xpm \
xmms-icon18.xpm \
xmms-icon19.xpm \
xmms-icon20.xpm \
xmms-icon21.xpm \
xmms-icon22.xpm \
xmms-icon23.xpm \
xmms-icon24.xpm \
xmms-icon25.xpm \
xmms-icon26.xpm \
xmms-icon27.xpm \
xmms-icon28.xpm \
xmms-icon29.xpm \
xmms-icon30.xpm \
xmms-icon31.xpm \
xmms-icon32.xpm \
xmms-icon33.xpm \
xmms-icon34.xpm \
xmms-icon35.xpm \
xmms-icon36.xpm \
xmms-init.xpm

all : E-xmms.epplet

E-xmms.epplet : E-xmms.c
	${GCC} E-xmms.c -lepplet ${LDFLAGS} ${CFLAGS} -o E-xmms.epplet ${LDLIBS}
	strip E-xmms.epplet

install : E-xmms.epplet
	install -m 755 E-xmms.epplet /usr/bin/E-xmms.epplet
	$(mkinstalldirs) ${EROOT}/epplet_data/E-xmms
	@for file in $(ICONS); do \
	    echo install -m 644 icons/$$file ${EROOT}/epplet_data/E-xmms/$$file; \
	    install -m 644 icons/$$file ${EROOT}/epplet_data/E-xmms/$$file; \
	done


uninstall :
	rm /usr/bin/E-xmms.epplet
	rm -rf ${EROOT}/epplet_data/E-xmms

clean :
	rm -f E-xmms.epplet E-xmms.o

