#######################################
#    Makefile for SmileNetwork		  #
#######################################

#   The general compiling commands    #

CP = cp
CC = gcc
SUDO = sudo
CCFLAGES = -c

TARGET = SmileNetwork

TGT = tgt/*

OBJS = tgt/main.o tgt/command.o tgt/executor.o tgt/stringtype.o tgt/hashmap.o tgt/mnist.o tgt/model.o tgt/activator.o tgt/label.o tgt/layer.o tgt/loss.o tgt/matrix.o tgt/result.o tgt/vector.o tgt/traindata.o

.PHONY : build clean

build : clean $(TARGET)

clean :
	 rm -rf $(TARGET) $(TGT)

tgt/stringtype.o : src/datatype/stringtype.c src/datatype/stringtype.h
	$(CC) $(CCFLAGES) $< -o $@

tgt/hashmap.o : src/datatype/hashmap.c src/datatype/hashmap.h
	$(CC) $(CCFLAGES) $< -o $@

tgt/mnist.o : src/dataset/mnist.c src/dataset/mnist.h
	$(CC) $(CCFLAGES) $< -o $@

tgt/model.o : src/model/model.c src/model/model.h
	$(CC) $(CCFLAGES) $< -o $@

tgt/activator.o : src/network/activator.c src/network/activator.h
	$(CC) $(CCFLAGES) $< -o $@

tgt/label.o : src/network/label.c src/network/label.h
	$(CC) $(CCFLAGES) $< -o $@

tgt/layer.o : src/network/layer.c src/network/layer.h
	$(CC) $(CCFLAGES) $< -o $@

tgt/loss.o : src/network/loss.c src/network/loss.h
	$(CC) $(CCFLAGES) $< -o $@

tgt/matrix.o : src/network/matrix.c src/network/matrix.h
	$(CC) $(CCFLAGES) $< -o $@

tgt/result.o : src/network/result.c src/network/result.h
	$(CC) $(CCFLAGES) $< -o $@

tgt/vector.o : src/network/vector.c src/network/vector.h
	$(CC) $(CCFLAGES) $< -o $@

tgt/traindata.o : src/traindata/traindata.c src/traindata/traindata.h
	$(CC) $(CCFLAGES) $< -o $@

tgt/command.o : src/command/command.c src/command/command.h
	$(CC) $(CCFLAGES) $< -o $@

tgt/executor.o : src/command/executor.c src/command/executor.h
	$(CC) $(CCFLAGES) $< -o $@

tgt/main.o : src/main.c
	$(CC) $(CCFLAGES) $< -o $@

SmileNetwork : $(OBJS)
	$(CC) $(OBJS) -o $@ -lz -lm