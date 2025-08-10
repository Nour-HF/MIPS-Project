    .data
msg:    .asciiz "You typed: "

    .text
main:
    # Print prompt string
    addi $a0, $zero, msg   # load address of msg into $a0 (works if address fits in 16-bit immediate)
    trap 2                 # PRINT_STRING

    # Read a single character (syscall 4) -> result in $v0
    trap 4                 # READ_CHARACTER

    # Move char (in $v0) into $a0 (argument for PRINT_CHARACTER)
    addu $a0, $v0, $zero

    # Print the character
    trap 1                 # PRINT_CHARACTER

    # Print newline (optional) - you can write a newline char into memory and print it,
    # but simplest way is to print character 10 directly:
    addi $a0, $zero, 10
    trap 1

    # Exit
    trap 5