global start

extern os_write_stdout
extern os_read_stdin
extern os_exit

extern read_buffer
extern lookup_table
extern move_list
extern print_buffer

extern sort_moves
extern count_unique_moves
extern print_number

section .text

start:

	sub rsp, 24

	; Santa's location
	mov dword [rsp], 0      ; Santa X
	mov dword [rsp + 4], 0  ; Santa Y
	mov dword [rsp + 8], 0      ; Robo-Santa X
	mov dword [rsp + 12], 0  ; Robo-Santa Y

	; Store the initial Santa position
	lea r9, [rel move_list]
	mov qword [r9], 0
	mov qword [r9 + 8], 0
	add r9, 16
	mov [rsp + 16], r9 ; save the pointer to stack

read_loop:

	; Read a block of input
	lea r15, [rel read_buffer]
	mov rdi, r15
	mov rsi, 1024
	call os_read_stdin

	; Quit the loop if nothing read
	test rax, rax
	jz read_done

	; Load the registers from stack
	lea r8, [rel lookup_table] ; r8 = lookup table
	mov r9, qword [rsp + 16] ; r9 = move list
	mov r10d, dword [rsp]     ; r10 = Santa X
	mov r11d, dword [rsp + 4] ; r11 = Santa Y
	mov r12d, dword [rsp + 8] ; r12 = Robo-Santa X
	mov r13d, dword [rsp +12] ; r13 = Robo-Santa Y

	; Setup the loop (rsi -> rdi)
	mov rsi, r15
	mov rdi, r15
	add rdi, rax
	loop:
		cmp rsi, rdi
		je done

		; Lookup the delta x and y for the next character
		movzx rax, byte [rsi]
		lea rax, [r8 + rax * 2]

		; Move Santa by the looked up values
		movsx ebx, byte [rax]
		movsx ecx, byte [rax + 1]
		add r10d, ebx
		add r11d, ecx

		mov dword [r9], r10d     ; Store santa X
		mov dword [r9 + 4], r11d ; Store santa Y
		add r9, 8 ; Increment move list position

		inc rsi
		cmp rsi, rdi
		je done

		; Lookup the delta x and y for the next character
		movzx rax, byte [rsi]
		lea rax, [r8 + rax * 2]

		; Move Robo-Santa by the looked up values
		movsx ebx, byte [rax]
		movsx ecx, byte [rax + 1]
		add r12d, ebx
		add r13d, ecx

		mov dword [r9], r12d     ; Store robo-santa X
		mov dword [r9 + 4], r13d ; Store robo-santa Y
		add r9, 8 ; Increment move list position

		inc rsi

		jmp loop

	done:
	
	; Spill the registers to stack and read another block
	mov dword [rsp], r10d     ; Santa X = r10
	mov dword [rsp + 4], r11d ; Santa Y = r11
	mov dword [rsp + 8], r12d ; Robo-Santa X = r12
	mov dword [rsp +12], r13d ; Robo-Santa Y = r13
	mov qword [rsp +16], r9 ; Move list pointer = r9
	jmp read_loop

read_done:

	; Sort the move list
	lea rdi, [rel move_list]
	mov rsi, [rsp + 16]
	call sort_moves

	; Count the unique elements
	lea rdi, [rel move_list]
	mov rsi, [rsp + 16]
	call count_unique_moves

	; Print the count to buffer
	lea rdi, [rel print_buffer]
	mov rsi, rax
	call print_number

	lea rdi, [rel print_buffer]

	; Add newline
	mov byte [rdi + rax], 10
	inc rax

	; Write the number to stdout
	mov rsi, rax
	call os_write_stdout
	
	mov rdi, 0
	call os_exit
	
	add rsp, 24


