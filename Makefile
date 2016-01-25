# Makefile Version 3.0
# depend on daiker install directory.
# ============================================================================
# Copyright (c) Moonlight Daiker Inc 2014
#
# Use of this software is controlled by the terms and conditions found in the
# license agreement under which this software has been supplied or provided.
# ============================================================================

DAIKER_PATH=../..

#include $(DAIKER_INSTALL_DIR)
#include $(DAIKER_PATH)/m/Rule.make


TARGET = $(notdir $(CURDIR))
# Comment this out if you want to see full compiler and linker output.
VERBOSE = @

#DAIKER_LIB=$(DAIKER_INSTALL_DIR)/l/libdaiker.a
INCLUDE_PATH=-I$(DAIKER_INSTALL_DIR)/h

CROSS_COMPILE=
CROSS_COMPILE=arm-hisiv100nptl-linux-
#CROSS_COMPILE=arm-none-linux-gnueabi-

CC=$(CROSS_COMPILE)gcc

#C_FLAGS += -Wall -g -O
C_FLAGS += -Wall -g -O2
LD_FLAGS += -lpthread 

COMPILE.c = $(VERBOSE) $(CC) $(C_FLAGS) -c
LINK.c = $(VERBOSE) $(CC) $(LD_FLAGS)

SOURCES = $(wildcard *.c) $(wildcard ../*.c)
HEADERS = $(wildcard *.h) $(wildcard ../*.h) 

OBJFILES = $(SOURCES:%.c=%.o)

.PHONY: clean install

all:	daiker

daiker:	daiker_al

daiker_al:	$(TARGET)

$(TARGET):	$(OBJFILES) $(DAIKER_LIB)
	@echo Linking $@ from $^..
	$(LINK.c) $(INCLUDE_PATH)  -o $@ $^

$(OBJFILES):	%.o: %.c $(HEADERS) $(XDC_CFLAGS)
	@echo Compiling $@ from $<..
	$(COMPILE.c) $(INCLUDE_PATH) -o $@ $<

$(XDC_LFILE) $(XDC_CFLAGS):	$(XDC_CFGFILE)
	@echo
	@echo ======== Building $(TARGET) ========
	@echo Configuring application using $<
	@echo
	$(VERBOSE) XDCPATH="$(XDC_PATH)" $(CONFIGURO) -c $(MVTOOL_DIR) -o $(XDC_CFG) -t $(XDC_TARGET) -p $(XDC_PLATFORM) $(XDC_CFGFILE)

clean:
	$(VERBOSE) -$(RM) -rf  $(OBJFILES) $(TARGET) *~ *.d .dep
