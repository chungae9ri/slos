
add_library(
  core_objs OBJECT
  faults.c
  frame_pool.c
  initrd.S
  ktimer.c
  loader.c
  mailbox.c
  main.c
  mm.c
  page_table.c
  percpu.c
  printk.c
  rbtree.c
  runq.c
  sched.c
  sgi.c
  task.c
  timer.c
  vm_pool.c
  waitq.c
)

target_include_directories(core_objs PRIVATE ../fs/littlefs)
