# SLOS
SLOS represents Simple Light OS. 
This repository is for the source code of SLOS running in ARM processor. 
It is tested in Cortex-A9 in Xilinx Zynq7000 chipset (currently tested in coraz7 board)

Implementation list 
1. Process Management
   - TCB (Task Control Block)
   - task fork - forkyi()
   - GIC - interrupt handler (top half / bottom half)
   - task synchronization(spin lock)
   - task state - TASK_RUNNING, TASK_WAITING
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
6. File Systems
   - Custom SLFS (Simple Light File System) 
      - based on ramdisk
      - 2 level file system : file size up to 800KB
      - ramdisk and elf loader for user applications
   - Opensource littleFS ported
      - ramdisk and elf loader is working with littleFS
8. syscalls through the svc instruction - libslos.a
9. Hardware-Software codesign
   - Two FPGA (modcore, odev) implementations for a custom peripheral HW in PL subsystem 
   - Script base Vivado project implementation
   - a device driver with DMA interrupt handler
   - a device driver for out-stream device
10. SMP (Symmetric Multiprocessor)
   - CPU 0 and CPU 1 boots up and running with the identical kernel
   - PERCPU resource storage
   - SGI (Software Generated Interrupt) between CPU 0 and CPU 1
   - Mailbox for communication between CPU 0 and CPU 1 

Build system:
   - CMake build, Make build both are working
   - Kconfig
   - Statically linked device tree for device driver

Refer A little book on custom OS developement from scratch.pdf for more information.

Notes: To petalinux-package for cora z7, petalinux v2023.2 is used and verified

