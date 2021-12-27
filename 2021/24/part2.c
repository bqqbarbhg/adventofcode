#define _CRT_SECURE_NO_WARNINGS

#define X64A_IMPLEMENTATION
#include "x64a.h"

#include <stdio.h>

#if defined(_WIN32)
#define WIN32_LEAN_AND_MEAN
#define NOMINMAX
#include <Windows.h>

static void *alloc_exec(const void *code, size_t length)
{
	void *pages = VirtualAlloc(NULL, length, MEM_COMMIT, PAGE_READWRITE);
	if (!pages) return NULL;
	memcpy(pages, code, length);
	DWORD prev_protect = 0;
	if (!VirtualProtect(pages, length, PAGE_EXECUTE_READ, &prev_protect)) {
		VirtualFree(pages, 0, MEM_RELEASE);
		return NULL;
	}
	return pages;
}

static void free_exec(void *ptr)
{
	VirtualFree(ptr, 0, MEM_RELEASE);
}

#endif

void assemble_lines(x64_assembler *a, size_t sym_prev, size_t depth, size_t prefix_len)
{
	char line[128];
	for (;;) {
		if (feof(stdin)) {
			x64_test64_reg_rm(a, X64_R11, X64_R11);
			x64_jcc_symbol(a, X64_CC_NZ, sym_prev);
			x64_mov64_rm_imm(a, X64_RAX, 1);
			x64_mov64_reg_rm(a, X64_RDI, x64_mem(X64_RSP, -24));
			x64_mov64_reg_rm(a, X64_RBX, x64_mem(X64_RSP, -16));
			x64_ret(a);
			return;
		}

		fgets(line, sizeof(line), stdin);

		char ca, cb;
		int32_t imm;
		if (sscanf(line, "inp %c", &ca) == 1) {
			uint32_t ai = (uint32_t)(ca - 'w');
			x64_reg ra = X64_R8 + ai;
			size_t base = depth * 32;

			if (depth < prefix_len) {
				x64_mov64_reg_rm(a, ra, x64_mem(X64_RDI, depth*8));

				// Store all registers to stack
				x64_mov64_rm_reg(a, x64_mem(X64_RCX, base+((4-ai)%4*8)), X64_R8);
				x64_mov64_rm_reg(a, x64_mem(X64_RCX, base+((5-ai)%4*8)), X64_R9);
				x64_mov64_rm_reg(a, x64_mem(X64_RCX, base+((6-ai)%4*8)), X64_R10);
				x64_mov64_rm_reg(a, x64_mem(X64_RCX, base+((7-ai)%4*8)), X64_R11);

				assemble_lines(a, sym_prev, depth + 1, prefix_len);
			} else {
				x64_mov64_rm_imm(a, ra, 0);

				// Store all registers to stack
				x64_mov64_rm_reg(a, x64_mem(X64_RCX, base+((4-ai)%4*8)), X64_R8);
				x64_mov64_rm_reg(a, x64_mem(X64_RCX, base+((5-ai)%4*8)), X64_R9);
				x64_mov64_rm_reg(a, x64_mem(X64_RCX, base+((6-ai)%4*8)), X64_R10);
				x64_mov64_rm_reg(a, x64_mem(X64_RCX, base+((7-ai)%4*8)), X64_R11);
				size_t sym_start = x64_new_symbol(a);
				x64_set_label(a, sym_start);

				// Load registers from stack
				x64_mov64_reg_rm(a, X64_R8,  x64_mem(X64_RCX, base+((4-ai)%4*8)));
				x64_mov64_reg_rm(a, X64_R9,  x64_mem(X64_RCX, base+((5-ai)%4*8)));
				x64_mov64_reg_rm(a, X64_R10, x64_mem(X64_RCX, base+((6-ai)%4*8)));
				x64_mov64_reg_rm(a, X64_R11, x64_mem(X64_RCX, base+((7-ai)%4*8)));

				// Increment the current counter
				x64_add64_rm_imm(a, ra, 1);
				x64_cmp64_rm_imm(a, ra, 10);
				x64_jcc_symbol(a, X64_CC_Z, sym_prev);

				// Store the decremented value back to the stack
				x64_mov64_rm_reg(a, x64_mem(X64_RCX, base), ra);

				assemble_lines(a, sym_start, depth + 1, prefix_len);
			}
			return;

		} else if (sscanf(line, "add %c %d", &ca, &imm) == 2) {
			uint32_t ai = (uint32_t)(ca - 'w');
			x64_reg ra = X64_R8 + ai;
			if (imm == 0) {
				// Nop
			} else {
				x64_add64_rm_imm(a, ra, imm);
			}
		} else if (sscanf(line, "add %c %c", &ca, &cb) == 2) {
			uint32_t ai = (uint32_t)(ca - 'w'), bi = (uint32_t)(cb - 'w');
			x64_reg ra = X64_R8 + ai, rb = X64_R8 + bi;
			x64_add64_reg_rm(a, ra, rb);
		} else if (sscanf(line, "mul %c %d", &ca, &imm) == 2) {
			uint32_t ai = (uint32_t)(ca - 'w');
			x64_reg ra = X64_R8 + ai;
			if (imm == 0) {
				x64_xor64_reg_rm(a, ra, ra);
			} else if (imm == 1) {
				// Nop
			} else {
				x64_imul64_reg_rm_imm(a, ra, ra, imm);
			}
		} else if (sscanf(line, "mul %c %c", &ca, &cb) == 2) {
			uint32_t ai = (uint32_t)(ca - 'w'), bi = (uint32_t)(cb - 'w');
			x64_reg ra = X64_R8 + ai, rb = X64_R8 + bi;
			x64_imul64_reg_rm(a, ra, rb);
		} else if (sscanf(line, "div %c %d", &ca, &imm) == 2) {
			uint32_t ai = (uint32_t)(ca - 'w');
			x64_reg ra = X64_R8 + ai;
			if (imm == 1) {
				// Nop
			} else {
				x64_mov64_reg_rm(a, X64_RAX, ra);
				x64_xor64_reg_rm(a, X64_RDX, X64_RDX);
				x64_mov64_rm_imm(a, X64_RBX, imm);
				x64_idiv64_ax_rm(a, X64_RBX);
				x64_mov64_reg_rm(a, ra, X64_RAX);
			}
		} else if (sscanf(line, "div %c %c", &ca, &cb) == 2) {
			uint32_t ai = (uint32_t)(ca - 'w'), bi = (uint32_t)(cb - 'w');
			x64_reg ra = X64_R8 + ai, rb = X64_R8 + bi;

			x64_test64_reg_rm(a, rb, rb);
			x64_jcc_symbol(a, X64_CC_Z, sym_prev);

			x64_mov64_reg_rm(a, X64_RAX, ra);
			x64_xor64_reg_rm(a, X64_RDX, X64_RDX);
			x64_idiv64_ax_rm(a, rb);
			x64_mov64_reg_rm(a, ra, X64_RAX);
		} else if (sscanf(line, "mod %c %d", &ca, &imm) == 2) {
			uint32_t ai = (uint32_t)(ca - 'w');
			x64_reg ra = X64_R8 + ai;

			x64_cmp64_rm_imm(a, ra, 0);
			x64_jcc_symbol(a, X64_CC_L, sym_prev);

			x64_mov64_reg_rm(a, X64_RAX, ra);
			x64_xor64_reg_rm(a, X64_RDX, X64_RDX);
			x64_mov64_rm_imm(a, X64_RBX, imm);
			x64_idiv64_ax_rm(a, X64_RBX);
			x64_mov64_reg_rm(a, ra, X64_RDX);
		} else if (sscanf(line, "mod %c %c", &ca, &cb) == 2) {
			uint32_t ai = (uint32_t)(ca - 'w'), bi = (uint32_t)(cb - 'w');
			x64_reg ra = X64_R8 + ai, rb = X64_R8 + bi;

			x64_cmp64_rm_imm(a, ra, 0);
			x64_jcc_symbol(a, X64_CC_L, sym_prev);
			x64_cmp64_rm_imm(a, rb, 0);
			x64_jcc_symbol(a, X64_CC_LE, sym_prev);

			x64_mov64_reg_rm(a, X64_RAX, ra);
			x64_xor64_reg_rm(a, X64_RDX, X64_RDX);
			x64_idiv64_ax_rm(a, rb);
			x64_mov64_reg_rm(a, ra, X64_RDX);
		} else if (sscanf(line, "eql %c %d", &ca, &imm) == 2) {
			uint32_t ai = (uint32_t)(ca - 'w');
			x64_reg ra = X64_R8 + ai;
			x64_cmp64_rm_imm(a, ra, imm);
			x64_setcc8_rm(a, X64_CC_E, X64_RAX);
			x64_emit(a, 0x0fb6, X64_F_REGRM, ra, X64_RAX, 0); // movzx r32, r/m8
		} else if (sscanf(line, "eql %c %c", &ca, &cb) == 2) {
			uint32_t ai = (uint32_t)(ca - 'w'), bi = (uint32_t)(cb - 'w');
			x64_reg ra = X64_R8 + ai, rb = X64_R8 + bi;
			x64_cmp64_reg_rm(a, ra, rb);
			x64_setcc8_rm(a, X64_CC_E, X64_RAX);
			x64_emit(a, 0x0fb6, X64_F_REGRM, ra, X64_RAX, 0); // movzx r32, r/m8
		}
	}
}

