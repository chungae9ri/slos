#add_executable(kernel.elf main.c)
add_executable(${KERNEL_ELF} main.c)

#set_target_properties(kernel.elf PROPERTIES RUNTIME_OUTPUT_DIRECTORY ../)
set_target_properties(${KERNEL_ELF} PROPERTIES RUNTIME_OUTPUT_DIRECTORY ../)