    .text
main:

    addi $a0, $zero, 42    # arg = 42
    trap 0    # syscall 1 = print_int
    trap 5