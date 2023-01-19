/* tz imple:
 * 1. read scr register and determine the originated state (secure vs non-secure)
 * 2. do context switch between secure and non-secure state
 *    2-1. save current state (secure or non-secure) ctx into corresponding
 *         state's ctx memory.
 *    2-2. load the other state's ctx.
 *    2-3. set the NS bit in the scr with the other state (secure or non-secure).
 *    2-4. jump to secure smc handler if this is from non-secure smc call
 *    2-4. run ERET to exception return 
 *         https://developer.arm.com/documentation/ddi0406/c/System-Level-Architecture/System-Instructions/Alphabetical-list-of-instructions/ERET?lang=en
 * 3. 
 */
