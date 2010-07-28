# lots of shit here stolen from elsewhere, most notably the linux kernel's
# makefiles

CFLAGS += -Wall -Werror -Wextra -D_FILE_OFFSET_BITS=64 -D_BSD_SOURCE -D_XOPEN_SOURCE=600 -std=c99 -g3
LDFLAGS += -g3

BINS=baleet
baleet_SRCS=baleet.c
baleet_OBJS=$(addprefix .objs/,$(baleet_SRCS:.c=.o))
baleet_DEPS=$(addprefix .deps/,$(baleet_SRCS:.c=.d))

cmd_cc_o_c=$(CC) $(CFLAGS) -o $@ -c $<
quiet_cmd_cc_o_c = CC       $*.o
cmd_md_d_c=$(CC) $(CFLAGS) -o $@ -MT .objs/$(patsubst %.c,%.o,$<) -M $<
quiet_cmd_md_d_c = MD       $*.d
cmd_ld_bin_o=$(CC) $(LDFLAGS) -o $@ $^
quiet_cmd_ld_bin_o = LD       $@
cmd_TAGS=etags $^
quiet_cmd_TAGS = BUILD    TAGS
cmd_tags=ctags $^
quiet_cmd_tags = BUILD    tags
cmd_cscope=cscope -b $^
quiet_cmd_cscope = BUILD    cscope.out

ifdef VERBOSE
	quiet=
	Q=
else
	quiet=quiet_
	Q=@
endif

.PHONY: all check clean allclean

all: $(BINS) TAGS tags cscope.out

baleet: $(baleet_OBJS)
	@echo "   $($(quiet)cmd_ld_bin_o)"; $(cmd_ld_bin_o)

check: $(BINS)
	$(Q)./test.sh && echo "pass!"

.objs/%.o: .deps/%.d
.deps/%.d: %.c
	$(Q)[ -d .deps ] || mkdir -p .deps
	@echo "   $($(quiet)cmd_md_d_c)"; $(cmd_md_d_c)

.objs/%.o: %.c
	$(Q)[ -d .objs ] || mkdir -p .objs
	@echo "   $($(quiet)cmd_cc_o_c)"; $(cmd_cc_o_c)

clean:
	$(Q)rm -f $(BINS) $(baleet_OBJS)

allclean: clean
	$(Q)rm -f .*.sw? "#"*"#" *\~ tags TAGS cscope.out $(baleet_DEPS)

TAGS tags cscope: $(wildcard *.c) $(wildcard *.h)
TAGS:
	@echo "   $($(quiet)cmd_TAGS)"; $(cmd_TAGS)
tags:
	@echo "   $($(quiet)cmd_tags)"; $(cmd_tags)
cscope.out:
	@echo "   $($(quiet)cmd_cscope)"; $(cmd_cscope)

-include $(baleet_DEPS)