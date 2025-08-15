TARGETS = control_station soldier

SOURCES = $(TARGETS:=.c)

obj-m += chardev.o

CC = gcc
CFLAGS = -Wall

all: $(TARGETS) modules

$(TARGETS): $(SOURCES)
	$(CC) $(CFLAGS) -o $@ $@.c

modules:
	make -C /lib/modules/$(shell uname -r)/build M=$(PWD) modules

clean:
	rm -f $(TARGETS)
	make -C /lib/modules/$(shell uname -r)/build M=$(PWD) clean

