set confirm off
set architecture riscv:rv32
target remote 127.0.0.1:6640

# set debug-file-directory build/release/
add-symbol-file build/release/earth.elf
add-symbol-file build/release/grass.elf

set disassemble-next-line auto
set riscv use-compressed-breakpoints no

# maintenance packet Qqemu.sstep=0x0

# # show pid of the current running app
# disp/d proc_set[proc_curr_idx].pid

# # show finished instruction
# disp/i $pc-4
