# ***********************************************************************
# *        File:  makefile
# *     Version:  1.0.0
# *   Copyright:  2011 (c) Guo Wenxue <guowenxue@gmail.com>
# * Description:  Makefile used to cross compile the ASM and C source code
# *   ChangeLog:  1, Release initial version on "Mon Mar 21 21:09:52 CST 2011"
# *
# ***********************************************************************

CROSS = arm-linux-
CFLAGS = 

led.bin: start.S led.c
	arm-linux-gcc $(CFLAGS) -c -o start.o start.S
	arm-linux-gcc $(CFLAGS) -c -o led.o led.c
	arm-linux-ld -Tled.lds start.o led.o -o led.elf
	arm-linux-objcopy -O binary -S led.elf led.bin
	rm -f *.elf *.o

install:
	cp led.bin ~/winxp -f --reply=yes

clean:
	rm -f *.elf *.o
	rm -f led.bin
