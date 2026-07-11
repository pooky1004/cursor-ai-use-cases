# Y2K38 ABI / liby2k38safe — host or ELDK cross build
#
# Host:
#   make
#   make check
#
# Cross (ELDK 3.1.1 example):
#   make cross CROSS=ppc_82xx- SYSROOT=/opt/eldk/ppc_82xx
#   make stage  CROSS=ppc_82xx- SYSROOT=/opt/eldk/ppc_82xx DESTDIR=./staging
#   make install-sysroot CROSS=ppc_82xx- SYSROOT=/opt/eldk/ppc_82xx
#
# Or:
#   ./scripts/cross-build-eldk.sh
#   ./scripts/deploy-board.sh user@board:/opt/y2k38

CROSS     ?=
CC        := $(CROSS)gcc
AR        := $(CROSS)ar
RANLIB    := $(CROSS)ranlib
STRIP     := $(CROSS)strip

ROOT      := $(CURDIR)
INCDIR    := $(ROOT)/include
LIBDIR    := $(ROOT)/lib
PREFIX    ?= /usr/local
DESTDIR   ?=
STAGE     ?= $(ROOT)/staging

# Static link of lib into daemons on cross builds (safer for old boards
# without LD_LIBRARY_PATH setup). Override: make LINK_STATIC=0
LINK_STATIC ?= 1

CPPFLAGS  := -I$(INCDIR)
CFLAGS    ?= -O2 -Wall -Wextra -std=c99
LDFLAGS   :=
LIBS      :=

ifneq ($(SYSROOT),)
  CPPFLAGS += --sysroot=$(SYSROOT)
  LDFLAGS  += --sysroot=$(SYSROOT)
endif

ifeq ($(LINK_STATIC),1)
  Y2K38_LIB := $(LIBDIR)/liby2k38safe.a
else
  Y2K38_LIB := -L$(LIBDIR) -ly2k38safe
  LDFLAGS  += -Wl,-rpath,/usr/lib
endif

LIB_SRCS  := $(LIBDIR)/y2k38_time.c \
             $(LIBDIR)/y2k38_eventlog.c \
             $(LIBDIR)/y2k38_offset.c
LIB_OBJS  := $(LIB_SRCS:.c=.o)

DEMO_BINS := examples/broken/y2k38_broken \
             examples/broken/broken_daemon_pipeline \
             examples/fixed/y2k38_fixed \
             examples/fixed/kernel_offset_demo \
             examples/fixed/y2k38_threads_demo \
             examples/fixed/y2k38_autowrap_demo

DAEMON_BINS := daemons/daemon_a/daemon_a \
               daemons/daemon_b/daemon_b

TOOL_BINS := tools/y2k38_offsetctl

.PHONY: all clean install install-sysroot stage demos daemons tools lib \
        check check-offset cross help

all: lib demos daemons tools

help:
	@echo "Targets: all lib demos daemons tools check check-offset"
	@echo "         cross stage install install-sysroot clean"
	@echo "Vars: CROSS= SYSROOT= PREFIX= DESTDIR= LINK_STATIC=1|0"

lib: $(LIBDIR)/liby2k38safe.a $(LIBDIR)/liby2k38safe.so

$(LIBDIR)/liby2k38safe.a: $(LIB_OBJS)
	$(AR) rcs $@ $^
	$(RANLIB) $@

$(LIBDIR)/liby2k38safe.so: $(LIB_SRCS)
	$(CC) $(CPPFLAGS) $(CFLAGS) -fPIC -shared -Wl,-soname,liby2k38safe.so.1 \
		-o $@ $(LIB_SRCS)

$(LIBDIR)/%.o: $(LIBDIR)/%.c
	$(CC) $(CPPFLAGS) $(CFLAGS) -fPIC -c -o $@ $<

demos: $(DEMO_BINS)

examples/broken/y2k38_broken: examples/broken/y2k38_broken.c
	$(CC) $(CPPFLAGS) $(CFLAGS) -o $@ $< $(LDFLAGS)

examples/broken/broken_daemon_pipeline: examples/broken/broken_daemon_pipeline.c
	$(CC) $(CPPFLAGS) $(CFLAGS) -o $@ $< $(LDFLAGS)

examples/fixed/y2k38_fixed: examples/fixed/y2k38_fixed.c $(LIBDIR)/liby2k38safe.a
	$(CC) $(CPPFLAGS) $(CFLAGS) -o $@ $< $(Y2K38_LIB) $(LDFLAGS) $(LIBS)

examples/fixed/kernel_offset_demo: examples/fixed/kernel_offset_demo.c $(LIBDIR)/liby2k38safe.a
	$(CC) $(CPPFLAGS) $(CFLAGS) -o $@ $< $(Y2K38_LIB) $(LDFLAGS) $(LIBS)

examples/fixed/y2k38_threads_demo: examples/fixed/y2k38_threads_demo.c $(LIBDIR)/liby2k38safe.a
	$(CC) $(CPPFLAGS) $(CFLAGS) -pthread -o $@ $< $(Y2K38_LIB) $(LDFLAGS) $(LIBS) -pthread

examples/fixed/y2k38_autowrap_demo: examples/fixed/y2k38_autowrap_demo.c $(LIBDIR)/liby2k38safe.a
	$(CC) $(CPPFLAGS) $(CFLAGS) -o $@ $< $(Y2K38_LIB) $(LDFLAGS) $(LIBS)

daemons: $(DAEMON_BINS)

daemons/daemon_a/daemon_a: daemons/daemon_a/daemon_a.c $(LIBDIR)/liby2k38safe.a
	$(CC) $(CPPFLAGS) $(CFLAGS) -o $@ $< $(Y2K38_LIB) $(LDFLAGS) $(LIBS)

