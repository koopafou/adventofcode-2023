all:
	echo need a number

TARGETS=$(shell cd src/day && ls)

PKGLIST=fmt
CXXFLAGS=-std=c++20 -Wall -Wextra

define MKTARGET =

SRC_$(1)=$(shell cd src/day/$(1) && ls *.cc)

build/$(1)$(2)/dep/%.cc.d: src/day/$(1)/%.cc
	mkdir -p `dirname $$@`
	g++ -Isrc $(3) $$(CXXFLAGS) -MM -MG -MT $$@ -MF $$@ $$<

build/$(1)$(2)/Makefile-deps: $$(addprefix build/$(1)$(2)/dep/, $$(addsuffix .d, $$(SRC_$(1))))
	cat $$^ > $$@

-include build/$(1)$(2)/Makefile-deps

build/$(1)$(2)/obj/%.cc.o: src/day/$(1)/%.cc build/$(1)$(2)/dep/%.cc.d
	mkdir -p `dirname $$@`
	g++ -Isrc $(3) $$(CXXFLAGS) `pkg-config --cflags $$(PKGLIST)` -c $$< -o $$@

build/bin/$(1)$(2): $$(addprefix build/$(1)$(2)/obj/, $$(addsuffix .o, $$(SRC_$(1))))
	mkdir -p `dirname $$@`
	g++ -Isrc $(3) $$(CXXFLAGS) `pkg-config --libs $$(PKGLIST)` $$< -o $$@

$(1)$(2): build/bin/$(1)$(2) src/day/$(1)/input
	@echo ===========
	@echo
	@./build/bin/$(1)$(2) < src/day/$(1)/input | tee $@

endef

$(foreach TARGET,$(TARGETS), $(eval $(call MKTARGET,$(TARGET),,)))
$(foreach TARGET,$(TARGETS), $(eval $(call MKTARGET,$(TARGET),.g,-g)))
