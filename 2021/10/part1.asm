; Memory map:
; $f001  putchar(W)  R=getchar()
; $f002  exit(W)

.data

del_stack:
    .res 256

score_tab:
    .byte $03,$00,$00
    .byte $57,$00,$00
    .byte $97,$11,$00
    .byte $37,$51,$02

score:
    .res 4

.code

main:

@main_loop:
    jsr process_line
    cpy #0
    bne @main_loop

    ; Print the score
    lda score+3
    jsr print_bcd
    lda score+2
    jsr print_bcd
    lda score+1
    jsr print_bcd
    lda score+0
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
process_line:

    ; Reset delimiter stack
    ldx #0

@loop:
    ; Read input byte from stream
    ldy $f001

    ; Return from the loop if we hit \0 or \n
    beq @ret
    cpy #$0a
    beq @ret

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
    beq @corrupt

    ; Compare the current character with the pushed one
    dex
    cmp del_stack,X
    bne @corrupt

    ; OK: continue to the next character
    jmp @loop

@corrupt:

    ; Add the score in BCD
    sed
    clc
    lda score+0
    adc score_tab+0,Y
    sta score+0
    lda score+1
    adc score_tab+1,Y
    sta score+1
    lda score+2
    adc score_tab+2,Y
    sta score+2
    lda score+3
    adc #0
    sta score+3
    cld

    ; !! fallthrough to @skip_loop !!
    ; We only care about the first error so skip the rest of the line

@skip_loop:
    ; Read input byte from stream
    ldy $f001

    ; Return from the loop if we hit \0 or \n
    beq @ret
    cpy #$0a
    beq @ret

    jmp @skip_loop

@ret:
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
