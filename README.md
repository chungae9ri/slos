# slos
SLOS represents Simple Light OS. 
This repository is for the source code of SLOS running in ARM processor. 
It is tested in Cortex-A9 in Xilinx Zynq7000 chipset but seems to be expanded to other products.

Implementation list 
1. Process Management
   - TCB (Task Control Block)
   - task fork - forkyi()
   - GIC - interrupt handler
   - task synchronization(spin lock)
2. Timer frame work
   - realtime timer, sched timer, oneshot timer
   - timer interrupt handler
3. Realtime scheduler for rt task
   - Earliest Deadline First scheduler
   - preemptive context switch
4. CFS scheduler for task others
   - run q and wait q
   - imitate Linux run q and CFS scheduler 
   - sched entity with virtual runtime
   - shell task, worker task, cpu idle task, and dummy task
5. Memory Management 
   - page frame pool
   - small page table walk
   - virtual memory manager
   - page fault handler
   - demand paging
6. slfs(Simple Light File System) 
   - based on ramdisk
   - 2 level file system : file size up to 800KB
   - ram disk and elf loader for use applications
7. syscalls through the svc instruction - libslos.a
8. Hardware-Software codesign
   - Two FPGA (modcore, odev) implementations for a custom peripheral HW in PL subsystem 
   - Script base Vivado project implementation
   - a device driver with DMA interrupt handler
   - a device driver for out-stream device

Refer A little book on custom OS developement from scratch.pdf for more information.