void assemble(x64_assembler *a, size_t prefix_len)
{
	size_t sym_fail = x64_new_symbol(a);
	size_t sym_nofail = x64_new_symbol(a);

	// Prologue
	x64_mov64_rm_reg(a, x64_mem(X64_RSP, -16), X64_RBX);
	x64_mov64_rm_reg(a, x64_mem(X64_RSP, -24), X64_RDI);

	x64_mov64_rm_reg(a, X64_RDI, X64_RDX);

	// Clear the initial registers
	x64_xor64_reg_rm(a, X64_R8, X64_R8);
	x64_xor64_reg_rm(a, X64_R9, X64_R9);
	x64_xor64_reg_rm(a, X64_R10, X64_R10);
	x64_xor64_reg_rm(a, X64_R11, X64_R11);

	// Fail branch
	x64_jmp_symbol(a, sym_nofail);
	x64_set_label(a, sym_fail);
	x64_mov32_rm_imm(a, X64_RAX, 0);
	x64_mov64_reg_rm(a, X64_RDI, x64_mem(X64_RSP, -24));
	x64_mov64_reg_rm(a, X64_RBX, x64_mem(X64_RSP, -16));
	x64_ret(a);

	x64_set_label(a, sym_nofail);
	assemble_lines(a, sym_fail, 0, prefix_len);
}

