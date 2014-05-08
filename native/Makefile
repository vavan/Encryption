


APP = ssl-key-client

CPP_FILES := \
    main.cpp \
    connection.cpp \
    config.cpp \
    proxy.cpp \
    socket.cpp
     

#-Wextra 
CFLAGS := -Wall -ggdb -I./include -I./objs
LDFLAGS := -ggdb -lcrypto -llog4cpp
OBJS := objs
BIN := bin
OBJ_FILES := $(addprefix $(OBJS)/,$(CPP_FILES:.cpp=.o))
MIN_VERSION = $(OBJS)/version.out
DEPENDENCY = $(OBJS)/depend 
TARGET = $(BIN)/$(APP)
CPP := g++
LD := g++
MAJ_VERSION := 0
MIN_VERSION := 1

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

$(OBJ_FILES): objs/%.o : %.cpp | $(DEPENDENCY)
	$(CPP) $(CFLAGS) -DMIN_VERSION=$(MIN_VERSION) -DMAJ_VERSION=$(MAJ_VERSION) -c $< -o $@ 

$(TARGET): $(OBJ_FILES) | $(BIN)
	@echo Build version: $(MAJ_VERSION).$(MIN_VERSION)
	$(LD) $(OBJ_FILES) -o $@ $(LDFLAGS) 


clean:
	rm -rf $(BIN) $(OBJS)


install:
	cp $(TARGET) /usr/bin/



