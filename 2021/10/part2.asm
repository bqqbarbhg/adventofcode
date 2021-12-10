; Memory map:
; $f001  putchar(W)  R=getchar()
; $f002  exit(W)

.data

del_stack:
    .res 256

score_a: .res 256
score_b: .res 256
score_c: .res 256
score_d: .res 256
score_e: .res 256
score_f: .res 256

.code

main:

@main_loop:
    jsr process_line

    ; Add the score to the list if X != 0
    cpx #0
    beq @skip_add
    ldx $30
    lda $10
    sta score_a,X
    lda $11
    sta score_b,X
    lda $12
    sta score_c,X
    lda $13
    sta score_d,X
    lda $14
    sta score_e,X
    lda $15
    sta score_f,X
    inx
    stx $30
@skip_add:

    cpy #0
    bne @main_loop

    ; Bubble sort the scores
    jsr bubble_sort

    ; DEBUG: Print the values
    ldx #0
@temploop:

    lda score_a,X
    sta $10
    lda score_b,X
    sta $11
    lda score_c,X
    sta $12
    lda score_d,X
    sta $13
    lda score_e,X
    sta $14
    lda score_f,X
    sta $15
    sta $f110

    inx
    cpx $30
    bne @temploop

    ; Copy the middle score to the zero page
    lda $30
    lsr
    tax
    lda score_a,X
    sta $10
    lda score_b,X
    sta $11
    lda score_c,X
    sta $12
    lda score_d,X
    sta $13
    lda score_e,X
    sta $14
    lda score_f,X
    sta $15

    ; Print the score
    lda $15
    jsr print_bcd
    lda $14
    jsr print_bcd
    lda $13
    jsr print_bcd
    lda $12
    jsr print_bcd
    lda $11
    jsr print_bcd
    lda $10
    jsr print_bcd

    ; Print a zero if we did not print anything
    lda $02
    bne @no_zero
    lda #$30 ; '0'
    sta $f001
@no_zero:

    ; Write return code 0 to f002 to exit
    lda #$00
    sta $f002

; Process a line of input, returns the last characer read in Y
; Returns X=1 if the score in $10-$13 should be added to the list
process_line:

    ; Reset delimiter stack
    ldx #0

    ; Reset the current score
    stx $10
    stx $11
    stx $12
    stx $13
    stx $14
    stx $15

@loop:
    ; Read input byte from stream
    ldy $f001

    ; Return from the loop if we hit \0 or \n
    beq @incomplete
    cpy #$0a
    beq @incomplete

    ; Push opening delimiters, in ASCII closing ')' is next to '('
    ; The rest have one character between.
    cpy #$28 ; '('
    beq @push1
    cpy #$5b ; '['
    beq @push2
    cpy #$7b ; '{'
    beq @push2
    cpy #$3c ; '<'
    beq @push2

    ; Move the character to A as we will use Y for the score
    tya

    ; Pop closing delimiters and setup the BCD score to the zero page
    ldy #0
    cmp #$29 ; ')'
    beq @pop
    ldy #3
    cmp #$5d ; ']'
    beq @pop
    ldy #6
    cmp #$7d ; '}'
    beq @pop
    ldy #9
    cmp #$3e ; '>'
    beq @pop

    ; Ignore unknown characters
    jmp @loop

    ; Push a character to the stack, increment A once or twice to
    ; get the closing delimiter.
@push2: iny
@push1: iny
    tya
    sta del_stack,X
    inx
    jmp @loop

@pop:
    ; Empty stack
    cpx 0
    beq @skip_loop

    ; Compare the current character with the pushed one
    dex
    cmp del_stack,X
    bne @skip_loop

    ; OK: continue to the next character
    jmp @loop

@incomplete:
    ; Remember the final character
    sty $20

    ; Loop through the pushed characters
@incomplete_loop:
    cpx #0
    beq @incomplete_done
    dex

    lda del_stack,X

    ldy #1
    cmp #$29 ; ')'
    beq @incomplete_char
    iny
    cmp #$5d ; ']'
    beq @incomplete_char
    iny
    cmp #$7d ; '}'
    beq @incomplete_char
    iny
    cmp #$3e ; '>'

    ; Multiply the score by 5 and add the per-character score
