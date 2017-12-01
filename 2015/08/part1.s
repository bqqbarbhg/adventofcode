.text
.globl computed_length

.thumb
.thumb_func
@ int computed_length(const char* data:r0, int *source_len:r1, int *data_len:r2)
computed_length:
        push {r4-r7}
        mov r4, r0

        ldrb r5, [r4]
        cmp r5, #34
        bne .fail
        add r4, r4, #1

        mov r6, #0

.loop:
        ldrb r5, [r4]
        add r4, r4, #1

        cmp r5, #0
        beq .fail

        cmp r5, #34
        beq .done

        add r6, r6, #1

        cmp r5, #92
        bne .loop

        ldrb r5, [r4]
        add r4, r4, #1
        cbz r5, .fail

        cmp r5, #120
        bne .loop

        ldrb r5, [r4]
        add r4, r4, #1
        cbz r5, .fail

        ldrb r5, [r4]
        add r4, r4, #1
        cbz r5, .fail

        b .loop

.done:
        sub r4, r4, r0
        str r4, [r1]
        str r6, [r2]
        movs r0, #1
        pop {r4-r7}
        bx lr

.fail:
        movs r0, #0
        pop {r4-r7}
        bx lr