daemons/daemon_b/daemon_b: daemons/daemon_b/daemon_b.c $(LIBDIR)/liby2k38safe.a
	$(CC) $(CPPFLAGS) $(CFLAGS) -o $@ $< $(Y2K38_LIB) $(LDFLAGS) $(LIBS)

tools: $(TOOL_BINS)

tools/y2k38_offsetctl: tools/y2k38_offsetctl.c $(LIBDIR)/liby2k38safe.a
	$(CC) $(CPPFLAGS) $(CFLAGS) -o $@ $< $(Y2K38_LIB) $(LDFLAGS) $(LIBS)

check: scripts/abi_check
	./scripts/abi_check

check-offset: examples/fixed/kernel_offset_demo tools/y2k38_offsetctl
	./examples/fixed/kernel_offset_demo
	./tools/y2k38_offsetctl calibrate 2147483748 --file /tmp/y2k38_off.conf \
		--mock-kernel -2147483548 --notify
	./tools/y2k38_offsetctl show --file /tmp/y2k38_off.conf \
		--mock-kernel -2147483548
	./tools/y2k38_offsetctl reload --file /tmp/y2k38_off.conf \
		--mock-kernel -2147483548
	./tools/y2k38_offsetctl set 4294967296 --file /tmp/y2k38_off2.conf --notify
	./tools/y2k38_offsetctl sync --file /tmp/y2k38_off2.conf
	@test "$$(./tools/y2k38_offsetctl get-time --file /tmp/y2k38_off.conf \
		--mock-kernel -2147483548)" = "2038-01-19:03:15:48"

scripts/abi_check: scripts/abi_check.c
	$(CC) $(CPPFLAGS) $(CFLAGS) -o $@ $< $(LDFLAGS)

# Cross build convenience (same as all with CROSS/SYSROOT already set)
cross: all
	@echo "Cross build done: CROSS=$(CROSS) SYSROOT=$(SYSROOT)"
	@file daemons/daemon_a/daemon_a 2>/dev/null || true

# Populate DESTDIR$(PREFIX) tree for tarball / scp deploy
stage: all
	$(MAKE) install DESTDIR=$(STAGE) PREFIX=/usr
	mkdir -p $(STAGE)/etc
	@if [ ! -f $(STAGE)/etc/y2k38_offset ]; then \
		echo "# placeholder — calibrate on board" > $(STAGE)/etc/y2k38_offset; \
		echo "OFFSET 0" >> $(STAGE)/etc/y2k38_offset; \
	fi
	mkdir -p $(STAGE)/usr/share/y2k38
	cp -a scripts/board-init-y2k38.sh $(STAGE)/usr/share/y2k38/ 2>/dev/null || true
	@echo "Staged under $(STAGE)"

install: lib
	mkdir -p $(DESTDIR)$(PREFIX)/include/y2k38
	mkdir -p $(DESTDIR)$(PREFIX)/include
	mkdir -p $(DESTDIR)$(PREFIX)/lib
	mkdir -p $(DESTDIR)$(PREFIX)/bin
	cp -a include/y2k38/*.h $(DESTDIR)$(PREFIX)/include/y2k38/
	cp -a include/y2k38.h $(DESTDIR)$(PREFIX)/include/
	cp -a $(LIBDIR)/liby2k38safe.a $(DESTDIR)$(PREFIX)/lib/
	cp -a $(LIBDIR)/liby2k38safe.so $(DESTDIR)$(PREFIX)/lib/liby2k38safe.so.1.0.0
	ln -sf liby2k38safe.so.1.0.0 $(DESTDIR)$(PREFIX)/lib/liby2k38safe.so.1
	ln -sf liby2k38safe.so.1 $(DESTDIR)$(PREFIX)/lib/liby2k38safe.so
	cp -a daemons/daemon_a/daemon_a $(DESTDIR)$(PREFIX)/bin/
	cp -a daemons/daemon_b/daemon_b $(DESTDIR)$(PREFIX)/bin/
	cp -a tools/y2k38_offsetctl $(DESTDIR)$(PREFIX)/bin/
	cp -a examples/fixed/y2k38_fixed $(DESTDIR)$(PREFIX)/bin/ 2>/dev/null || true
	cp -a examples/fixed/kernel_offset_demo $(DESTDIR)$(PREFIX)/bin/ 2>/dev/null || true

# Install ABI directly into ELDK sysroot (developer machine)
install-sysroot:
	@test -n "$(SYSROOT)" || (echo "SYSROOT required" >&2; exit 1)
	$(MAKE) all CROSS=$(CROSS) SYSROOT=$(SYSROOT)
	$(MAKE) install CROSS=$(CROSS) SYSROOT=$(SYSROOT) \
		DESTDIR= PREFIX=$(SYSROOT)/usr
	mkdir -p $(SYSROOT)/usr/lib/gcc-specs
	cp -a toolchain/abi/y2k38.specs $(SYSROOT)/usr/lib/gcc-specs/y2k38.specs
	@echo "Installed Y2K38 ABI into $(SYSROOT)/usr"

clean:
	rm -f $(LIB_OBJS) \
		$(LIBDIR)/liby2k38safe.a $(LIBDIR)/liby2k38safe.so \
		$(DEMO_BINS) $(DAEMON_BINS) $(TOOL_BINS) \
		scripts/abi_check \
		examples/broken/broken_events.log examples/fixed/fixed_events.log \
		demo_y2k38_offset.conf
	rm -rf $(STAGE)
