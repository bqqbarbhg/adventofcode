; Memory map:
; $f001  putchar(W)  R=getchar()
; $f002  exit(W)

.data

del_stack:
    .res 256

score_list: .res 256*6

.zeropage

score_sum: .res 6
score_tmp: .res 6
num_scores: .res 1
print_zeros: .res 1
bubble_continue:
final_char: .res 1

.code

main:

@main_loop:
    jsr process_line

    ; Add the score to the list if X != 0
    cpx #0
    beq @skip_add
    ldx num_scores
    .repeat 6,I
        lda score_sum+I
        sta score_list+I*256,X
    .endrepeat
    inx
    stx num_scores
@skip_add:

    ; Keep reading lines until main_loop returns Y=0
    cpy #0
    bne @main_loop

    ; Bubble sort the scores
    jsr bubble_sort

    ; Copy the middle score to the zero page
    lda num_scores
    lsr
    tax
    .repeat 6,I
        lda score_list+I*256,X
        sta score_sum+I
    .endrepeat

    ; Print the score
    .repeat 6,I
        lda score_sum+5-I
        jsr print_bcd
    .endrepeat

    ; Print a zero if we did not print anything
    lda print_zeros
    bne @no_zero
    lda #$30 ; '0'
    sta $f001 ; putchar()
@no_zero:

    ; Write return code 0 to f002 to exit
    lda #$00
    sta $f002 ; exit()

; Process a line of input, returns the last characer read in Y
; Returns X=1 if the score in $10-$13 should be added to the list
process_line:

    ; Reset delimiter stack
    ldx #0

    ; Reset the current score
    .repeat 6,I
        stx score_sum+I
    .endrepeat

@loop:
    ; Read input byte from stream
    ldy $f001 ; getchar()

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
    sty final_char

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
    .repeat 6,I
        adc score_sum+I
        sta score_sum+I
        lda #0
    .endrepeat
    cld

    jmp @incomplete_loop
@incomplete_done:
    ; Report as incomplete and load spilled final_char
    ldx #1
    ldy final_char
    rts

@skip_loop:
    ; Read input byte from stream
    ldy $f001 ; getchar()

    ; Return from the loop if we hit \0 or \n
    beq @ret
    cpy #$0a ; '\n'
    beq @ret

    jmp @skip_loop

@ret:
    ldx #0
    rts

; Multiply BCD score_sum by 5 (clobbers A,Y,score_tmp)
mul_bcd_5:

    ; Copy the score to a temporary slot
    .repeat 6,I
        lda score_sum+I
        sta score_tmp+I
    .endrepeat

    ldy #0
@mul_loop:

    ; Add $15-$19 to $10-$14 in BCD
    sed
    clc
    .repeat 6,I
        lda score_sum+I
        adc score_tmp+I
        sta score_sum+I
    .endrepeat
    cld

    ; Repeat the addition 5 times
    iny
    cpy #4
    bne @mul_loop

    rts

bubble_swap:
    .repeat 6,I
        lda score_list+I*256,X   ; A = score_list[X]
        ldy score_list+I*256-1,X ; Y = score_list[X-1]
        sta score_list+I*256-1,X ; score_list[X-1] = A
        tya                      ; A = Y
        sta score_list+I*256,X   ; score_list[X-1] = A
    .endrepeat
    rts

; Bubble sort scroes
bubble_sort:
    ; Iterate backwards
    ldx num_scores
    dex

    ; Reset mark that indicates whether we need an extra iteration
    ldy #0
    sty bubble_continue

@loop:
    ; Subtract two consecutive values in BCD to get the carry flag
    sed
    clc
    .repeat 6,I
        lda score_list+I*256,X
        sbc score_list+I*256-1,X
    .endrepeat
    cld

    ; Swap the values if necessary and mark that we need to continue sorting
    bcs @skip_swap
    jsr bubble_swap
    ldy #1
    sty bubble_continue
@skip_swap:

    ; Loop until we reach 0
    dex
    bne @loop

    ; Do another iteration if we did at least one swap
    lda bubble_continue
    bne bubble_sort

    rts

; Print two BCD digits at A
; print_zeros contains 0 if leading zeros should not be printed, ff otherwise
print_bcd:
    ; Use Y as loop counter
    ldy #2

    ; Store the pre-shifted input to X
    tax

    ; Shift the high nibble to low as we want to print it first
    .repeat 4
        lsr
    .endrepeat
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
    cmp print_zeros
    beq @loop

    ; Print the digit
    clc
    adc #$30 ; '0'
    sta $f001 ; putchar()

    ; Mark that leading zeroes should be printed
    lda #$ff
    sta print_zeros

    jmp @loop

@end:
   rts

