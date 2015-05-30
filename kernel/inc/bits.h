#include <stdint-gcc.h>
/*
 * Bit manipulation macros
 */
#define BIT(x) (1<<(x))
#define BM(msb, lsb)        (((((uint32_t)-1) << (31-msb)) >> (31-msb+lsb)) << lsb)
#define BVAL(msb, lsb, val) (((val) << lsb) & BM(msb, lsb))

