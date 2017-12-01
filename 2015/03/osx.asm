global os_write_stdout
global os_read_stdin
global os_exit

section .text

; U64 os_write_stdout(const void * buffer:rdi, U64 size:rsi)
os_write_stdout:

	; Note the order: rsi is overwritten!
	mov rdx, rsi ; syscall.length: size
	mov rsi, rdi ; syscall.pointer: buffer
	mov rdi, 1   ; syscall.fd: 1 (stdout)

	mov rax, 0x2000004 ; Write
	syscall

	; Return value already in eax
	ret

; U64 os_read_stdin(void * buffer:rdi, U64 size:rsi)
os_read_stdin:

	; Note the order: rsi is overwritten!
	mov rdx, rsi ; syscall.length: size
	mov rsi, rdi ; syscall.pointer: buffer
	mov rdi, 0   ; syscall.fd: 0 (stdin)

	mov rax, 0x2000003 ; Read
	syscall

	; Return value already in eax
	ret

; void os_exit(I64 code:rdi)
os_exit:
	mov rax, 0x2000001 ; Exit
	; Exit code is already in rdi
	syscall

	ret

