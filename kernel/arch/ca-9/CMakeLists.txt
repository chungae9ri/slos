add_library(
  arch_objs OBJECT 
  cache.S
  init-mm.S
  ops.S
  smp-reset.S
  smp-vector.S
  vector.S
)
