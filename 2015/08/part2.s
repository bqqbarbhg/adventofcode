.text
.globl computed_length

.thumb
.thumb_func
@ int computed_length(const char* data:r0, int *source_len:r1, int *data_len:r2)
computed_length:
        push {r4-r7}
        mov r4, r0

        mov r6, #2

.loop:
        ldrb r5, [r4]
        cbz r5, .done
        cmp r5, #10
        beq .done
        add r4, r4, #1

        add r6, r6, #1

        cmp r5, #34
        beq .escape
        cmp r5, #92
        beq .escape
        b .loop

.escape:
        add r6, r6, #1
        b .loop

.done:
        sub r4, r4, r0
        str r4, [r2]
        str r6, [r1]
        movs r0, #1
        pop {r4-r7}
        bx lr
