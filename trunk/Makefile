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
LIBS =  -lpthread -lminigui -lm -lpng -ljpeg  -lmgext -lts
###################编译选项及编译器###################################
PLATFORM=arm-linux-
CC=$(PLATFORM)gcc
AS=$(PLATFORM)as

LDFLAGS+=$(LIBS)
DEFS =# -DPACKAGE_NAME=\"\" -DPACKAGE_TARNAME=\"\" -DPACKAGE_VERSION=\"\" -DPACKAGE_STRING=\"\" -DPACKAGE_BUGREPORT=\"\" -DPACKAGE=\"mde\" -DVERSION=\"1.6.10\" -DSTDC_HEADERS=1 -DHAVE_SYS_WAIT_H=1 -DTIME_WITH_SYS_TIME=1 -DHAVE_SYS_TYPES_H=1 -DHAVE_SYS_STAT_H=1 -DHAVE_STDLIB_H=1 -DHAVE_STRING_H=1 -DHAVE_MEMORY_H=1 -DHAVE_STRINGS_H=1 -DHAVE_INTTYPES_H=1 -DHAVE_STDINT_H=1 -DHAVE_UNISTD_H=1 -DHAVE_SYS_TIME_H=1 -DHAVE_UNISTD_H=1 -DHAVE_MINIGUI_COMMON_H=1 -DHAVE_DECL__MGRM_THREADS=1 -DHAVE_DECL__MGRM_PROCESSES=0 -DHAVE_DECL__MGRM_STANDALONE=0 -DHAVE_DECL__USE_NEWGAL=1 -DHAVE_DECL__HAVE_MATH_LIB=1 -DHAVE_DECL__PNG_FILE_SUPPORT=1 -DHAVE_DECL__JPG_FILE_SUPPORT=1 -DHAVE_DECL__TYPE1_SUPPORT=0 -DHAVE_DECL__TTF_SUPPORT=0 -DHAVE_DECL__FT2_SUPPORT=0 -D_STAND_ALONE
CFLAGS+= $(DEFS) -I/usr/local/arm/3.4.1/arm-linux/include -L/usr/local/arm/3.4.1/arm-linux/lib -Wall -W  -O3
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

$(OBJ_DIR)/asmpoly.o:$(SRC_DIR)/asmpoly.s
	$(CC) $< -o $@ -c  $(CFLAGS)

$(OBJ_DIR)/asmmisc.o:$(SRC_DIR)/asmmisc.s
	$(CC) $<  -o $@ -c  $(CFLAGS)

rebuild: clean all

clean:
	rm -rf $(OBJS) $(LIB_DIR)/lib* $(BIN_DIR)/* 

