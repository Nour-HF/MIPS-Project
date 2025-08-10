.text
main:
    addi $t0, $zero, 5      # t0 = 5
    addi $t1, $zero, 7      # t1 = 7
    add  $v0, $t0, $t1      # v0 = t0 + t1 = 12
    add  $a0, $zero, $v0    # a0 = result
    trap 0                  # print_int
    trap 5