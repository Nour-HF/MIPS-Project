.text
main:
    trap 3              # read_int
    addi $a0, $v0, 5    # add 5
    trap 0              # print_int
    trap 5