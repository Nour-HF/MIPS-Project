    .text
main:
    addi $t0, $zero, 5      # t0 = 5
    addi $t1, $zero, 7      # t1 = 7
    add  $v0, $t0, $t1      # v0 = t0 + t1 = 12
    # optionally check memory store/load
    sw   $v0, 0($sp)        # store v0 at top of stack
    lw   $t2, 0($sp)        # t2 = v0
    trap  