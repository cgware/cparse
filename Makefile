BUILDDIR := $(CURDIR)/
SRCDIR := $(CURDIR)/

TCC := $(CC)

ifeq ($(ARCH), x64)
BITS := 64
endif
ifeq ($(ARCH), x86)
BITS := 32
endif

ifeq ($(CONFIG), Debug)
CFLAGS := -Wall -Wextra -Werror -pedantic -O0 -ggdb -coverage
LDFLAGS := -coverage
endif
ifeq ($(CONFIG), Release)
CFLAGS := -Wall -Wextra -Werror -pedantic
LDFLAGS :=
endif

$(SRCDIR)deps/cbase/cbase.mk:
ifeq ($(DEV), true)
	git clone git@github.com:cgware/cbase.git $(SRCDIR)deps/cbase
else
	git clone https://github.com/cgware/cbase.git $(SRCDIR)deps/cbase
endif

include $(SRCDIR)deps/cbase/cbase.mk

$(SRCDIR)deps/ctest/ctest.mk:
ifeq ($(DEV), true)
	git clone git@github.com:cgware/ctest.git $(SRCDIR)deps/ctest
else
	git clone https://github.com/cgware/ctest.git $(SRCDIR)deps/ctest
endif

include $(SRCDIR)deps/ctest/ctest.mk

$(SRCDIR)deps/cutils/cutils.mk:
ifeq ($(DEV), true)
	git clone git@github.com:cgware/cutils.git $(SRCDIR)deps/cutils
else
	git clone https://github.com/cgware/cutils.git $(SRCDIR)deps/cutils
endif

include $(SRCDIR)deps/cutils/cutils.mk

include $(SRCDIR)cparse.mk

.PHONY: test
test: cparse_test

.PHONY: coverage
coverage: test
	@lcov -q -c -o $(BUILDDIR)/bin/lcov.info -d $(CPARSE_OUTDIR)
ifeq ($(SHOW), true)
	@genhtml -q -o $(BUILDDIR)/report/coverage $(BUILDDIR)/bin/lcov.info 
	@open $(BUILDDIR)/report/coverage/index.html
endif

libs/lib.a:
libs2/lib2.a:

LIBS := libs/lib.a libs2/lib2.a

build: $(LIBS)
	gcc main.c $(patsubst %,-L%,$(dir $(LIBS))) $(patsubst %,-l:%,$(notdir $(LIBS)))
