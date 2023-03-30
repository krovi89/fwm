CC := gcc

CFLAGS  += -std=c99 -pedantic -Wall -Wextra
LDFLAGS +=
LDLIBS   = -lxcb

WM_SRCDIR ?= src
WM_OBJDIR ?= obj
WM_OUTDIR ?= out

WM_OUT := fwm
WM_SRC := fwm.c      \
	  events.c   \
	  messages.c \
          log.c
WM_OBJ := $(patsubst %.c,obj/%.o,$(WM_SRC))

VPATH := $(WM_SRCDIR)

all: prepare $(WM_OUTDIR)/$(WM_OUT)

prepare:
	@echo [CFLAGS] $(CFLAGS)
	@echo [LDFLAGS] $(LDFLAGS)
	@echo [LDLIBS] $(LDLIBS)

	@mkdir -p $(WM_OBJDIR) $(WM_OUTDIR)

$(WM_OUTDIR)/$(WM_OUT): $(WM_OBJ)
	@echo [CC] $@: $^
	@$(CC) $(CFLAGS) $^ -o $@ $(LDFLAGS) $(LDLIBS)

$(WM_OBJDIR)/%.o: %.c
	@echo [CC] $@: $<
	@$(CC) $(CFLAGS) -c $< -o $@

$(WM_OBJDIR)/fwm.o: fwm.c fwm.h events.h messages.h log.h
$(WM_OBJDIR)/events.o: events.c events.h fwm.h
$(WM_OBJDIR)/messages.o: messages.c messages.h fwm.h log.h
$(WM_OBJDIR)/log.o: log.c log.h fwm.h
$(WM_OBJ): makefile

run:
	@cd $(WM_OUTDIR) && echo [RUN] $$(pwd): $(WM_OUT) && exec ./$(WM_OUT)

clean:
	@rm -f $(WM_OUTDIR)/$(WM_OUT) $(WM_OBJ)
	@echo [RM] $(WM_OUTDIR)/$(WM_OUT) $(WM_OBJ)

.PHONY: all prepare run clean
