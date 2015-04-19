/*
 * memtester version 4
 *
 * Very simple but very effective user-space memory tester.
 * Originally by Simon Kirby <sim@stormix.com> <sim@neato.org>
 * Version 2 by Charles Cazabon <charlesc-memtester@pyropus.ca>
 * Version 3 not publicly released.
 * Version 4 rewrite:
 * Copyright (C) 2004-2012 Charles Cazabon <charlesc-memtester@pyropus.ca>
 * Licensed under the terms of the GNU General Public License version 2 (only).
 * See the file COPYING for details.
 *
 */

#define __version__ "4.3.0"

#include <sys/types.h>
#include <stddef.h>
#include <libci20/uart.h>

#include "types.h"
#include "sizes.h"
#include "tests.h"

#define EXIT_FAIL_NONSTARTER    0x01
#define EXIT_FAIL_ADDRESSLINES  0x02
#define EXIT_FAIL_OTHERTEST     0x04

struct test tests[] = {
    { "Random Value", test_random_value },
    { "Compare XOR", test_xor_comparison },
    { "Compare SUB", test_sub_comparison },
    { "Compare MUL", test_mul_comparison },
    { "Compare DIV",test_div_comparison },
    { "Compare OR", test_or_comparison },
    { "Compare AND", test_and_comparison },
    { "Sequential Increment", test_seqinc_comparison },
    { "Solid Bits", test_solidbits_comparison },
    { "Block Sequential", test_blockseq_comparison },
    { "Checkerboard", test_checkerboard_comparison },
    { "Bit Spread", test_bitspread_comparison },
    { "Bit Flip", test_bitflip_comparison },
    { "Walking Ones", test_walkbits1_comparison },
    { "Walking Zeroes", test_walkbits0_comparison },
#ifdef TEST_NARROW_WRITES    
    { "8-bit Writes", test_8bit_wide_random },
    { "16-bit Writes", test_16bit_wide_random },
#endif
    { NULL, NULL }
};

int memtester_pagesize(void) {
    return 4096;
}

/* Global vars - so tests have access to this information */
int use_phys = 0;
off_t physaddrbase = 0;

int main(int argc, char **argv) {
    ul loops, loop, i;
    size_t wantraw, wantbytes, bufsize,
         halflen, count;
    void volatile *buf, *aligned;
    ulv *bufa, *bufb;
    int exit_code = 0;
    int memshift;
    ul testmask = 0;

    uart_print("memtester version " __version__ " (%x4-bit)\r\n", UL_LEN);
    uart_print("Copyright (C) 2001-2012 Charles Cazabon.\r\n");
    uart_print("Licensed under the GNU General Public License version 2 (only).\r\n");
    uart_print("\r\n");

    physaddrbase = 0x80009000;
    use_phys = 1;
    wantraw = 200; 
    memshift = 20; /* megabytes */
    wantbytes = wantraw << memshift;
    loops = 0; /* Run forever */

    buf = (void volatile *)physaddrbase;

    bufsize = wantbytes; /* accept no less */
    aligned = buf;

    halflen = bufsize / 2;
    count = halflen / sizeof(ul);
    bufa = (ulv *) aligned;
    bufb = (ulv *) ((size_t) aligned + halflen);

    for(loop=1; ((!loops) || loop <= loops); loop++) {
        uart_print("Loop %x4", loop);
        if (loops) {
            uart_print("/%x4", loops);
        }
        uart_print(":\r\n");
        uart_print("  %s: ", "Stuck Address");
        if (!test_stuck_address(aligned, bufsize / sizeof(ul))) {
             uart_print("ok\r\n");
        } else {
            exit_code |= EXIT_FAIL_ADDRESSLINES;
        }
        for (i=0;;i++) {
            if (!tests[i].name) break;
            /* If using a custom testmask, only run this test if the
               bit corresponding to this test was set by the user.
             */
            if (testmask && (!((1 << i) & testmask))) {
                continue;
            }
            uart_print("  %s: ", tests[i].name);
            if (!tests[i].fp(bufa, bufb, count)) {
                uart_print("ok\r\n");
            } else {
                exit_code |= EXIT_FAIL_OTHERTEST;
            }
        }
        uart_print("\r\n");
    }
    uart_print("Done.\r\n");

    return exit_code;
}

