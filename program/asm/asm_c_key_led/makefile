# ***********************************************************************
# *        File:  makefile
# *     Version:  1.0.0
# *   Copyright:  2011 (c) Guo Wenxue <guowenxue@gmail.com>
# * Description:  Makefile used to cross compile the ASM and C source code
# *   ChangeLog:  1, Release initial version on "Tue Mar 22 21:58:24 CST 2011"
# *
# ***********************************************************************

CROSS = /opt/buildroot-2011.02/arm920t/usr/bin/arm-linux-
CFLAGS = 

key.bin: start.S buzzer_led.c
	${CROSS}gcc $(CFLAGS) -c -o start.o start.S
	${CROSS}gcc $(CFLAGS) -c -o buzzer_led.o buzzer_led.c
	${CROSS}ld -Tkey.lds start.o buzzer_led.o -o key.elf
	${CROSS}objcopy -O binary -S key.elf key.bin
	rm -f *.elf *.o

install:
	cp key.bin ~/winxp -f --reply=yes

clean:
	rm -f *.elf *.o
	rm -f key.bin
