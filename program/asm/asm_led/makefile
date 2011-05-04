
# ***********************************************************************
# *        File:  makefile
# *     Version:  1.0.0
# *   Copyright:  2011 (c) Guo Wenxue <guowenxue@gmail.com>
# * Description:  Makefile used to cross compile the ASM source code
# *   ChangeLog:  1, Release initial version on "Sun Mar 20 18:41:04 CST 2011"
# *
# ***********************************************************************

CROSS=arm-linux-

led.bin: led.S
	${CROSS}gcc led.S -c -o led.o
	${CROSS}ld -Ttext=0x33000000 led.o -o led.elf
	${CROSS}objcopy -O binary led.elf led.bin
	rm -f *.elf *.o

install:
	cp led.bin ~/winxp -f --reply=yes

clean:
	rm -f *.elf *.o
	rm -f led.bin
