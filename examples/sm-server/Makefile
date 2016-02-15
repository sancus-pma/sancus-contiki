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

ifndef NO_SANCUS
SM_FILES = buttons.c buttons_driver.c port1_mmio.S port1_mmio_decl.c \
           lcd.c lcd_driver.c lcd_mmio.S lcd_mmio_decl.c
else
PROJECT_SOURCEFILES += buttons_no_sancus.c lcd_no_sancus.c
endif

CONTIKI_WITH_IPV4 = 1
include $(CONTIKI)/Makefile.include
