/* 
 *  MIT License
 *  
 *  Copyright (c) 2019 Sergey V. DUDANOV
 *  
 *  Permission is hereby granted, free of charge, to any person obtaining a copy
 *  of this software and associated documentation files (the "Software"), to deal
 *  in the Software without restriction, including without limitation the rights
 *  to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 *  copies of the Software, and to permit persons to whom the Software is
 *  furnished to do so, subject to the following conditions:
 *  
 *  The above copyright notice and this permission notice shall be included in all
 *  copies or substantial portions of the Software.
 *  
 *  THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 *  IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 *  FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 *  AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 *  LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 *  OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 *  SOFTWARE.
 */

#include <stdint.h>
#include <avr/io.h>
#include <avr/interrupt.h>
#include "nec.h"

#ifndef F_CPU
# warning "F_CPU is not defined. Using AVR default 1 MHz."
# define F_CPU 1000000
#endif

#define CPU_NS   (1000000000 / (F_CPU))
#define TIM_PRSC ((1476 + (CPU_NS)) / (CPU_NS))

#if TIM_PRSC > 64
# error "Your F_CPU is too high!"
#elif TIM_PRSC > 8
# undef  TIM_PRSC
# define TIM_PRSC 64
# define TIM_CTRL 3
#elif TIM_PRSC > 1
# undef  TIM_PRSC
# define TIM_PRSC 8
# define TIM_CTRL 2
#else
# undef  TIM_PRSC
# define TIM_PRSC 1
# define TIM_CTRL 1
#endif

#define NEC_T(t) ((562500 * (t) - 1) / ((CPU_NS) * (TIM_PRSC)))

/* status bits definitions */
#define NEC_REPEAT_EN   0x40
#define NEC_REPEAT_ST   0x80
#define NEC_REPEAT_MASK (NEC_REPEAT_ST | NEC_REPEAT_EN)

/* nec transmit data (5 bytes) */
static struct nec_ctx {
    uint32_t code;
    uint8_t status;
} ctx;

/* nec protocol data union */
union nec_code {
    uint32_t raw;           // [31:0]
    uint16_t ex_address;    // [15:0]
    struct {
        uint8_t address;    // [7:0]
        uint8_t _address;   // [15:8]
        uint8_t command;    // [23:16]
        uint8_t _command;   // [31:24]
    };
};

void
nec_init(void)
{
    /* setup OC1B(PB2) pin mode to output */
    DDRB |= _BV(DDB2);
    
    /* setup timer to FastPWM mode with TOP at OCR1A and non-inverted output at OC1B pin */
    TIMSK1 = _BV(OCIE1B);
    TCCR1A = _BV(COM1B1) | (3 << WGM10);
    TCCR1B = (3 << WGM12);

    /* write period and pulse init values to OCR1x buffer registers */
    OCR1A = NEC_T(24);
    OCR1B = NEC_T(16);
}

uint8_t
nec_send(uint8_t command, uint8_t address, uint8_t repeat)
{
    union nec_code code;

    code.address = address;
    code._address = ~address;

    return nec_send_ex(command, code.ex_address, repeat);
}

uint8_t
nec_send_ex(uint8_t command, uint16_t address, uint8_t repeat)
{
    union nec_code code;
    
    code.ex_address = address;
    code.command = command;
    code._command = ~command;

    return nec_send_raw(code.raw, repeat);
}

uint8_t
nec_send_raw(uint32_t code, uint8_t repeat)
{
    /* use TCCR1B as BUSY condition */
    const uint8_t tx_busy = TCCR1B & (TIM_CTRL << CS10);

    if (!tx_busy)
    {
        ctx.code = code;
        ctx.status = 33;
        
        if (repeat)
            ctx.status |= NEC_REPEAT_EN;

        /* start timer */
        TCCR1B = (3 << WGM12) | (TIM_CTRL << CS10);
    }

    return tx_busy;
}

void
nec_abort(void)
{
    ctx.status = 0;
}

/* TIM1 compare OCR1B interrupt handler */
ISR(TIMER1_COMPB_vect)
{
    /* FastPWM mode use double buffering feature of OCR1A and OCR1B registers */
    /* updating their values has been on old OCR1A compare event (TOP value) */
    
    /* Default: pulse = 1T, period = 2T */
    uint16_t pulse = NEC_T(1);
    uint16_t period = NEC_T(2);

    uint8_t repeat = ctx.status & NEC_REPEAT_MASK;
    uint8_t counter = ctx.status & ~NEC_REPEAT_MASK;

    /* main frame */
    if (counter)
    {
        /* 32-bit code: period = (0: 2T, 1: 4T) */
        if (--counter)
        {
            if (ctx.code & 1)
                period = NEC_T(4);
            
            ctx.code >>= 1;
        }
        
        /* tail */
        else
        {
            period = NEC_T(72);
        }
    }
    
    /* repeat frames */
    else if (repeat)
    {
        repeat ^= NEC_REPEAT_ST;

        if (repeat & NEC_REPEAT_ST)
        {
            period = NEC_T(20);
            goto pu16;
        }
        
        period = NEC_T(172);
    }
    
    /* stop */
    else
    {
        /* stop timer */
        TCCR1B = (3 << WGM12);
        
        /* prepare timer for the next transmit */
        period = NEC_T(24);
pu16:   pulse = NEC_T(16);
    }

    /* write new period and pulse values to OCR1x buffer registers */
    OCR1A = period;
    OCR1B = pulse;

    /* update status */
    ctx.status = counter | repeat;
}
