program      := trial
sources      := trial.c
objects      := $(subst .c,.o,$(sources))
libraries    := -lev \
                -lminIni \
                -lpthread \
                -lcivetweb \
                -ldl \
                -lm
dependencies := $(subst .c,.d,$(sources))

include_dirs := ./include \
                ../../include
CPPFLAGS     += $(addprefix -I , $(include_dirs))
vpath %.h $(include_dirs)

lib_dirs     := ../../lib/readerConfiguration \
                ../../lib/readerHealthMonitor \
                ../../lib/readerInitialization \
                ../../lib/readerSerialComm \
                ../../lib/readerTCPComm \
                ../../lib/readerStatesAndModes \
                ../../lib/readerDataProcessor \
                ../../lib/db ../../lib/ui
vpath %.a $(lib_dirs)

all: $(program)

$(program): $(objects) $(libraries)

.PHONY: clean
clean:
	$(RM) $(program) $(objects) $(dependencies)

ifneq "$(MAKECMDGOALS)" "clean"
  include $(dependencies)
endif

%.d: %.c
	$(CC) $(CFLAGS) $(CPPFLAGS) $(TARGET_ARCH) -M $< |	\
	sed 's,\($*\.o\) *:,\1 $@: ,' > $@.tmp
	mv $@.tmp $@
