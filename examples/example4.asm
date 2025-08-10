.data
msg: .asciiz "Hello World\n"

.text
main:
    addi $a0, $zero, 12     # msg starts at address 8 (after 2 instructions)
    trap 2                 # print_string
    trap 5