#define PREFIX_LEN 5

int main(int argc, char **argv)
{
	x64_assembler assembler = { 0 };

	assemble(&assembler, PREFIX_LEN);
	x64_optimize_relocs(&assembler);
	x64_patch_relocs(&assembler);

	void *exec = alloc_exec(assembler.code, assembler.code_length);

	typedef bool test_fn(int64_t *stack, int64_t *prefix);
	test_fn *test = (test_fn*)exec;

	int full_prefix = 9;
	for (size_t i = 1; i < PREFIX_LEN; i++ ){
		full_prefix = full_prefix * 10 + 9;
	}

	int min_prefix = INT_MAX;
	int64_t min_result = INT64_MAX;

	int prefix;
	#pragma omp parallel for schedule(dynamic, 1)
	for (prefix = 0; prefix <= full_prefix; prefix++) {
		bool bad = false;

		if (prefix > min_prefix) bad = true;

		int64_t pre = prefix;
		int64_t prefix_buf[PREFIX_LEN];
		for (size_t i = PREFIX_LEN; i > 0; i--) {
			int64_t digit = pre % 10;
			if (digit == 0) {
				bad = true;
				break;
			}
			prefix_buf[i - 1] = digit;
			pre /= 10;
		}

		if (!bad) {
			int64_t stack[14][4] = { 0 };
			if (test((int64_t*)stack, prefix_buf)) {
				int64_t result = 0;
				for (size_t i = 0; i < 14; i++) {
					result = result * 10 + stack[i][0];
				}

				#pragma omp critical
				{
					if (result < min_result) {
						min_prefix = prefix;
						min_result = result;
					}
				}
			}
		}
	}

	free_exec(exec);

	printf("%llu\n", min_result);
	return 0;
}
