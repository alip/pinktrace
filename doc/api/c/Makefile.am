SUBDIRS= .
MAINTAINERCLEANFILES = \
		       Makefile.in \
		       doxygen.conf
CLEANFILES= *~ c/* pinktrace.tag

doxygen_files=
EXTRA_DIST= doxygen.conf.in doxygen-filter-git.sh $(doxygen_files)

if HAVE_DOXYGEN
all-local: doxygen

doxygen:
	test -n "$(HAVE_DOT)" && $(MAKE) c-with-dot || $(MAKE) c

c: doxygen.conf $(doxygen_files)
	$(AM_V_GEN)
	$(AM_V_at)doxygen doxygen.conf

c-with-dot:
	$(AM_V_GEN)
	$(AM_V_at)$(SED) '/^HAVE_DOT/s:NO:YES:' doxygen.conf | doxygen -

install-data-local:
	mkdir -p $(DESTDIR)/$(htmldir)/api
	cp -R c $(DESTDIR)/$(htmldir)/api
else
doxygen:
	@echo "You don't have doxygen installed!"
	exit 1
endif # HAVE_DOXYGEN

clean-local:
	rm -fr c

uninstall-local:
	rm -fr $(DESTDIR)/$(htmldir)/api/c

.PHONY: doxygen
