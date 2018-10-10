# slos
SLOS represents Simple Light OS. 
This repository is for the source code of SLOS running in ARM processor. 
It is tested in Qualcomm msm8626 but seems to be expanded to other products.

Implementation list 

1. Memory Management 
   - page frame pool
   - page table walk
   - virtual memory manager
   - page fault handler
   - lazy memory allocation
2. Timer frame work
   - realtime timer, sched timer, oneshot timer
   - timer interrupt handler
3. Realtime scheduler for rt task
   - Earliest Deadline First scheduler
   - preemptive context switch
4. CFS scheduler for task others
   - run q and wait q
   - task fork - forkyi()
   - imitate Linux run q and CFS scheduler 
   - sched entity
   - shell task, worker task, cpu idle task, and dummy task
5. slfs(Simple Light File System) 
   - based on ramdisk
   - 2 level file system : file size up to 800KB
6. syscalls - exit, print_msg
7. ram disk and elf loader 
8. task synchronization(spin lock)

Refer Doc/SLOS.pdf for more information.
