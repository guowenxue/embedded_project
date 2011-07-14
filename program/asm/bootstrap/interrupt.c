/********************************************************************************************
 *        File:  interrupt.c
 *     Version:  1.0.0
 *   Copyright:  2011 (c) Guo Wenxue <guowenxue@gmail.com>
 * Description:  The CPU interrupt vector table depends on function, called in start.S 
 *   ChangeLog:  1, Release initial version on "Tue Jul 12 16:43:18 CST 2011"
 *
 *******************************************************************************************/

#include <common.h>

void do_undefined_instruction(void)
{

}

void do_software_interrupt(void)
{
}

void do_prefetch_abort(void)
{
}

void do_data_abort(void)
{
}

void do_not_used(void)
{
}

void do_irq(void)
{
}

void do_fiq(void)
{
}
