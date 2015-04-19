PORT_SRC=memtester.c tests.c
PORT_TYPE=elf
PORT_TARGET=memtester.elf
PORT_INCLUDES=ports/posix/include
PORT_LIBS=build/libci20.a ports/posix/build/libposix.a

include ../baremetal.mk

