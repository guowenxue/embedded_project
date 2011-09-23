
# ***********************************************************************
# *        File:  makefile
# *     Version:  1.0.0
# *   Copyright:  2011 (c) Guo Wenxue <guowenxue@gmail.com>
# * Description:  Makefile used to cross compile the ASM source code
# *   ChangeLog:  1, Release initial version on "Sun Mar 20 18:41:04 CST 2011"
# *
# ***********************************************************************

CROSS=/opt/buildroot-2011.02/arm920t/usr/bin/arm-linux-

init.bin: init.S
	${CROSS}gcc init.S -c -o init.o
	${CROSS}ld -Ttext=0x33000000 init.o -o init.elf
	${CROSS}objcopy -O binary init.elf init.bin
	rm -f *.elf *.o

install:
	cp init.bin ~/winxp -f --reply=yes

clean:
	rm -f *.elf *.o
	rm -f init.bin
