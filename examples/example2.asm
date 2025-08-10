    .text
main:

    addi $a0, $zero, 42    # arg = 42
    addi $v0, $zero, 1     # syscall 1 = print_int
    trap