SUBDIRS= api .

.PHONY: upload
upload:
	$(MAKE) -C api $@

.PHONY: doxygen
doxygen:
	$(MAKE) -C api $@

.PHONY: pydoctor
pydoctor:
	$(MAKE) -C api $@

.PHONY: site-check
site-check:
	hugo server

.PHONY: site
site: $(top_srcdir)/pinktrace-www.tar.gz

%.toml: %.toml.in
	$(AM_V_GEN)
	$(AM_V_at)$(SED) \
		-e "s,#VERSION#,$(PINK_VERSION_FULL)," \
		$< > $@

$(top_srcdir)/pinktrace-www.tar.gz: config.toml
	hugo
	tar -C public -cvzf $@ .
