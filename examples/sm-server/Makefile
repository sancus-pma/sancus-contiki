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

PROJECT_SOURCEFILES = buttons_process.c
SM_FILES = buttons.c buttons_driver.c port1_mmio.S port1_mmio_decl.c

CONTIKI_WITH_IPV4 = 1
include $(CONTIKI)/Makefile.include
