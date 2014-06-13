#
#
#   Authors:
#    Lars Fenneberg		<lf@elemental.net>	 
#
#   This software is Copyright 1996,1997 by the above mentioned author(s), 
#   All Rights Reserved.
#
#   The license which is distributed with this software in the file COPYRIGHT
#   applies to this software. If your distribution is missing this file, you
#   may request it from <reubenhwk@gmail.com>.
#


# $(CC) $(DEFS) -I. [$(mumble_CPPFLAGS)|$(AM_CPPFLAGS)] $(CPPFLAGS) [$(mumble_CFLAGS)|$(AM_CFLAGS)] $(CFLAGS)
# mumble_CPPFLAGS override AM_CPPFLAGS
# CPP is C PreProcessor

AUTOMAKE_OPTIONS = 1.2 foreign subdir-objects

###  Compiler flags and such ###

DEFS = \
	-DINET6=1 \
	-DLOG_FACILITY=$(LOG_FACILITY) \
	-DPATH_RADVD_CONF=\"$(PATH_RADVD_CONF)\" \
	-DPATH_RADVD_LOG=\"$(PATH_RADVD_LOG)\" \
	-DPATH_RADVD_PID=\"$(PATH_RADVD_PID)\" \
	-DVERSION=\"$(VERSION)\"

ENABLE_WARNINGS = \
	-Wall \
	-Wcast-qual \
	-Wmissing-declarations \
	-Wmissing-prototypes \
	-Wpointer-arith \
	-Wstrict-prototypes

DISABLE_WARNINGS = \
	-Wno-strict-aliasing \
	-Wno-cast-align \
	-Wno-strict-overflow

AM_CFLAGS = \
	-fno-strict-aliasing \
	-fstack-protector \
	$(ENABLE_WARNINGS) \
	$(DISABLE_WARNINGS)

### CPP is C PreProcessor ###

AM_YFLAGS = -d

radvd_CPPFLAGS = \
	@DAEMON_CFLAGS@

noinst_LIBRARIES = libradvd-parser.a

radvd_LDADD = \
	libradvd-parser.a \
	@CONDITIONAL_SOURCES@ \
	@DAEMON_LIBS@

### Programs and sources ###


libradvd_parser_a_SOURCES = \
	gram.h \
	gram.y \
	scanner.l

sbin_PROGRAMS = \
	radvd \
	radvdump

EXTRA_radvd_SOURCES = \
	device-bsd44.c \
	device-linux.c \
	netlink.c \
	netlink.h \
	privsep-linux.c

radvd_SOURCES = \
	defaults.h \
	includes.h \
	log.c \
	log.h \
	pathnames.h \
	radvd.h \
	recv.c \
	socket.c \
	util.c \
	device-common.c \
	interface.c \
	process.c \
	radvd.c \
	send.c \
	timer.c

radvdump_SOURCES = \
	defaults.h \
	includes.h \
	log.c \
	log.h \
	pathnames.h \
	radvd.h \
	recv.c \
	socket.c \
	util.c \
	radvdump.c

### coverage ###

cov: check
	gcov *.c *.h *.l *.y
	lcov --capture --directory . --output-file coverage.info
	genhtml coverage.info --output-directory cov

### man pages ###

SUFFIXES = .man

.man:
	$(AM_V_GEN)$(SED) \
		-e 's,@''VERSION''@,$(VERSION),g' \
		-e 's,@''sbindir''@,$(sbindir),g' \
		-e 's,@''PATH_RADVD_CONF''@,$(PATH_RADVD_CONF),g' \
		-e 's,@''PATH_RADVD_PID''@,$(PATH_RADVD_PID),g' \
		-e 's,@''PATH_RADVD_LOG''@,$(PATH_RADVD_LOG),g' \
		-e 's,@''LOG_FACILITY''@,$(LOG_FACILITY),g' \
		$< > $@

man_MANS = \
	radvd.8 \
	radvd.conf.5 \
	radvdump.8


### build support ###

indent:
	indent -linux -l125 *.c *.h

EXTRA_DIST = \
	CHANGES \
	COPYRIGHT \
	INTRO.html \
	copyright.blurb \
	radvd.8.man \
	radvd.conf.5.man \
	radvd.conf.example \
	radvdump.8.man \
	redhat/radvd.conf.empty \
	redhat/radvd.init \
	redhat/radvd.spec \
	redhat/radvd.sysconfig \
	redhat/radvd-tmpfs.conf

EXTENSIONS = gz bz2 lz lzma xz tarZ shar zip
HASHES = md5 sha1 sha256

sign:
	$(AM_V_GEN)for e in $(EXTENSIONS); do \
		if [ -f radvd-$(VERSION).tar.$$e ]; then \
			for h in $(HASHES); do \
				$${h}sum radvd-$(VERSION).tar.$$e > radvd-$(VERSION).tar.$$e.$$h; \
			done; \
			gpg -sba radvd-$(VERSION).tar.$$e; \
		fi; \
	done

verify:
	$(AM_V_GEN)for e in $(EXTENSIONS); do \
		if [ -f radvd-$(VERSION).tar.$$e ]; then \
			for h in $(HASHES); do \
				$${h}sum -c radvd-$(VERSION).tar.$$e.$$h; \
			done; \
			gpg --verify radvd-$(VERSION).tar.$$e.asc; \
		fi; \
	done

html:
	$(AM_V_GEN)echo "<ul>"; \
	for e in $(EXTENSIONS); do \
		if [ -f radvd-$(VERSION).tar.$$e ]; then \
			echo "<li>"; \
			echo "<a href=\"dist/radvd-$(VERSION).tar.$$e\">radvd-$(VERSION).tar.$$e</a>"; \
			for h in $(HASHES); do \
				echo "<a href=\"dist/radvd-$(VERSION).tar.$$e.$$h\">[$$h]</a>"; \
			done; \
			echo "<a href=\"dist/radvd-$(VERSION).tar.$$e.asc\">[GPG]</a>"; \
			echo "</li>"; \
		fi; \
	done; \
	echo "</ul>"

packages:
	$(MAKE) dist-bzip2
	$(MAKE) dist-gzip
	$(MAKE) dist-xz
	$(MAKE) sign
	$(MAKE) verify
	$(MAKE) html


dist-hook:
	rm -f $(distdir)/gram.c
	rm -f $(distdir)/gram.h
	rm -f $(distdir)/scanner.c

CLEANFILES = \
	radvd.8 \
	radvd.conf.5 \
	radvdump.8 \
	gram.c \
	gram.h \
	gram.dot \
	scanner.c \
	scanner.h \
	gram.dot

### Explicit Dependencies ###

radvd_DEPENDENCIES = \
	@CONDITIONAL_SOURCES@ \
	libradvd-parser.a

gram.h: gram.c
gram.c: gram.y
	$(AM_V_GEN) ${YACC} --defines=gram.h --graph=gram.dot --output=gram.c gram.y

scanner.h: scanner.c
scanner.c: scanner.l
	$(AM_V_GEN) ${LEX} --header-file=scanner.h --outfile=scanner.c scanner.l

radvd-gram.o gram.o: gram.c scanner.h
radvd-scanner.o scanner.o: scanner.c gram.h

