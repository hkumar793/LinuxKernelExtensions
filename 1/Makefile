obj-m += lkm1.o lkm2.o lkm3.o lkm4.o lkm5.o

all:
	make -C /lib/modules/$(shell uname -r)/build M=$(PWD) modules
	gcc -o test1 test1.c
	gcc -o test2 test2.c

clean:
	make -C /lib/modules/$(shell uname -r)/build M=$(PWD) clean
	rm -f test1 test2