@incomplete_char:

    ; Push X and Y to stack, but pop Y -> A as we use it for addition
    tya
    pha
    jsr mul_bcd_5
    pla

    ; Do the addition in BCD
    sed
    clc
    adc $10
    sta $10
    lda #0
    adc $11
    sta $11
    lda #0
    adc $12
    sta $12
    lda #0
    adc $13
    sta $13
    cld
    lda #0
    adc $14
    sta $14
    cld
    lda #0
    adc $15
    sta $15
    cld

    jmp @incomplete_loop
@incomplete_done:
    ; TODO
    ldx #1
    ldy $20
    rts

@skip_loop:
    ; Read input byte from stream
    ldy $f001

    ; Return from the loop if we hit \0 or \n
    beq @ret
    cpy #$0a
    beq @ret

    jmp @skip_loop

@ret:
    ldx #0
    rts

; Multiply BCD $10-$15 by 5 (clobbers A,Y, $20-$25)
mul_bcd_5:

    ; Copy $10-$15 to $20-$25
    lda $10
    sta $20
    lda $11
    sta $21
    lda $12
    sta $22
    lda $13
    sta $23
    lda $14
    sta $24
    lda $15
    sta $25

    ldy #0
@mul_loop:

    ; Add $15-$19 to $10-$14 in BCD
    sed
    clc
    lda $10
    adc $20
    sta $10
    lda $11
    adc $21
    sta $11
    lda $12
    adc $22
    sta $12
    lda $13
    adc $23
    sta $13
    lda $14
    adc $24
    sta $14
    lda $15
    adc $25
    sta $15
    lda $16
    adc $26
    sta $16
    cld

    ; Repeat the addition 5 times
    iny
    cpy #4
    bne @mul_loop

    rts

bubble_swap:
    lda score_a,X
    ldy score_a-1,X
    sta score_a-1,X
    tya
    sta score_a,X
    lda score_b,X
    ldy score_b-1,X
    sta score_b-1,X
    tya
    sta score_b,X
    lda score_c,X
    ldy score_c-1,X
    sta score_c-1,X
    tya
    sta score_c,X
    lda score_d,X
    ldy score_d-1,X
    sta score_d-1,X
    tya
    sta score_d,X
    lda score_e,X
    ldy score_e-1,X
    sta score_e-1,X
    tya
    sta score_e,X
    lda score_f,X
    ldy score_f-1,X
    sta score_f-1,X
    tya
    sta score_f,X
    ldy #1
    sty $31
    rts

; Bubble sort scroes
bubble_sort:
    ; Iterate backwards
    ldx $30
    dex

    ; Reset $31 mark that indicates whether we need an extra iteration
    ldy #0
    sty $31

@loop:
    ; Subtract two consecutive values in BCD
    sed
    clc
    lda score_a,X
    sbc score_a-1,X
    lda score_b,X
    sbc score_b-1,X
    lda score_c,X
    sbc score_c-1,X
    lda score_d,X
    sbc score_d-1,X
    lda score_e,X
    sbc score_e-1,X
    lda score_f,X
    sbc score_f-1,X
    cld

    ; Swap the values if necessary
    bcs @skip_swap
    jsr bubble_swap
@skip_swap:

    ; Loop until we reach 0
    dex
    bne @loop

    ; Do another iteration if we did at least one swap
    lda $31
    bne bubble_sort

    rts

; Print two BCD digits at A
; $02 contains 0 if leading zeros should not be printed, ff otherwise
print_bcd:
    ; Use Y as loop counter
    ldy #2

    ; Store the pre-shifted input to X
    tax

    ; Shift the high nibble to low as we want to print it first
    lsr
    lsr
    lsr
    lsr
    jmp @loop_a

@loop:
    ; Load the input from X
    txa
    and #$0f

    ; Decrement the loop counter
    dey
    beq @end

@loop_a:
    ; Skip printing leading zeroes
    cmp $02
    beq @loop

    ; Print the digit
    clc
    adc #$30 ; '0'
    sta $f001

    ; Mark that leading zeroes should be printed
    lda #$ff
    sta $02

    jmp @loop

@end:
   rts

