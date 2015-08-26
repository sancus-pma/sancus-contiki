# Should be the name of the main .c file without the extension
CONTIKI_PROJECT = sm-server
all: $(CONTIKI_PROJECT)

APPS = sm-loader reactive
SM_LOADER_PORT = 2000
REACTIVE_PORT = 2001

# Should point to <GIT_ROOT>/targets
TARGETDIRS += ../../targets
# Should point to <GIT_ROOT>/apps
APPDIRS += ../../apps
# Should point to <GIT_ROOT>/contiki
CONTIKI = ../../contiki

CONTIKI_WITH_IPV4 = 1
include $(CONTIKI)/Makefile.include
