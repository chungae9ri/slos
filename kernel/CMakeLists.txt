set(KERNEL_ELF kernel.elf)
add_subdirectory(arm)
add_subdirectory(core)
add_subdirectory(drivers)


#target_include_directories(kernel.elf PUBLIC inc)
target_include_directories(${KERNEL_ELF} PUBLIC inc)
