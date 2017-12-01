global read_buffer
global lookup_table
global move_list
global print_buffer

global sort_moves
global count_unique_moves
global print_number

section .data

move_list: resb 1048576
read_buffer: resb 1024
print_buffer: resb 1024

lookup_table:
db  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0
db  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0
db  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0
db  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0
db  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0
db  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0
db  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0
db  0, 0, 0, 0, 0, 0, 0, 0,-1, 0, 0, 0, 1, 0, 0, 0
db  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0
db  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0
db  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0
db  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0
db  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0
db  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0
db  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,-1, 0, 0
db  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0
db  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0
db  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0
db  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0
db  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0
db  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0
db  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0
db  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0
db  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0
db  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0
db  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0
db  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0
db  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0
db  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0
db  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0
db  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0
db  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0

section .text

; void sort_moves(void *begin:rdi, void *end:rsi)
sort_moves:
	; rax : position
	mov rax, 1

	; rdx : count (end - begin) / 8
	mov rdx, rsi
	sub rdx, rdi
	shr rdx, 3

sort_loop:
	cmp rax, rdx
	jge sort_end

	mov r10, qword [rdi + 8 * rax]
	mov rbx, rax

	back_loop:
		cmp [rdi + 8 * rbx - 8], r10
		jle back_end

		mov r11, [rdi + 8 * rbx - 8]
		mov [rdi + 8 * rbx], r11
		dec rbx
		jz back_end
		jmp back_loop
	back_end:

	mov [rdi + 8 * rbx], r10

	inc rax
	jmp sort_loop

sort_end:
	ret

; U64 count_unique_moves(void *begin:rdi, void *end:rsi)
count_unique_moves:
	; return 0 if empty
	cmp rdi, rsi
	jne do_count
	mov rax, 0
	ret

do_count:
	; counter : 1
	mov rax, 1

	; previous value
	mov r10, qword [rdi]

	add rdi, 8

	unique_loop:
		cmp rdi, rsi
		je unique_end

		mov r11, qword [rdi]
		mov rbx, rax
		inc rbx
		cmp r10, r11
		cmovne rax, rbx
		mov r10, r11

		add rdi, 8
		jmp unique_loop
	unique_end:

	ret

; U64 print_number(void *buffer:rdi, U64 number:rsi)
print_number:
	mov rax, rsi
	mov r10, 10

	; Store the start pointer
	mov rsi, rdi

	; Output the digits in the wrong order using mod 10
number_loop:
	xor rdx, rdx
	div r10
	add edx, 0x30 ; + '0'
	mov [rdi], edx
	inc rdi
	test rax, rax
	jnz number_loop

	; Store the length as the return value
	mov rax, rdi
	sub rax, rsi

	; Flip the digits
	dec rdi
flip_loop:
	cmp rsi, rdi
	jge flip_done
	mov bl, byte [rsi]
	mov cl, byte [rdi]
	mov byte [rsi], cl
	mov byte [rdi], bl
	inc rsi
	dec rdi
	jmp flip_loop

flip_done:
	ret
