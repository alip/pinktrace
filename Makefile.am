ACLOCAL_AMFLAGS= -I m4 ${ACLOCAL_FLAGS}
AUTOMAKE_OPTIONS= dist-bzip2 no-dist-gzip std-options subdir-objects foreign
AM_MAKEFLAGS= --no-print-directory

CLEANFILES= *~
MAINTAINERCLEANFILES= \
		      Makefile.in \
		      configure \
		      aclocal.m4 \
		      config.h \
		      config.h.in \
		      INSTALL

EXTRA_DIST= \
	    autogen.sh

dist_doc_DATA= \
	       COPYING \
	       LGPL-2.1-or-later \
	       README.markdown

# FIXME: --enable-python-doc requires epydoc which is no longer maintained.
DISTCHECK_CONFIGURE_FLAGS= \
			   --enable-maintainer-mode \
			   --enable-doxygen \
			   --enable-python \
			   --enable-ipv6
# "make distcheck" builds the dvi target, so use it to check that the
# documentation is built correctly.
dvi:
	$(MAKE) SPHINXOPTS_EXTRA=-W html

BUILT_SOURCES = .version
EXTRA_DIST+= \
	     .version \
	     ChangeLog \
	     CREDITS.in \
	     maint/gitlog-to-changelog
$(srcdir)/.version:
	$(AM_V_GEN)echo $(VERSION) > $@-t && mv $@-t $@

if MAINTAINER_MODE

# For v1.0.0, do:
# gen_changelog_start_date = 2015-01-12 12:00
# This includes all and we're fine with it.
gen_changelog_start_date = 1999-08-17 03:02
$(srcdir)/ChangeLog: $(srcdir)/maint/gitlog-to-changelog $(srcdir)/Makefile.in \
		     $(srcdir)/.version
	@rm -f $@.new
	(cd $(srcdir); \
	 ./maint/gitlog-to-changelog --append-dot \
	   --since='$(gen_changelog_start_date)'; \
	 echo; echo; echo '' \
	) > $@.new
	chmod 444 $@.new
	mv -f $@.new $@

$(srcdir)/CREDITS: $(srcdir)/CREDITS.in $(srcdir)/.mailmap \
		   $(srcdir)/Makefile.in $(srcdir)/.version
	$(AM_V_GEN)							\
	(								\
	  D="$(D)";							\
	  export D;							\
	  cd $(srcdir);							\
	  sed '/^##/,$$d' CREDITS.in;					\
	  sed -n '1,/^##>/d; s/   */\t/; s/^./&/p' CREDITS.in		\
	    | maint/gen-contributors-list.sh - -e HEAD --initial	\
	    | sed 's/\t/ /g; s/^/\t/'					\
	) > $@-t && mv $@-t $@

export TAR_OPTIONS = --owner=0 --group=0 --numeric-owner --mode=go-w,go+rX

endif

TAR_FILE= $(PACKAGE)-$(VERSION).tar.bz2
SHA1_FILE= $(TAR_FILE).sha1sum
GPG_FILE= $(SHA1_FILE).asc

.PHONY: doxygen
doxygen: all
	$(MAKE) -C doc $@

.PHONY: epydoc
epydoc: all
	$(MAKE) -C doc $@

$(SHA1_FILE): dist
	@echo "SHA1 $(TAR_FILE)"
	sha1sum $(TAR_FILE) > $(SHA1_FILE)

$(GPG_FILE): $(SHA1_FILE)
	@echo "SIGN $(SHA1_FILE)"
	gpg --detach-sign --armor $(SHA1_FILE)

.PHONY: upload
upload:
	$(MAKE) -C doc upload

.PHONY: upload-release
upload-release: $(SHA1_FILE) $(GPG_FILE)
	rsync --partial --progress -ave ssh $(TAR_FILE) $(SHA1_FILE) $(GPG_FILE) dev.exherbo.org:public_html/pinktrace/release

SUBDIRS= pinktrace python doc pkg-config .
