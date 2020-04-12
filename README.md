## nec

Unmodulated NEC protocol library for [DMBS](https://github.com/abcminiuser/dmbs). Use TIM1 16-bit timer.

Output: OC1B pin (positive active).

```c
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
```
