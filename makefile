CFLAGS += -std=c99 -pedantic -Wall -Wextra \
          -D _POSIX_C_SOURCE
LDLIBS := -lxcb

FWM_SRCDIR ?= src
FWM_OBJDIR ?= obj
FWM_OUTDIR ?= out

FWM_OUT := fwm
FWM_SRC := fwm.c           \
           events.c        \
           actions.c       \
           keybinds.c      \
           messages.c      \
	   files.c         \
           log.c           \
           execute.c       \
           close_focused.c
FWM_OBJ := $(patsubst %.c,$(FWM_OBJDIR)/%.o,$(FWM_SRC))

VPATH := $(FWM_SRCDIR)
VPATH += $(FWM_SRCDIR)/actions

all: prepare $(FWM_OUTDIR)/$(FWM_OUT)

prepare:
	@echo [CFLAGS] $(CFLAGS)
	@echo [LDLIBS] $(LDLIBS)

	@mkdir -p $(FWM_OBJDIR) $(FWM_OUTDIR)

$(FWM_OUTDIR)/$(FWM_OUT): $(FWM_OBJ)
	@echo [CC] $@: $^
	@$(CC) $(CFLAGS) $^ -o $@ $(LDLIBS)

$(FWM_OBJDIR)/%.o: %.c
	@echo [CC] $@: $<
	@$(CC) $(CFLAGS) -c $< -o $@

$(FWM_OBJDIR)/fwm.o: fwm.c fwm.h events.h messages.h log.h
$(FWM_OBJDIR)/events.o: events.c events.h fwm.h actions.h keybinds.h
$(FWM_OBJDIR)/actions.o: actions.c actions.h fwm.h
$(FWM_OBJDIR)/keybinds.o: keybinds.c keybinds.h fwm.h
$(FWM_OBJDIR)/messages.o: messages.c messages.h fwm.h actions.h keybinds.h log.h
$(FWM_OBJDIR)/files.o: files.c files.h fwm.h
$(FWM_OBJDIR)/log.o: log.c log.h fwm.h
$(FWM_OBJ): makefile

run:
	@cd $(FWM_OUTDIR) && echo [RUN] $$(pwd): $(FWM_OUT) && exec ./$(FWM_OUT)

clean:
	@rm -f $(FWM_OUTDIR)/$(FWM_OUT) $(FWM_OBJ)
	@echo [RM] $(FWM_OUTDIR)/$(FWM_OUT) $(FWM_OBJ)

.PHONY: all prepare run clean
