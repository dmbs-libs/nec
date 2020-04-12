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

#ifndef _NEC_H_
#define _NEC_H_

#include <stdint.h>

/*
 *  NEC init function
 */
void
nec_init(void);

/*  
 *  Send NEC protocol command with repeat frames (if repeat != 0)
 *  Return: zero if success, else - tx is busy
 */
uint8_t
nec_send(uint8_t command, uint8_t address, uint8_t repeat);

/*  
 *  Send extended NEC protocol command with repeat frames (if repeat != 0)
 *  Return: zero if success, else - tx is busy
 */
uint8_t
nec_send_ex(uint8_t command, uint16_t address, uint8_t repeat);

/*  
 *  Send raw NEC 32-bit code with repeat frames (if repeat != 0)
 *  Return: zero if success, else - tx is busy
 */
uint8_t
nec_send_raw(uint32_t code, uint8_t repeat);

/*
 *  Abort transmit
 */
void
nec_abort(void);

#endif
