CPARSE_DIR := $(dir $(abspath $(lastword $(MAKEFILE_LIST))))

CPARSE_SRC := $(CPARSE_DIR)src/
CPARSE_C := $(shell find $(CPARSE_SRC) -type f -name '*.c')
CPARSE_H := $(shell find $(CPARSE_SRC) -type f -name '*.h')
CPARSE_HEADERS := $(shell find $(CPARSE_DIR)include/ -type f -name '*.h')
CPARSE_HEADERS += $(CUTILS_HEADERS)

CPARSE_OUTDIR := $(BUILDDIR)bin/$(ARCH)-$(CONFIG)/cparse
CPARSE_INTDIR := $(CPARSE_OUTDIR)/int/
CPARSE_OBJ := $(patsubst $(CPARSE_SRC)%.c,$(CPARSE_INTDIR)%.o,$(CPARSE_C))
CPARSE_GCDA := $(patsubst %.o,%.gcda,$(CPARSE_OBJ))

CPARSE_INCLUDES := -I$(CPARSE_DIR)include/ $(CUTILS_INCLUDES)
CPARSE_LIBS := $(CUTILS) $(CUTILS_LIBS)

CPARSE := $(CPARSE_OUTDIR)/cparse.a

.PHONY: cparse
cparse: $(CPARSE)

$(CPARSE): $(CPARSE_OBJ)
	@mkdir -p $(@D)
	@ar rcs $@ $(CPARSE_OBJ)

$(CPARSE_INTDIR)%.o: $(CPARSE_SRC)%.c $(CPARSE_H) $(CPARSE_HEADERS)
	@mkdir -p $(@D)
	@$(TCC) -m$(BITS) -c -I$(CPARSE_SRC) $(CPARSE_INCLUDES) $(CFLAGS) -o $@ $<

CPARSE_TEST_SRC := $(CPARSE_DIR)test/
CPARSE_TEST_C := $(shell find $(CPARSE_TEST_SRC) -type f -name '*.c')
CPARSE_TEST_H := $(shell find $(CPARSE_TEST_SRC) -type f -name '*.h')
CPARSE_TEST_HEADERS := $(CPARSE_HEADERS) $(CTEST_HEADERS)

CPARSE_TEST_OUTDIR := $(BUILDDIR)bin/$(ARCH)-$(CONFIG)/cparse_test
CPARSE_TEST_INTDIR := $(CPARSE_TEST_OUTDIR)/int/
CPARSE_TEST_OBJ := $(patsubst $(CPARSE_DIR)%.c,$(CPARSE_TEST_INTDIR)%.o,$(CPARSE_TEST_C))
CPARSE_TEST_GCDA := $(patsubst %.o,%.gcda,$(CPARSE_TEST_OBJ))

CPARSE_TEST_INCLUDES := $(CPARSE_INCLUDES) $(CTEST_INCLUDES) 
CPARSE_TEST_LIBS := $(CPARSE) $(CUTILS) $(CTEST) $(CBASE)

CPARSE_TEST := $(CPARSE_TEST_OUTDIR)/cparse_test

.PHONY: cparse_test
cparse_test: $(CPARSE_TEST)
	@rm -rf $(CPARSE_GCDA) $(CPARSE_TEST_GCDA)
	@$(CPARSE_TEST)

$(CPARSE_TEST): $(CPARSE_TEST_OBJ) $(CPARSE_TEST_LIBS)
	@mkdir -p $(@D)
	@$(TCC) -m$(BITS) $(LDFLAGS) -o $@ $(CPARSE_TEST_OBJ) $(patsubst %,-L%,$(dir $(CPARSE_TEST_LIBS))) $(patsubst %,-l:%,$(notdir $(CPARSE_TEST_LIBS)))

$(CPARSE_TEST_INTDIR)%.o: $(CPARSE_DIR)%.c $(CPARSE_TEST_H) $(CPARSE_TEST_HEADERS)
	@mkdir -p $(@D)
	@$(TCC) -m$(BITS) -c -I$(CPARSE_TEST_SRC) $(CPARSE_TEST_INCLUDES) $(CFLAGS) -o $@ $<
