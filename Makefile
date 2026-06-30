#######################################
#    Makefile for SmileNetwork		  #
#######################################

#   The general compiling commands    #

CP = cp
CC = gcc
SUDO = sudo
CCFLAGES = -c -g -O0

TARGET = SmileNetwork

TGT = tgt/*

OBJS = tgt/main.o tgt/memory.o tgt/command.o tgt/logger.o tgt/executor.o tgt/stringtype.o tgt/arraylist.o tgt/hashmap.o tgt/stack.o tgt/json.o tgt/bytereader.o tgt/exception.o tgt/assertion.o tgt/generator.o tgt/printer.o tgt/file.o tgt/model.o tgt/config.o tgt/network.o tgt/activator.o tgt/optimizer.o tgt/linear.o tgt/conv.o tgt/loss.o tgt/matrix.o tgt/tensor.o tgt/bias.o tgt/vector.o tgt/mnist.o tgt/train.o

.PHONY : build clean

build : clean $(TARGET)

clean :
	 rm -rf $(TARGET) $(TGT)

tgt/memory.o : src/memory/memory.c src/memory/memory.h
	$(CC) $(CCFLAGES) $< -o $@

tgt/stringtype.o : src/datatype/stringtype.c src/datatype/stringtype.h
	$(CC) $(CCFLAGES) $< -o $@

tgt/arraylist.o : src/datatype/arraylist.c src/datatype/arraylist.h
	$(CC) $(CCFLAGES) $< -o $@

tgt/hashmap.o : src/datatype/hashmap.c src/datatype/hashmap.h
	$(CC) $(CCFLAGES) $< -o $@

tgt/stack.o : src/datatype/stack.c src/datatype/stack.h
	$(CC) $(CCFLAGES) $< -o $@

tgt/json.o : src/json/json.c src/json/json.h
	$(CC) $(CCFLAGES) $< -o $@

tgt/bytereader.o : src/reader/bytereader.c src/reader/bytereader.h
	$(CC) $(CCFLAGES) $< -o $@

tgt/exception.o : src/except/exception.c src/except/exception.h
	$(CC) $(CCFLAGES) $< -o $@

tgt/assertion.o : src/except/assertion.c src/except/assertion.h
	$(CC) $(CCFLAGES) $< -o $@

tgt/generator.o : src/generator/generator.c src/generator/generator.h
	$(CC) $(CCFLAGES) $< -o $@

tgt/printer.o : src/printer/printer.c src/printer/printer.h
	$(CC) $(CCFLAGES) $< -o $@

tgt/file.o : src/file/file.c src/file/file.h
	$(CC) $(CCFLAGES) $< -o $@

tgt/model.o : src/model/model.c src/model/model.h
	$(CC) $(CCFLAGES) $< -o $@

tgt/config.o : src/network/config.c src/network/config.h
	$(CC) $(CCFLAGES) $< -o $@

tgt/network.o : src/network/network.c src/network/network.h
	$(CC) $(CCFLAGES) $< -o $@

tgt/activator.o : src/network/activator.c src/network/activator.h
	$(CC) $(CCFLAGES) $< -o $@

tgt/optimizer.o : src/network/optimizer.c src/network/optimizer.h
	$(CC) $(CCFLAGES) $< -o $@

tgt/linear.o : src/network/linear.c src/network/linear.h
	$(CC) $(CCFLAGES) $< -o $@

tgt/conv.o : src/network/conv.c src/network/conv.h
	$(CC) $(CCFLAGES) $< -o $@

tgt/loss.o : src/network/loss.c src/network/loss.h
	$(CC) $(CCFLAGES) $< -o $@

tgt/bias.o : src/network/bias.c src/network/bias.h
	$(CC) $(CCFLAGES) $< -o $@

tgt/tensor.o : src/network/tensor.c src/network/tensor.h
	$(CC) $(CCFLAGES) $< -o $@

tgt/matrix.o : src/network/matrix.c src/network/matrix.h
	$(CC) $(CCFLAGES) $< -o $@

tgt/vector.o : src/network/vector.c src/network/vector.h
	$(CC) $(CCFLAGES) $< -o $@

tgt/mnist.o : src/dataset/mnist.c src/dataset/mnist.h
	$(CC) $(CCFLAGES) $< -o $@

tgt/train.o : src/dataset/train.c src/dataset/train.h
	$(CC) $(CCFLAGES) $< -o $@

tgt/logger.o : src/logger/logger.c src/logger/logger.h
	$(CC) $(CCFLAGES) $< -o $@

tgt/command.o : src/command/command.c src/command/command.h
	$(CC) $(CCFLAGES) $< -o $@

tgt/executor.o : src/command/executor.c src/command/executor.h
	$(CC) $(CCFLAGES) $< -o $@

tgt/main.o : src/main.c
	$(CC) $(CCFLAGES) $< -o $@

SmileNetwork : $(OBJS)
	$(CC) $(OBJS) -g -O0 -o $@ -lz -lm