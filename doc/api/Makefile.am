SUBDIRS= c python .

.PHONY: upload
upload:
	$(MAKE) -C c $@
	$(MAKE) -C python $@

.PHONY: doxygen
doxygen:
	$(MAKE) -C c $@

.PHONY: pydoctor
pydoctor:
	$(MAKE) -C python $@
