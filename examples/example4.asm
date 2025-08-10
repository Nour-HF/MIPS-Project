    .data
msg: .asciiz "Hello World\n"

    .text
main:
    addi $a0, $zero, msg   # address of string
    trap  2