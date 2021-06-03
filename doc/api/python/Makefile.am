SUBDIRS= .
PYDOCTOR_FLAGS= \
	      --verbose \
	      --project-name "PinkTrace" \
	      --project-url "$(SITE_URI_HOME)api/python"

if ENABLE_PYTHON_DOC
all-local: pydoctor

pydoctor: python

python:
	$(AM_V_GEN)
	$(AM_V_at)rm -fr python
	$(AM_V_at)mkdir -p python
	$(PYDOCTOR) \
		--introspect-c-modules \
		--make-html $(PYDOCTOR_FLAGS) \
		--html-output $@ \
		$(top_builddir)/python/pinktrace || true
	test -e python/index.html || rmdir python
	test -e python/index.html

install-data-local:
	mkdir -p $(DESTDIR)$(htmldir)/api
	cp -R python $(DESTDIR)$(htmldir)/api
upload: pydoctor
	rsync --partial --progress --delete -ave ssh python/ dev.exherbo.org:public_html/pinktrace/api/python/
else
pydoctor:
	@echo "Can not build Python API docs"
	exit 0
upload:
	@echo "You haven't enabled Python API doc generation!"
	exit 0
endif # ENABLE_PYTHON_DOC

clean-local:
	rm -fr python

uninstall-local:
	rm -fr $(DESTDIR)$(htmldir)/api/python

.PHONY: pydoctor upload
