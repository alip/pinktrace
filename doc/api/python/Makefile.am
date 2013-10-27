SUBDIRS= .
EXTRA_DIST= epydoc.css
EPYDOC_FLAGS= \
	      --debug \
	      --verbose \
	      --name "pinktrace" \
	      --url "$(SITE_URI_HOME)api/python" \
	      --no-frames \
	      --css $(srcdir)/epydoc.css

if ENABLE_PYTHON_DOC
all-local: epydoc

epydoc: python

python:
	$(AM_V_GEN)
	$(AM_V_at)rm -fr python
	$(AM_V_at)mkdir -p python
	$(PYTHON) $(EPYDOC) $(EPYDOC_FLAGS) -o $@ $(top_builddir)/python/pinktrace

install-data-local:
	mkdir -p $(DESTDIR)$(htmldir)/api
	cp -R python $(DESTDIR)$(htmldir)/api

else
epydoc:
	@echo "Can not build Python API docs"
	exit 0
endif # ENABLE_PYTHON_DOC

clean-local:
	rm -fr python

uninstall-local:
	rm -fr $(DESTDIR)$(htmldir)/api/python

.PHONY: epydoc
