    .text
main:
    addi $t0, $zero, 5      # t0 = 5
    addi $t1, $zero, 7      # t1 = 7
    add  $v0, $t0, $t1      # v0 = t0 + t1 = 12

    add $a0, $zero, $v0    # arg = 42
    addi $v0, $zero, 1     # syscall 1 = print_int
    trap