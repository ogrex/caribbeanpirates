######################################################################
#	makefile 模版 (by lichuang)
#
######################################################################

###################项目路径和程序名称#################################
DIR=$(shell pwd)
BIN_DIR=$(DIR)/bin
LIB_DIR=$(DIR)/lib
SRC_DIR=$(DIR)/src
INCLUDE_DIR=/usr/local/arm/3.4.1/lib/
OBJ_DIR=$(DIR)/obj
DEPS_DIR=$(DIR)/deps
PROGRAM=$(BIN_DIR)/test

###################OBJ文件及路径############################################
EXTENSION=c
OBJS=$(patsubst $(SRC_DIR)/%.$(EXTENSION), $(OBJ_DIR)/%.o,$(wildcard $(SRC_DIR)/*.$(EXTENSION)))
DEPS=$(patsubst $(OBJ_DIR)/%.o, $(DEPS_DIR)/%.d, $(OBJS))
OBJS+=$(OBJ_DIR)/asmmisc.o
OBJS+=$(OBJ_DIR)/asmpoly.o
###################include头文件路径##################################
INCLUDE=\
		-I$(INCLUDE_DIR)
		
###################lib文件及路径######################################

###################编译选项及编译器###################################
PLATFORM=arm-linux-
CC=$(PLATFORM)gcc
AS=$(PLATFORM)as
CFLAGS=-Wall -W  -O3
LDFLAGS=

###################编译目标###########################################
.PHONY: all clean rebuild

all:$(OBJS) 
	@echo $(DEPS_DIR)
	$(CC) -o $(PROGRAM) $(OBJS) $(LDFLAGS) 


$(DEPS_DIR)/%.d: $(SRC_DIR)/%.c
	$(CC) -MM $(INCLUDE) $(CFLAGS) $< > $@

sinclude $(DEPS)

$(OBJ_DIR)/%.o:$(SRC_DIR)/%.c
	$(CC) $< -o $@ -c $(CFLAGS) $(INCLUDE) 




rebuild: clean all

clean:
	rm -rf $(OBJS) $(LIB_DIR)/lib* $(BIN_DIR)/* 

