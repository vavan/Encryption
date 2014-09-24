
APP = key-client tunnel

CPP_FILES := \
    connection.cpp \
    config.cpp \
    proxy.cpp \
    socket.cpp \
    security.cpp


LOG4CPP:=TRUE
ifeq ($(CROSS), TRUE)
PREFIX:=arm-none-linux-gnueabi-
endif

#-Wextra 
CFLAGS := -Wall -O0 -ggdb -I./include -I./objs
LDFLAGS := -ggdb -lcrypto 
ifeq ($(LOG4CPP), TRUE) 
LDFLAGS += -llog4cpp
CFLAGS += -DLOG4CPP 
endif
OBJS := objs
BIN := bin
OBJ_FILES := $(addprefix $(OBJS)/,$(CPP_FILES:.cpp=.o))
DEPENDENCY = $(OBJS)/depend 
TARGET = $(addprefix $(BIN)/,$(APP))
CPP := $(PREFIX)g++
LD := $(PREFIX)g++
MAJ_VERSION := 0
MIN_VERSION := 3

all: $(TARGET)

debug: CFLAGS += -g -O0
debug: $(TARGET)

$(OBJS):
	mkdir $(OBJS)

$(BIN):
	mkdir $(BIN)

$(DEPENDENCY): $(CPP_FILES) | $(OBJS)
	rm -f $@
	$(CPP) $(CFLAGS) -MM $^ >> $@

# only include if goal is not clean mor install
ifneq ($(MAKECMDGOALS),clean)
ifneq ($(MAKECMDGOALS),install)
-include $(DEPENDENCY)
endif
endif

MAIN=$(addsuffix .o,$(addprefix $(OBJS)/,$(APP)))

$(OBJ_FILES) $(MAIN): objs/%.o : %.cpp | $(DEPENDENCY)
	$(CPP) $(CFLAGS) -DMIN_VERSION=$(MIN_VERSION) -DMAJ_VERSION=$(MAJ_VERSION) -c $< -o $@ 

$(TARGET): $(OBJ_FILES) $(MAIN) | $(BIN)
	@echo Build version: $(MAJ_VERSION).$(MIN_VERSION)
	$(LD) $(OBJ_FILES) $(OBJS)/$(notdir $@).o -o $@ $(LDFLAGS) 


clean:
	rm -rf $(BIN) $(OBJS) *.pem *.tgz core


install:
	cp $(TARGET) /usr/bin/

tar:
	tar -czf $(APP).$(MAJ_VERSION).$(MIN_VERSION).tgz $(CPP_FILES) *.h Makefile 

