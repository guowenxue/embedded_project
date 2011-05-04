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

beep.bin: start.S beep.c
	arm-linux-gcc $(CFLAGS) -c -o start.o start.S
	arm-linux-gcc $(CFLAGS) -c -o beep.o beep.c
	arm-linux-ld -Tbeep.lds start.o beep.o -o beep.elf
	arm-linux-objcopy -O binary -S beep.elf beep.bin
	rm -f *.elf *.o

install:
	cp beep.bin ~/winxp -f --reply=yes

clean:
	rm -f *.elf *.o
	rm -f beep.bin
