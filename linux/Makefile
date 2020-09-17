CC = gcc
INC = -I ./scheduler -I ./data_structure/linked_list -I ./data_structure/priority_queue -I ./utils
CFLAGS = -Wall -fpic $(INC) -lm -O0
LIBS = -lpthread
LDFLAGS = -shared 
TARGET = libscheduler.so

SRCDIR = scheduler 			\
	data_structure/linked_list 	\
	data_structure/priority_queue	\
	utils

SOURCES = $(foreach dir, $(SRCDIR), $(wildcard $(dir)/*.c))
HEADERS = $(foreach dir, $(SRCDIR), $(wildcard $(dir)/*.h))
OBJECTS = $(SOURCES:.c=.o)

build: $(TARGET)

$(TARGET) : $(OBJECTS)
	$(CC)  $(LDFLAGS) $(LIBS) -o $@ $(OBJECTS)

.PHONY: clean
clean:
	rm -fr $(OBJECTS) $(TARGET)
