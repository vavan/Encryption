
APP = tunnel

CPP_FILES := \
    config.cpp \
    proxy.cpp \
    socket.cpp \
    queue.cpp \
    worker.cpp \
    normal_socket.cpp \
    secure_socket.cpp \
    secure_impl.cpp \
    secure_state.cpp


ifeq ($(CROSS), TRUE)
PREFIX:=arm-none-linux-gnueabi-
endif

#-Wextra 
CFLAGS := -Wall -O3 -ggdb -DLOG4CPP -I./include -I./objs
LDFLAGS := -ggdb -lcrypto -lssl -llog4cpp 
OBJS := objs
BIN := bin
OBJ_FILES := $(addprefix $(OBJS)/,$(CPP_FILES:.cpp=.o))
DEPENDENCY = $(OBJS)/depend 
TARGET = $(addprefix $(BIN)/,$(APP))
CPP := $(PREFIX)g++
LD := $(PREFIX)g++
MAJ_VERSION := 2
MIN_VERSION := 8

ifeq ($(PROF), TRUE)
CFLAGS += -pg
LDFLAGS += -pg
endif


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
	rm -rf $(BIN) $(OBJS) *.tgz core


install:
	cp $(TARGET) /usr/bin/

tar:
	tar -czf $(APP).$(MAJ_VERSION).$(MIN_VERSION).tgz $(CPP_FILES) *.h Makefile 

test: all
	cd ../test/pipe && python -m unittest -v test_proxy