# Getting Started
 
  SLOS is tested and running in Cora z7 dual core product.
  Getting started is based on Ubuntu 22.04, Vivado 2023.2, Petalinux 2019.2.
  Vivado is needed to run xsdb which is served as gdb server.
  Petalinux is needed to package the Xilinx BOOT.BIN which is loaded Xilinx first stage bootloader.
 
  ## Build 
  1. Download SLOS (https://github.com/chungae9ri/slos)
 
  2. Install Python 3.10.x and CMake > 3.27
 
  3. Install ARM GCC toolchain and add the toolchain bin location to the path.
     Current version used in SLOS build is gcc-arm-none-eabi-10.3-2021.10.
 
  4. To build 32bit SLOS, there is two way. One is using CMake and the other one is using Make.
     4.1 build-cmake.sh is the build script with CMake. There is help option with -h, it will print help message as,
         Usage: ./build-cmake.sh [options...]
         Options:
           -a    build all targets
           -c    clean builds
           -p    build apps
           -r    build ramdisk
           -k    build kernel32
           -l    build kernel64
           -h    show help message
         Run './build-cmake.sh -prk' to build 32bit SLOS and the output is placed in build/kernel32/kernel32.elf.
         kernel32.elf is SLOS monolithic executable image including user applications in a ramdisk.img.
     4.2 For using Make, just run make
     
  5. Package kernel32.elf into Xilinx boot image - BOOT.BIN
     For this, Petalinux SDK tools are used.
     5.1 Petlinux SDK is used to generate the BOOT.BIN. There is genesis.xsa in bins folder for SLOS hardware 
         description used to generate Petalinux project. For Petalinux 2019 installation and how to generate 
         a Petalinux project, refer to Xilinx Petalinux reference manual.
     5.2 Once Petlinux 2019 is installed and generate a Petalinux project by using SLOS hardware description
         file (genesis.xsa), copy provided genesis_2023.bit, zynq_fsbl.elf in biins folder to Petalinux 
         image/linux folder. Copy the kernel32.elf from build/kernel32 folder.
     5.3 Finally run Petalinux tool as 
  ```
         Petalinux-package --boot --fpga genesis_2023.bit --fsbl zynq_fsbl --u-boot=kernel32.elf --force
  ```
 
  ## Boot test
  Uart is used as STDIO for SLOS and there is shell task running for user input.
  For booting SLOS, copy the BOOT.BIN to the SD card and turn power on the Cora z7 board.
  Connect a serial terminal to Cora z7 board. Set correct port, baudrate and Uart configuration.
  Uart configuration is 115200 baudrate, 8N1. Once it boots up, there should be bootup message like below.
  ```
  stdio uart initializedcpu 0 scr: 0x0
  Total 170 number of kmalloc calls(vm region desc) supported!                                    
  init_kernmem done.                                                                                  
  start secondary cpu.                                                                                
  I am cpuidle                                                                                        
  I am cpu 1!                                                                                         
  cpu 1 scr: 0x0                                                                                      
  cpu 0 qworker enq_idx: 0, deq_idx: 0                                                            
  I am cpuidle_secondaryI am shell                                                                    
  shell > cpu 1 qworker enq_idx: 0, deq_idx: 0
  ``` 
  ## Run commands to shell task
 
  1. Press enter, then shell task should show commands it supports as below
  ```
  shell >
  apprun             : run user application in the ramdisk
  cfs task           : create and run test cfs tasks
  oneshot task       : create and run test oneshot task
  rt task            : create and run test rt tasks
  run                : wakeup and run a task with pid
  sgi                : generate sgi interrupt to cpu1
  sleep              : sleep a task with pid
  start cs           : start outstream consumer hw
  start dma          : start dma task
  taskstat           : show task statistics
  test mem           : run memory test task
  whoami, hide whoami: show or hide printing current task name 
  ```
  2. taskstat command
 
  taskstat command shows each task's information. Below is an example of taskstat after bootup.
  If more tasks are added such as CFS task, rt task, then they also are shown by taskstat command.
 
  ```
  shell > taskstat
  **** cpu: 0 taskstat ****
  cfs task:idle task
  pid: 0
  state: 0
  priority: 16
  jiffies_vruntime: 3696
  jiffies_consumed: 231
  cfs task:shell
  pid: 2
  state: 0
  priority: 2
  jiffies_vruntime: 3690
  jiffies_consumed: 1845
  cfs task:workq_worker:0
  pid: 3
  state: 1
  priority: 4
  jiffies_vruntime: 0
  jiffies_consumed: 0
  shell > ### cpu 1 qworker enq_idx: 1, deq_idx: 0
  **** cpu: 1 taskstat ****
  cfs task:idle task secondary
  pid: 1
  state: 0
  priority: 16
  jiffies_vruntime: 33232
  jiffies_consumed: 2077
  cfs task:workq_worker:1
  pid: 4
  state: 0
  priority: 4
  jiffies_vruntime: 33220
  jiffies_consumed: 8305
  ``` 
  3. apprun command
 
  This command load ramdisk and read user application executable and run it. When loading from ramdisk,
  SLOS open a file and save the exeutable as a file (SLFS or littleFS). When executing, SLOS reads
  the executable from file and part it with ELF loader. The user application prints message via system call
  which is provided by libslos.a.
 
  4. cfs task command
 
  This command fork a cfs task and run it. cfs task doesn't do anything but printing its iteration counter and yield 
  the cpu to next task. cfs task is rescheduled every 1 second.
 
  5. oneshot task command
  
  This command fork a oneshot task which is running in 1 second. 
 
  6. rt task command
 
  This command forks two realtime task. SLOS uses EDF (Earliest Deadline First) scheduling for realtime tasks.
  All cfs task also has its own realtime slot (10msec timeout) and this slot is shared among all cfs tasks. 
  There are two other realtime task whose timeout is 120msec and 125msec. These realtime tasks just run its 
  loop for a short duration and yield the cpu to next task.
 
  7. run / sleep command
 
  These command put a task into sleep and wake. Task is chosen by using its pid. Once these commands run,
  shell asks the pid.
 
  8. sgi command
 
  This command runs an sgi (Software Generated Interrupt) to CPU1. Before generating sgi interrupt, it puts
  its letter into mailbox. This letter stores the requests to CPU1. Currently there are only two letters.
  One for taskstat to show CPU1 task status, the other one is starting odev (Outstream Device) task running in CPU1.
  sgi command is for running odev task from CPU1 and odev task reads data from memory, sends it to the
  outstream hardware implemented in the FPGA. In this outstream datapath, there are FIFOs, flow control, consumer
  device. Refer `A little book on the development of custom OS from scratch.pdf' for details.
 
  9. start cs command
  This command start consumer hardware in the outstream datapath. If consumer device doesn't consume
  data in the datapath, after all FIFOs are filled up, the outstream datapath is blocked to avoid overflow
  in the FiFOs. Once this command runs, it starts the consumer device, flush the data in the FIFOs and
  outstream datapath moves the data from memory to consumer again. While moving data from memory to data consumer
  SLOS prints current data ID in the terminal.
 
  10. start dma command
 
  This command start a dma task. dma task initiates the custom DMA device which is implemented in the FPGA and returns.
  ```
  shell > start dma                                                                                   
  dma start, src: 0x10000000, dst: 0x10001000, 0x1000bytes                                            
  shell > dma done!
  ```
  11. test mem command
 
  This command runs a memory allocation test. It allocates 1MiB memory which uses demanding page and writes / reads
  pattern data. It shows following in the terminal.
  ```
  shell > test mem                                                                                    
  shell > I am test_mem worker                                                                        
  test mem pass 
  ```