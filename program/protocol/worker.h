#ifndef __WORKER_H
#define __WORKER_H

#include <stdio.h>
#include <unistd.h>
#include "protocol.h"

uchar test_usb(uchar *pacParm, int iParmLen);
uchar test_led(uchar *pacParm, int iParmLen);
TEST_BODY * find_body(PROT_WORK *test_list, uchar ucCmd);

#endif
