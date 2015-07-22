# Should be the name of the main .c file without the extension
CONTIKI_PROJECT = hello-world
all: $(CONTIKI_PROJECT)

# Add files that should be compiled with the Sancus compiler to SM_FILES
SM_FILES = sm.c

# Add extra project files that should not be compiled with the Sancus compiler
# to PROJECT_SOURCEFILES

# Should point to <GIT_ROOT>/targets
TARGETDIRS += ../../targets
# Should point to <GIT_ROOT>/contiki
CONTIKI = ../../contiki

CONTIKI_WITH_IPV4 = 1
include $(CONTIKI)/Makefile.include
