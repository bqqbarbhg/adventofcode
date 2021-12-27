#ifndef X64A_H_INCLUDED
#define X64A_H_INCLUDED

#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <stdbool.h>
#include <assert.h>

typedef enum x64_reg {
	X64_RAX,
	X64_RCX,
	X64_RDX,
	X64_RBX,
	X64_RSP,
	X64_RBP,
	X64_RSI,
	X64_RDI,
	X64_R8,
	X64_R9,
	X64_R10,
	X64_R11,
	X64_R12,
	X64_R13,
	X64_R14,
	X64_R15,
} x64_reg;

typedef enum x64_cc {
	X64_CC_O   = 0x0,
	X64_CC_NO  = 0x1,
	X64_CC_B   = 0x2,
	X64_CC_NAE = 0x2,
	X64_CC_C   = 0x2,
	X64_CC_NB  = 0x3,
	X64_CC_AE  = 0x3,
	X64_CC_NC  = 0x3,
	X64_CC_Z   = 0x4,
	X64_CC_E   = 0x4,
	X64_CC_NZ  = 0x5,
	X64_CC_NE  = 0x5,
	X64_CC_BE  = 0x6,
	X64_CC_NA  = 0x6,
	X64_CC_NBE = 0x7,
	X64_CC_A   = 0x7,
	X64_CC_S   = 0x8,
	X64_CC_NS  = 0x9,
	X64_CC_P   = 0xa,
	X64_CC_PE  = 0xa,
	X64_CC_NP  = 0xb,
	X64_CC_PO  = 0xb,
	X64_CC_L   = 0xc,
	X64_CC_NGE = 0xc,
	X64_CC_NL  = 0xd,
	X64_CC_GE  = 0xd,
	X64_CC_LE  = 0xe,
	X64_CC_NG  = 0xe,
	X64_CC_NLE = 0xf,
	X64_CC_G   = 0xf,
} x64_cc;

typedef enum x64_flag {

	// Encoding options:

	// Instruction contains a REG/RM byte.
	X64_F_REGRM = 0x1,
	// Immediate is sized and should wrap instead of asserting range.
	X64_F_IMMSZ = 0x2,
	// REX prefix W bit is set (use 64-bit data).
	X64_F_REXW = 0x8,

	// Immediate sizes:

	// 8-bit signed immediate
	X64_F_IMM8 = 0x10,
	// 16-bit signed immediate
	X64_F_IMM16 = 0x20,
	// 32-bit signed immediate
	X64_F_IMM32 = 0x40,
	// 64-bit signed immediate
	X64_F_IMM64 = 0x80,

	// Immediate encoding special cases:

	// If the immediate fits in 8 bits encode it with a 8-bit immediate and
	// increment the opcode by 0x2.
	X64_F_IMM8OP2 = 0x100,
	// If the immediate does not fit in 8 bits encode it with a 32-bit immediate
	// and emit 0x10 prefix and increment the opcode by 0x10.
	X64_F_IMM32JCC = 0x200,
	// If the immediate is exactly 1 instead of emitting an immediate value
	// increment the opcode by 0x10.
	X64_F_IMM1OP10 = 0x400,

} x64_flag;

typedef enum x64_reloc_type {
	X64_RELOC_JCC_REL8,
	X64_RELOC_JCC_REL32,
	X64_RELOC_JMP_REL8,
	X64_RELOC_JMP_REL32,
} x64_reloc_type;

enum {
	X64_NUM_RELOC_TYPES = 4,
};

extern const uint8_t x64_reloc_type_size[X64_NUM_RELOC_TYPES];

typedef struct x64_symbol {
	size_t offset;
} x64_symbol;

typedef struct x64_label {
	size_t offset;
	size_t symbol;
} x64_label;

typedef struct x64_reloc {
	size_t offset;
	size_t symbol;
	x64_reloc_type type;
} x64_reloc;

typedef struct x64_assembler {
	uint8_t *code;
	size_t code_length;
	size_t code_cap;

	x64_reloc *relocs;
	size_t num_relocs;
	size_t reloc_cap;

	x64_symbol *symbols;
	size_t num_symbols;
	size_t symbol_cap;

	x64_label *labels;
	size_t num_labels;
	size_t label_cap;

	bool error;
} x64_assembler;

bool x64_reserve_code(x64_assembler *a, size_t min_length);

// Emit a single x64 instruction.
//   op:    Opcode with prefixes eg. 0xf20f58 for `addsd`.
//   flags: Encoding flags, see `x64_flag` for values
//   reg:   REG field of the MOD/RM byte, either register or opcode extension
//   rm:    Packed memory information, either `x64_reg` or eg. `x64_mem()`
//   imm:   Immediate value, width specified by `X64_IMMn` flags
// Returns the offset of the _end_ of the instruction
static size_t x64_emit(x64_assembler *a, uint64_t op, uint32_t flags, uint32_t reg, uint64_t rm, int64_t imm)
{
	if (a->code_cap - a->code_length < 16) {
		if (!x64_reserve_code(a, 128)) return SIZE_MAX;
	}

	uint8_t *p = a->code + a->code_length;
	if (op >> 24u) {
		if ((op >> 56u) & 0xff) *p++ = (uint8_t)(op >> 56u);
		if ((op >> 48u) & 0xff) *p++ = (uint8_t)(op >> 48u);
		if ((op >> 40u) & 0xff) *p++ = (uint8_t)(op >> 40u);
		if ((op >> 32u) & 0xff) *p++ = (uint8_t)(op >> 32u);
		if ((op >> 24u) & 0xff) *p++ = (uint8_t)(op >> 24u);
	}

	uint32_t sib_index = (uint32_t)(rm >> 4u) & 0xf;

	uint32_t imm_sz = ((flags>>4u) & 0xf);
	bool imm8 = (uint64_t)imm + 128 < 256;
	uint32_t op_inc = 0;
	if ((flags & X64_F_IMM32JCC) != 0 && !imm8) {
		op_inc = 0x10;
		imm_sz = 4;
		*p++ = 0x0f;
	} else if ((flags & X64_F_IMM8OP2) != 0 && imm8) {
		op_inc = 0x2;
		imm_sz = 1;
	} else if ((flags & X64_F_IMM1OP10) != 0 && imm == 1) {
		op_inc = 0x10;
		imm_sz = 0;
	}

	// Note that `X64_F_REXW == 0x8`
	if (((flags | rm | reg) & 0x8) != 0) {
		*p++ = (uint8_t)(0x40 | (flags&0x8) | ((reg&0x8)>>1) | ((sib_index&0x8)>>2) | ((rm&0x8)>>3));
	}

	if ((op >> 8u) & 0xffff) {
		if ((op >> 16u) & 0xff) *p++ = (uint8_t)(op >> 16u);
		if ((op >>  8u) & 0xff) *p++ = (uint8_t)(op >>  8u);
	}

	*p++ = (uint8_t)(op + op_inc);

	if (flags & X64_F_REGRM) {
		uint32_t rm_ctrl = (uint32_t)(rm >> 60u);
		uint32_t rm_reg = rm & 0x7;
		if (rm_ctrl) {
			bool rip_relative = false;
			bool rm_sib = (rm_ctrl & 0x2) != 0;
			uint32_t rm_disp_scale = rm_ctrl >> 2u;
			int32_t rm_disp = (int32_t)(rm >> 16u);
			uint32_t sib_scale = ((rm >> 8u) & 0x3);
			if (rm_disp_scale == 0 && rm_disp != 0) {
				// Automatic displacement scale depending on argument
				rm_disp_scale = (rm_disp >= -128 && rm_disp <= 127) ? 1 : 2;
			}
			if (rm_disp_scale == 0x3) {
				// RIP relative
				rm_disp_scale = 2;
				rip_relative = true;
			} else if ((rm_reg&0x7) == 0x4) {
				// [RSP+disp0/8/32] is used for [SIB+disp0/8/32]
				// We can encode [RSP+disp0/8/32] using SIB as [RSP + RSP*1 + disp/0/8/32]
				rm_sib = 1;
				sib_scale = 0;
				sib_index = X64_RSP;
			} else if ((rm_reg&0x7) == 0x5 && rm_disp_scale == 0) {
				// [BP] is used for [RIP+disp32]
				rm_disp_scale = 1;
			}

			if (rip_relative) {
				// RIP+disp32 special case with RM=RBP
				*p++ = (uint8_t)(((reg&0x7)<<3u) | (X64_RBP));
			} else if (rm_sib) {
				// SIB special case with RM=RSP
				*p++ = (uint8_t)((rm_disp_scale<<6u) | ((reg&0x7)<<3u) | (X64_RSP));
				*p++ = (uint8_t)((sib_scale<<6u) | ((sib_index&0x7)<<3u) | (rm_reg));
			} else {
				// Normal REG/RM
				*p++ = (uint8_t)((rm_disp_scale<<6u) | ((reg&0x7)<<3u) | (rm_reg));
			}

			// Optional displacement
			if (rm_disp_scale == 1) {
				*p++ = (uint8_t)rm_disp;
			} else if (rm_disp_scale == 2) { 
				p[0] = (uint8_t)(rm_disp >>  0);
				p[1] = (uint8_t)(rm_disp >>  8);
				p[2] = (uint8_t)(rm_disp >> 16);
				p[3] = (uint8_t)(rm_disp >> 24);
				p += 4;
			}
		} else {
			// Register REG/RM
			*p++ = (uint8_t)((0x3<<6u) | ((reg&0x7)<<3u) | (rm_reg));
		}
	}

	if (imm_sz == 1) {
		if ((flags & X64_F_IMMSZ) == 0) assert(imm >= INT8_MIN && imm <= INT8_MAX);
		*p++ = (uint8_t)imm;
	} else if (imm_sz == 2) {
		if ((flags & X64_F_IMMSZ) == 0) assert(imm >= INT16_MIN && imm <= INT16_MAX);
		p[0] = (uint8_t)(imm >> 0);
		p[1] = (uint8_t)(imm >> 8);
		p += 2;
	} else if (imm_sz == 4) {
		if ((flags & X64_F_IMMSZ) == 0) assert(imm >= INT32_MIN && imm <= INT32_MAX);
		p[0] = (uint8_t)(imm >>  0);
		p[1] = (uint8_t)(imm >>  8);
		p[2] = (uint8_t)(imm >> 16);
		p[3] = (uint8_t)(imm >> 24);
		p += 4;
	} else if (imm_sz == 8) {
		p[0] = (uint8_t)(imm >>  0);
		p[1] = (uint8_t)(imm >>  8);
		p[2] = (uint8_t)(imm >> 16);
		p[3] = (uint8_t)(imm >> 24);
		p[4] = (uint8_t)(imm >> 32);
		p[5] = (uint8_t)(imm >> 40);
		p[6] = (uint8_t)(imm >> 48);
		p[7] = (uint8_t)(imm >> 56);
		p += 8;
	}

	a->code_length = (p - a->code);
	return a->code_length;
}

static uint64_t x64_op_cc(uint64_t op, x64_cc cc) { assert((uint32_t)cc < 0x10); return op + cc; }
static uint64_t x64_op_lowreg(uint64_t op, uint32_t lowreg) { assert(lowreg < 0x8); return op + lowreg; }

size_t x64_new_symbol(x64_assembler *a);
bool x64_set_label(x64_assembler *a, size_t symbol);

bool x64_add_reloc(x64_assembler *a, x64_reloc_type type, size_t offset, size_t symbol);

bool x64_jmp_symbol(x64_assembler *a, size_t symbol);
bool x64_jcc_symbol(x64_assembler *a, x64_cc cc, size_t symbol);

void x64_optimize_relocs(x64_assembler *a);
void x64_patch_relocs(x64_assembler *a);

static uint32_t x64_scale_to_sib(size_t scale)
{
	if (scale == 1) return 0;
	else if (scale == 2) return 1;
	else if (scale == 4) return 2;
	else if (scale == 8) return 3;
	else {
		assert(false);
		return 0;
	}
}

#define x64_mem(reg, disp) (((uint64_t)0x1<<60u) | (reg) | (uint64_t)(uint32_t)(int32_t)(disp)<<16u)
#define x64_sib(scale, index, base, disp) (((uint64_t)0x3<<60u) | x64_scale_to_sib(scale) << 8 | (uint32_t)(index) << 4 | (base) | (uint64_t)(uint32_t)(int32_t)(disp)<<16u)
#define x64_mem_disp8(reg, disp) (((uint64_t)0x5<<60u) | (reg) | (uint64_t)(uint32_t)(int32_t)(disp)<<16u)
#define x64_sib_disp8(scale, index, base, disp) (((uint64_t)0x7<<60u) | (uint32_t)(scale) << 8 | (uint32_t)(index) << 4 | (base) | (uint64_t)(uint32_t)(int32_t)(disp)<<16u)
#define x64_mem_disp32(reg, disp) (((uint64_t)0x9<<60u) | (reg) | (uint64_t)(uint32_t)(int32_t)(disp)<<16u)
#define x64_sib_disp32(scale, index, base, disp) (((uint64_t)0xb<<60u) | (uint32_t)(scale) << 8 | (uint32_t)(index) << 4 | (base) | (uint64_t)(uint32_t)(int32_t)(disp)<<16u)
#define x64_mem_rip(disp) (((uint64_t)0xd<<60u) | (uint64_t)(uint32_t)(int32_t)(disp)<<16u)

#define x64_add8_rm_reg(a, rm, reg) x64_emit((a), 0x00, X64_F_REGRM, (reg), (rm), 0)
#define x64_add8_reg_rm(a, reg, rm) x64_emit((a), 0x02, X64_F_REGRM, (reg), (rm), 0)
#define x64_add16_rm_reg(a, rm, reg) x64_emit((a), 0x66000001, X64_F_REGRM, (reg), (rm), 0)
#define x64_add16_reg_rm(a, reg, rm) x64_emit((a), 0x66000003, X64_F_REGRM, (reg), (rm), 0)
#define x64_add32_rm_reg(a, rm, reg) x64_emit((a), 0x01, X64_F_REGRM, (reg), (rm), 0)
#define x64_add32_reg_rm(a, reg, rm) x64_emit((a), 0x03, X64_F_REGRM, (reg), (rm), 0)
#define x64_add64_rm_reg(a, rm, reg) x64_emit((a), 0x01, X64_F_REXW|X64_F_REGRM, (reg), (rm), 0)
#define x64_add64_reg_rm(a, reg, rm) x64_emit((a), 0x03, X64_F_REXW|X64_F_REGRM, (reg), (rm), 0)
#define x64_or8_rm_reg(a, rm, reg) x64_emit((a), 0x08, X64_F_REGRM, (reg), (rm), 0)
#define x64_or8_reg_rm(a, reg, rm) x64_emit((a), 0x0a, X64_F_REGRM, (reg), (rm), 0)
#define x64_or16_rm_reg(a, rm, reg) x64_emit((a), 0x66000009, X64_F_REGRM, (reg), (rm), 0)
#define x64_or16_reg_rm(a, reg, rm) x64_emit((a), 0x6600000b, X64_F_REGRM, (reg), (rm), 0)
#define x64_or32_rm_reg(a, rm, reg) x64_emit((a), 0x09, X64_F_REGRM, (reg), (rm), 0)
#define x64_or32_reg_rm(a, reg, rm) x64_emit((a), 0x0b, X64_F_REGRM, (reg), (rm), 0)
#define x64_or64_rm_reg(a, rm, reg) x64_emit((a), 0x09, X64_F_REXW|X64_F_REGRM, (reg), (rm), 0)
#define x64_or64_reg_rm(a, reg, rm) x64_emit((a), 0x0b, X64_F_REXW|X64_F_REGRM, (reg), (rm), 0)
#define x64_adc8_rm_reg(a, rm, reg) x64_emit((a), 0x10, X64_F_REGRM, (reg), (rm), 0)
#define x64_adc8_reg_rm(a, reg, rm) x64_emit((a), 0x12, X64_F_REGRM, (reg), (rm), 0)
#define x64_adc16_rm_reg(a, rm, reg) x64_emit((a), 0x66000011, X64_F_REGRM, (reg), (rm), 0)
#define x64_adc16_reg_rm(a, reg, rm) x64_emit((a), 0x66000013, X64_F_REGRM, (reg), (rm), 0)
#define x64_adc32_rm_reg(a, rm, reg) x64_emit((a), 0x11, X64_F_REGRM, (reg), (rm), 0)
#define x64_adc32_reg_rm(a, reg, rm) x64_emit((a), 0x13, X64_F_REGRM, (reg), (rm), 0)
#define x64_adc64_rm_reg(a, rm, reg) x64_emit((a), 0x11, X64_F_REXW|X64_F_REGRM, (reg), (rm), 0)
#define x64_adc64_reg_rm(a, reg, rm) x64_emit((a), 0x13, X64_F_REXW|X64_F_REGRM, (reg), (rm), 0)
#define x64_sbb8_rm_reg(a, rm, reg) x64_emit((a), 0x18, X64_F_REGRM, (reg), (rm), 0)
#define x64_sbb8_reg_rm(a, reg, rm) x64_emit((a), 0x1a, X64_F_REGRM, (reg), (rm), 0)
#define x64_sbb16_rm_reg(a, rm, reg) x64_emit((a), 0x66000019, X64_F_REGRM, (reg), (rm), 0)
#define x64_sbb16_reg_rm(a, reg, rm) x64_emit((a), 0x6600001b, X64_F_REGRM, (reg), (rm), 0)
#define x64_sbb32_rm_reg(a, rm, reg) x64_emit((a), 0x19, X64_F_REGRM, (reg), (rm), 0)
#define x64_sbb32_reg_rm(a, reg, rm) x64_emit((a), 0x1b, X64_F_REGRM, (reg), (rm), 0)
#define x64_sbb64_rm_reg(a, rm, reg) x64_emit((a), 0x19, X64_F_REXW|X64_F_REGRM, (reg), (rm), 0)
#define x64_sbb64_reg_rm(a, reg, rm) x64_emit((a), 0x1b, X64_F_REXW|X64_F_REGRM, (reg), (rm), 0)
#define x64_and8_rm_reg(a, rm, reg) x64_emit((a), 0x20, X64_F_REGRM, (reg), (rm), 0)
#define x64_and8_reg_rm(a, reg, rm) x64_emit((a), 0x22, X64_F_REGRM, (reg), (rm), 0)
#define x64_and16_rm_reg(a, rm, reg) x64_emit((a), 0x66000021, X64_F_REGRM, (reg), (rm), 0)
#define x64_and16_reg_rm(a, reg, rm) x64_emit((a), 0x66000023, X64_F_REGRM, (reg), (rm), 0)
#define x64_and32_rm_reg(a, rm, reg) x64_emit((a), 0x21, X64_F_REGRM, (reg), (rm), 0)
#define x64_and32_reg_rm(a, reg, rm) x64_emit((a), 0x23, X64_F_REGRM, (reg), (rm), 0)
#define x64_and64_rm_reg(a, rm, reg) x64_emit((a), 0x21, X64_F_REXW|X64_F_REGRM, (reg), (rm), 0)
#define x64_and64_reg_rm(a, reg, rm) x64_emit((a), 0x23, X64_F_REXW|X64_F_REGRM, (reg), (rm), 0)
#define x64_sub8_rm_reg(a, rm, reg) x64_emit((a), 0x28, X64_F_REGRM, (reg), (rm), 0)
#define x64_sub8_reg_rm(a, reg, rm) x64_emit((a), 0x2a, X64_F_REGRM, (reg), (rm), 0)
#define x64_sub16_rm_reg(a, rm, reg) x64_emit((a), 0x66000029, X64_F_REGRM, (reg), (rm), 0)
#define x64_sub16_reg_rm(a, reg, rm) x64_emit((a), 0x6600002b, X64_F_REGRM, (reg), (rm), 0)
#define x64_sub32_rm_reg(a, rm, reg) x64_emit((a), 0x29, X64_F_REGRM, (reg), (rm), 0)
#define x64_sub32_reg_rm(a, reg, rm) x64_emit((a), 0x2b, X64_F_REGRM, (reg), (rm), 0)
#define x64_sub64_rm_reg(a, rm, reg) x64_emit((a), 0x29, X64_F_REXW|X64_F_REGRM, (reg), (rm), 0)
#define x64_sub64_reg_rm(a, reg, rm) x64_emit((a), 0x2b, X64_F_REXW|X64_F_REGRM, (reg), (rm), 0)
#define x64_xor8_rm_reg(a, rm, reg) x64_emit((a), 0x30, X64_F_REGRM, (reg), (rm), 0)
#define x64_xor8_reg_rm(a, reg, rm) x64_emit((a), 0x32, X64_F_REGRM, (reg), (rm), 0)
#define x64_xor16_rm_reg(a, rm, reg) x64_emit((a), 0x66000031, X64_F_REGRM, (reg), (rm), 0)
#define x64_xor16_reg_rm(a, reg, rm) x64_emit((a), 0x66000033, X64_F_REGRM, (reg), (rm), 0)
#define x64_xor32_rm_reg(a, rm, reg) x64_emit((a), 0x31, X64_F_REGRM, (reg), (rm), 0)
#define x64_xor32_reg_rm(a, reg, rm) x64_emit((a), 0x33, X64_F_REGRM, (reg), (rm), 0)
#define x64_xor64_rm_reg(a, rm, reg) x64_emit((a), 0x31, X64_F_REXW|X64_F_REGRM, (reg), (rm), 0)
#define x64_xor64_reg_rm(a, reg, rm) x64_emit((a), 0x33, X64_F_REXW|X64_F_REGRM, (reg), (rm), 0)
#define x64_cmp8_rm_reg(a, rm, reg) x64_emit((a), 0x38, X64_F_REGRM, (reg), (rm), 0)
#define x64_cmp8_reg_rm(a, reg, rm) x64_emit((a), 0x3a, X64_F_REGRM, (reg), (rm), 0)
#define x64_cmp16_rm_reg(a, rm, reg) x64_emit((a), 0x66000039, X64_F_REGRM, (reg), (rm), 0)
#define x64_cmp16_reg_rm(a, reg, rm) x64_emit((a), 0x6600003b, X64_F_REGRM, (reg), (rm), 0)
#define x64_cmp32_rm_reg(a, rm, reg) x64_emit((a), 0x39, X64_F_REGRM, (reg), (rm), 0)
#define x64_cmp32_reg_rm(a, reg, rm) x64_emit((a), 0x3b, X64_F_REGRM, (reg), (rm), 0)
#define x64_cmp64_rm_reg(a, rm, reg) x64_emit((a), 0x39, X64_F_REXW|X64_F_REGRM, (reg), (rm), 0)
#define x64_cmp64_reg_rm(a, reg, rm) x64_emit((a), 0x3b, X64_F_REXW|X64_F_REGRM, (reg), (rm), 0)
#define x64_add8_rm_imm(a, rm, imm) x64_emit((a), 0x80, X64_F_IMM8|X64_F_REGRM, 0, (rm), (imm))
#define x64_add8_rm_imm8(a, rm, imm) x64_emit((a), 0x80, X64_F_IMM8|X64_F_IMMSZ|X64_F_REGRM, 0, (rm), (imm))
#define x64_add16_rm_imm(a, rm, imm) x64_emit((a), 0x66000081, X64_F_REGRM|X64_F_IMM16|X64_F_IMM8OP2, 0, (rm), (imm))
#define x64_add16_rm_imm8(a, rm, imm) x64_emit((a), 0x66000083, X64_F_IMM8|X64_F_IMMSZ|X64_F_REGRM, 0, (rm), (imm))
#define x64_add16_rm_imm16(a, rm, imm) x64_emit((a), 0x66000081, X64_F_IMMSZ|X64_F_REGRM|X64_F_IMM16, 0, (rm), (imm))
#define x64_add32_rm_imm(a, rm, imm) x64_emit((a), 0x81, X64_F_REGRM|X64_F_IMM32|X64_F_IMM8OP2, 0, (rm), (imm))
#define x64_add32_rm_imm8(a, rm, imm) x64_emit((a), 0x83, X64_F_IMM8|X64_F_IMMSZ|X64_F_REGRM, 0, (rm), (imm))
#define x64_add32_rm_imm32(a, rm, imm) x64_emit((a), 0x81, X64_F_IMMSZ|X64_F_REGRM|X64_F_IMM32, 0, (rm), (imm))
#define x64_add64_rm_imm(a, rm, imm) x64_emit((a), 0x81, X64_F_REGRM|X64_F_REXW|X64_F_IMM32|X64_F_IMM8OP2, 0, (rm), (imm))
#define x64_add64_rm_imm8(a, rm, imm) x64_emit((a), 0x83, X64_F_IMM8|X64_F_IMMSZ|X64_F_REXW|X64_F_REGRM, 0, (rm), (imm))
#define x64_add64_rm_imm32(a, rm, imm) x64_emit((a), 0x81, X64_F_IMMSZ|X64_F_REGRM|X64_F_REXW|X64_F_IMM32, 0, (rm), (imm))
#define x64_or8_rm_imm(a, rm, imm) x64_emit((a), 0x80, X64_F_IMM8|X64_F_REGRM, 1, (rm), (imm))
#define x64_or8_rm_imm8(a, rm, imm) x64_emit((a), 0x80, X64_F_IMM8|X64_F_IMMSZ|X64_F_REGRM, 1, (rm), (imm))
#define x64_or16_rm_imm(a, rm, imm) x64_emit((a), 0x66000081, X64_F_REGRM|X64_F_IMM16|X64_F_IMM8OP2, 1, (rm), (imm))
#define x64_or16_rm_imm8(a, rm, imm) x64_emit((a), 0x66000083, X64_F_IMM8|X64_F_IMMSZ|X64_F_REGRM, 1, (rm), (imm))
#define x64_or16_rm_imm16(a, rm, imm) x64_emit((a), 0x66000081, X64_F_IMMSZ|X64_F_REGRM|X64_F_IMM16, 1, (rm), (imm))
#define x64_or32_rm_imm(a, rm, imm) x64_emit((a), 0x81, X64_F_REGRM|X64_F_IMM32|X64_F_IMM8OP2, 1, (rm), (imm))
#define x64_or32_rm_imm8(a, rm, imm) x64_emit((a), 0x83, X64_F_IMM8|X64_F_IMMSZ|X64_F_REGRM, 1, (rm), (imm))
#define x64_or32_rm_imm32(a, rm, imm) x64_emit((a), 0x81, X64_F_IMMSZ|X64_F_REGRM|X64_F_IMM32, 1, (rm), (imm))
#define x64_or64_rm_imm(a, rm, imm) x64_emit((a), 0x81, X64_F_REGRM|X64_F_REXW|X64_F_IMM32|X64_F_IMM8OP2, 1, (rm), (imm))
#define x64_or64_rm_imm8(a, rm, imm) x64_emit((a), 0x83, X64_F_IMM8|X64_F_IMMSZ|X64_F_REXW|X64_F_REGRM, 1, (rm), (imm))
#define x64_or64_rm_imm32(a, rm, imm) x64_emit((a), 0x81, X64_F_IMMSZ|X64_F_REGRM|X64_F_REXW|X64_F_IMM32, 1, (rm), (imm))
#define x64_adc8_rm_imm(a, rm, imm) x64_emit((a), 0x80, X64_F_IMM8|X64_F_REGRM, 2, (rm), (imm))
#define x64_adc8_rm_imm8(a, rm, imm) x64_emit((a), 0x80, X64_F_IMM8|X64_F_IMMSZ|X64_F_REGRM, 2, (rm), (imm))
#define x64_adc16_rm_imm(a, rm, imm) x64_emit((a), 0x66000081, X64_F_REGRM|X64_F_IMM16|X64_F_IMM8OP2, 2, (rm), (imm))
#define x64_adc16_rm_imm8(a, rm, imm) x64_emit((a), 0x66000083, X64_F_IMM8|X64_F_IMMSZ|X64_F_REGRM, 2, (rm), (imm))
#define x64_adc16_rm_imm16(a, rm, imm) x64_emit((a), 0x66000081, X64_F_IMMSZ|X64_F_REGRM|X64_F_IMM16, 2, (rm), (imm))
#define x64_adc32_rm_imm(a, rm, imm) x64_emit((a), 0x81, X64_F_REGRM|X64_F_IMM32|X64_F_IMM8OP2, 2, (rm), (imm))
#define x64_adc32_rm_imm8(a, rm, imm) x64_emit((a), 0x83, X64_F_IMM8|X64_F_IMMSZ|X64_F_REGRM, 2, (rm), (imm))
#define x64_adc32_rm_imm32(a, rm, imm) x64_emit((a), 0x81, X64_F_IMMSZ|X64_F_REGRM|X64_F_IMM32, 2, (rm), (imm))
#define x64_adc64_rm_imm(a, rm, imm) x64_emit((a), 0x81, X64_F_REGRM|X64_F_REXW|X64_F_IMM32|X64_F_IMM8OP2, 2, (rm), (imm))
#define x64_adc64_rm_imm8(a, rm, imm) x64_emit((a), 0x83, X64_F_IMM8|X64_F_IMMSZ|X64_F_REXW|X64_F_REGRM, 2, (rm), (imm))
#define x64_adc64_rm_imm32(a, rm, imm) x64_emit((a), 0x81, X64_F_IMMSZ|X64_F_REGRM|X64_F_REXW|X64_F_IMM32, 2, (rm), (imm))
#define x64_sbb8_rm_imm(a, rm, imm) x64_emit((a), 0x80, X64_F_IMM8|X64_F_REGRM, 3, (rm), (imm))
#define x64_sbb8_rm_imm8(a, rm, imm) x64_emit((a), 0x80, X64_F_IMM8|X64_F_IMMSZ|X64_F_REGRM, 3, (rm), (imm))
#define x64_sbb16_rm_imm(a, rm, imm) x64_emit((a), 0x66000081, X64_F_REGRM|X64_F_IMM16|X64_F_IMM8OP2, 3, (rm), (imm))
#define x64_sbb16_rm_imm8(a, rm, imm) x64_emit((a), 0x66000083, X64_F_IMM8|X64_F_IMMSZ|X64_F_REGRM, 3, (rm), (imm))
#define x64_sbb16_rm_imm16(a, rm, imm) x64_emit((a), 0x66000081, X64_F_IMMSZ|X64_F_REGRM|X64_F_IMM16, 3, (rm), (imm))
#define x64_sbb32_rm_imm(a, rm, imm) x64_emit((a), 0x81, X64_F_REGRM|X64_F_IMM32|X64_F_IMM8OP2, 3, (rm), (imm))
#define x64_sbb32_rm_imm8(a, rm, imm) x64_emit((a), 0x83, X64_F_IMM8|X64_F_IMMSZ|X64_F_REGRM, 3, (rm), (imm))
#define x64_sbb32_rm_imm32(a, rm, imm) x64_emit((a), 0x81, X64_F_IMMSZ|X64_F_REGRM|X64_F_IMM32, 3, (rm), (imm))
#define x64_sbb64_rm_imm(a, rm, imm) x64_emit((a), 0x81, X64_F_REGRM|X64_F_REXW|X64_F_IMM32|X64_F_IMM8OP2, 3, (rm), (imm))
#define x64_sbb64_rm_imm8(a, rm, imm) x64_emit((a), 0x83, X64_F_IMM8|X64_F_IMMSZ|X64_F_REXW|X64_F_REGRM, 3, (rm), (imm))
#define x64_sbb64_rm_imm32(a, rm, imm) x64_emit((a), 0x81, X64_F_IMMSZ|X64_F_REGRM|X64_F_REXW|X64_F_IMM32, 3, (rm), (imm))
#define x64_and8_rm_imm(a, rm, imm) x64_emit((a), 0x80, X64_F_IMM8|X64_F_REGRM, 4, (rm), (imm))
#define x64_and8_rm_imm8(a, rm, imm) x64_emit((a), 0x80, X64_F_IMM8|X64_F_IMMSZ|X64_F_REGRM, 4, (rm), (imm))
#define x64_and16_rm_imm(a, rm, imm) x64_emit((a), 0x66000081, X64_F_REGRM|X64_F_IMM16|X64_F_IMM8OP2, 4, (rm), (imm))
#define x64_and16_rm_imm8(a, rm, imm) x64_emit((a), 0x66000083, X64_F_IMM8|X64_F_IMMSZ|X64_F_REGRM, 4, (rm), (imm))
#define x64_and16_rm_imm16(a, rm, imm) x64_emit((a), 0x66000081, X64_F_IMMSZ|X64_F_REGRM|X64_F_IMM16, 4, (rm), (imm))
#define x64_and32_rm_imm(a, rm, imm) x64_emit((a), 0x81, X64_F_REGRM|X64_F_IMM32|X64_F_IMM8OP2, 4, (rm), (imm))
#define x64_and32_rm_imm8(a, rm, imm) x64_emit((a), 0x83, X64_F_IMM8|X64_F_IMMSZ|X64_F_REGRM, 4, (rm), (imm))
#define x64_and32_rm_imm32(a, rm, imm) x64_emit((a), 0x81, X64_F_IMMSZ|X64_F_REGRM|X64_F_IMM32, 4, (rm), (imm))
#define x64_and64_rm_imm(a, rm, imm) x64_emit((a), 0x81, X64_F_REGRM|X64_F_REXW|X64_F_IMM32|X64_F_IMM8OP2, 4, (rm), (imm))
#define x64_and64_rm_imm8(a, rm, imm) x64_emit((a), 0x83, X64_F_IMM8|X64_F_IMMSZ|X64_F_REXW|X64_F_REGRM, 4, (rm), (imm))
#define x64_and64_rm_imm32(a, rm, imm) x64_emit((a), 0x81, X64_F_IMMSZ|X64_F_REGRM|X64_F_REXW|X64_F_IMM32, 4, (rm), (imm))
#define x64_sub8_rm_imm(a, rm, imm) x64_emit((a), 0x80, X64_F_IMM8|X64_F_REGRM, 5, (rm), (imm))
#define x64_sub8_rm_imm8(a, rm, imm) x64_emit((a), 0x80, X64_F_IMM8|X64_F_IMMSZ|X64_F_REGRM, 5, (rm), (imm))
#define x64_sub16_rm_imm(a, rm, imm) x64_emit((a), 0x66000081, X64_F_REGRM|X64_F_IMM16|X64_F_IMM8OP2, 5, (rm), (imm))
#define x64_sub16_rm_imm8(a, rm, imm) x64_emit((a), 0x66000083, X64_F_IMM8|X64_F_IMMSZ|X64_F_REGRM, 5, (rm), (imm))
#define x64_sub16_rm_imm16(a, rm, imm) x64_emit((a), 0x66000081, X64_F_IMMSZ|X64_F_REGRM|X64_F_IMM16, 5, (rm), (imm))
#define x64_sub32_rm_imm(a, rm, imm) x64_emit((a), 0x81, X64_F_REGRM|X64_F_IMM32|X64_F_IMM8OP2, 5, (rm), (imm))
#define x64_sub32_rm_imm8(a, rm, imm) x64_emit((a), 0x83, X64_F_IMM8|X64_F_IMMSZ|X64_F_REGRM, 5, (rm), (imm))
#define x64_sub32_rm_imm32(a, rm, imm) x64_emit((a), 0x81, X64_F_IMMSZ|X64_F_REGRM|X64_F_IMM32, 5, (rm), (imm))
#define x64_sub64_rm_imm(a, rm, imm) x64_emit((a), 0x81, X64_F_REGRM|X64_F_REXW|X64_F_IMM32|X64_F_IMM8OP2, 5, (rm), (imm))
#define x64_sub64_rm_imm8(a, rm, imm) x64_emit((a), 0x83, X64_F_IMM8|X64_F_IMMSZ|X64_F_REXW|X64_F_REGRM, 5, (rm), (imm))
#define x64_sub64_rm_imm32(a, rm, imm) x64_emit((a), 0x81, X64_F_IMMSZ|X64_F_REGRM|X64_F_REXW|X64_F_IMM32, 5, (rm), (imm))
#define x64_xor8_rm_imm(a, rm, imm) x64_emit((a), 0x80, X64_F_IMM8|X64_F_REGRM, 6, (rm), (imm))
#define x64_xor8_rm_imm8(a, rm, imm) x64_emit((a), 0x80, X64_F_IMM8|X64_F_IMMSZ|X64_F_REGRM, 6, (rm), (imm))
#define x64_xor16_rm_imm(a, rm, imm) x64_emit((a), 0x66000081, X64_F_REGRM|X64_F_IMM16|X64_F_IMM8OP2, 6, (rm), (imm))
#define x64_xor16_rm_imm8(a, rm, imm) x64_emit((a), 0x66000083, X64_F_IMM8|X64_F_IMMSZ|X64_F_REGRM, 6, (rm), (imm))
#define x64_xor16_rm_imm16(a, rm, imm) x64_emit((a), 0x66000081, X64_F_IMMSZ|X64_F_REGRM|X64_F_IMM16, 6, (rm), (imm))
#define x64_xor32_rm_imm(a, rm, imm) x64_emit((a), 0x81, X64_F_REGRM|X64_F_IMM32|X64_F_IMM8OP2, 6, (rm), (imm))
#define x64_xor32_rm_imm8(a, rm, imm) x64_emit((a), 0x83, X64_F_IMM8|X64_F_IMMSZ|X64_F_REGRM, 6, (rm), (imm))
#define x64_xor32_rm_imm32(a, rm, imm) x64_emit((a), 0x81, X64_F_IMMSZ|X64_F_REGRM|X64_F_IMM32, 6, (rm), (imm))
#define x64_xor64_rm_imm(a, rm, imm) x64_emit((a), 0x81, X64_F_REGRM|X64_F_REXW|X64_F_IMM32|X64_F_IMM8OP2, 6, (rm), (imm))
#define x64_xor64_rm_imm8(a, rm, imm) x64_emit((a), 0x83, X64_F_IMM8|X64_F_IMMSZ|X64_F_REXW|X64_F_REGRM, 6, (rm), (imm))
#define x64_xor64_rm_imm32(a, rm, imm) x64_emit((a), 0x81, X64_F_IMMSZ|X64_F_REGRM|X64_F_REXW|X64_F_IMM32, 6, (rm), (imm))
#define x64_cmp8_rm_imm(a, rm, imm) x64_emit((a), 0x80, X64_F_IMM8|X64_F_REGRM, 7, (rm), (imm))
#define x64_cmp8_rm_imm8(a, rm, imm) x64_emit((a), 0x80, X64_F_IMM8|X64_F_IMMSZ|X64_F_REGRM, 7, (rm), (imm))
#define x64_cmp16_rm_imm(a, rm, imm) x64_emit((a), 0x66000081, X64_F_REGRM|X64_F_IMM16|X64_F_IMM8OP2, 7, (rm), (imm))
#define x64_cmp16_rm_imm8(a, rm, imm) x64_emit((a), 0x66000083, X64_F_IMM8|X64_F_IMMSZ|X64_F_REGRM, 7, (rm), (imm))
#define x64_cmp16_rm_imm16(a, rm, imm) x64_emit((a), 0x66000081, X64_F_IMMSZ|X64_F_REGRM|X64_F_IMM16, 7, (rm), (imm))
#define x64_cmp32_rm_imm(a, rm, imm) x64_emit((a), 0x81, X64_F_REGRM|X64_F_IMM32|X64_F_IMM8OP2, 7, (rm), (imm))
#define x64_cmp32_rm_imm8(a, rm, imm) x64_emit((a), 0x83, X64_F_IMM8|X64_F_IMMSZ|X64_F_REGRM, 7, (rm), (imm))
#define x64_cmp32_rm_imm32(a, rm, imm) x64_emit((a), 0x81, X64_F_IMMSZ|X64_F_REGRM|X64_F_IMM32, 7, (rm), (imm))
#define x64_cmp64_rm_imm(a, rm, imm) x64_emit((a), 0x81, X64_F_REGRM|X64_F_REXW|X64_F_IMM32|X64_F_IMM8OP2, 7, (rm), (imm))
#define x64_cmp64_rm_imm8(a, rm, imm) x64_emit((a), 0x83, X64_F_IMM8|X64_F_IMMSZ|X64_F_REXW|X64_F_REGRM, 7, (rm), (imm))
#define x64_cmp64_rm_imm32(a, rm, imm) x64_emit((a), 0x81, X64_F_IMMSZ|X64_F_REGRM|X64_F_REXW|X64_F_IMM32, 7, (rm), (imm))
#define x64_test8_reg_rm(a, reg, rm) x64_emit((a), 0x84, X64_F_REGRM, (reg), (rm), 0)
#define x64_test16_reg_rm(a, reg, rm) x64_emit((a), 0x66000085, X64_F_REGRM, (reg), (rm), 0)
#define x64_test32_reg_rm(a, reg, rm) x64_emit((a), 0x85, X64_F_REGRM, (reg), (rm), 0)
#define x64_test64_reg_rm(a, reg, rm) x64_emit((a), 0x85, X64_F_REXW|X64_F_REGRM, (reg), (rm), 0)
#define x64_xchg8_reg_rm(a, reg, rm) x64_emit((a), 0x86, X64_F_REGRM, (reg), (rm), 0)
#define x64_xchg16_reg_rm(a, reg, rm) x64_emit((a), 0x66000087, X64_F_REGRM, (reg), (rm), 0)
#define x64_xchg32_reg_rm(a, reg, rm) x64_emit((a), 0x87, X64_F_REGRM, (reg), (rm), 0)
#define x64_xchg64_reg_rm(a, reg, rm) x64_emit((a), 0x87, X64_F_REXW|X64_F_REGRM, (reg), (rm), 0)
#define x64_lea16_reg_rm(a, reg, rm) x64_emit((a), 0x6600008d, X64_F_REGRM, (reg), (rm), 0)
#define x64_lea32_reg_rm(a, reg, rm) x64_emit((a), 0x8d, X64_F_REGRM, (reg), (rm), 0)
#define x64_lea64_reg_rm(a, reg, rm) x64_emit((a), 0x8d, X64_F_REXW|X64_F_REGRM, (reg), (rm), 0)
#define x64_imul16_reg_rm(a, reg, rm) x64_emit((a), 0x66000faf, X64_F_REGRM, (reg), (rm), 0)
#define x64_imul32_reg_rm(a, reg, rm) x64_emit((a), 0x0faf, X64_F_REGRM, (reg), (rm), 0)
#define x64_imul64_reg_rm(a, reg, rm) x64_emit((a), 0x0faf, X64_F_REXW|X64_F_REGRM, (reg), (rm), 0)
#define x64_imul16_reg_rm_imm(a, reg, rm, imm) x64_emit((a), 0x66000069, X64_F_REGRM|X64_F_IMM16, (reg), (rm), (imm))
#define x64_imul16_reg_rm_imm16(a, reg, rm, imm) x64_emit((a), 0x66000069, X64_F_IMMSZ|X64_F_REGRM|X64_F_IMM16, (reg), (rm), (imm))
#define x64_imul32_reg_rm_imm(a, reg, rm, imm) x64_emit((a), 0x69, X64_F_REGRM|X64_F_IMM32, (reg), (rm), (imm))
#define x64_imul32_reg_rm_imm32(a, reg, rm, imm) x64_emit((a), 0x69, X64_F_IMMSZ|X64_F_REGRM|X64_F_IMM32, (reg), (rm), (imm))
#define x64_imul64_reg_rm_imm(a, reg, rm, imm) x64_emit((a), 0x69, X64_F_REGRM|X64_F_REXW|X64_F_IMM32, (reg), (rm), (imm))
#define x64_imul64_reg_rm_imm32(a, reg, rm, imm) x64_emit((a), 0x69, X64_F_IMMSZ|X64_F_REGRM|X64_F_REXW|X64_F_IMM32, (reg), (rm), (imm))
#define x64_jmp_rel(a, imm) x64_emit((a), 0xe9, X64_F_IMM32|X64_F_IMM8OP2, 0, 0, (imm))
#define x64_jmp_rel8(a, imm) x64_emit((a), 0xeb, X64_F_IMM8|X64_F_IMMSZ, 0, 0, (imm))
#define x64_jmp_rel32(a, imm) x64_emit((a), 0xe9, X64_F_IMMSZ|X64_F_IMM32, 0, 0, (imm))
#define x64_jcc_rel(a, cc, imm) x64_emit((a), x64_op_cc(0x70, (cc)), X64_F_IMM8|X64_F_IMM32JCC, 0, 0, (imm))
#define x64_jcc_rel8(a, cc, imm) x64_emit((a), x64_op_cc(0x70, (cc)), X64_F_IMM8|X64_F_IMMSZ, 0, 0, (imm))
#define x64_jcc_rel32(a, cc, imm) x64_emit((a), x64_op_cc(0x0f80, (cc)), X64_F_IMMSZ|X64_F_IMM32, 0, 0, (imm))
#define x64_setcc8_rm(a, cc, rm) x64_emit((a), x64_op_cc(0x0f90, (cc)), X64_F_REGRM, 0, (rm), 0)
#define x64_cmovcc16_reg_rm(a, cc, reg, rm) x64_emit((a), x64_op_cc(0x66000f40, (cc)), X64_F_REGRM, (reg), (rm), 0)
#define x64_cmovcc32_reg_rm(a, cc, reg, rm) x64_emit((a), x64_op_cc(0x0f40, (cc)), X64_F_REGRM, (reg), (rm), 0)
#define x64_cmovcc64_reg_rm(a, cc, reg, rm) x64_emit((a), x64_op_cc(0x0f40, (cc)), X64_F_REXW|X64_F_REGRM, (reg), (rm), 0)
#define x64_mov8_rm_reg(a, rm, reg) x64_emit((a), 0x88, X64_F_REGRM, (reg), (rm), 0)
#define x64_mov8_reg_rm(a, reg, rm) x64_emit((a), 0x8a, X64_F_REGRM, (reg), (rm), 0)
#define x64_mov16_rm_reg(a, rm, reg) x64_emit((a), 0x66000089, X64_F_REGRM, (reg), (rm), 0)
#define x64_mov16_reg_rm(a, reg, rm) x64_emit((a), 0x6600008b, X64_F_REGRM, (reg), (rm), 0)
#define x64_mov32_rm_reg(a, rm, reg) x64_emit((a), 0x89, X64_F_REGRM, (reg), (rm), 0)
#define x64_mov32_reg_rm(a, reg, rm) x64_emit((a), 0x8b, X64_F_REGRM, (reg), (rm), 0)
#define x64_mov64_rm_reg(a, rm, reg) x64_emit((a), 0x89, X64_F_REXW|X64_F_REGRM, (reg), (rm), 0)
#define x64_mov64_reg_rm(a, reg, rm) x64_emit((a), 0x8b, X64_F_REXW|X64_F_REGRM, (reg), (rm), 0)
#define x64_mov8_lowreg_imm(a, lowreg, imm) x64_emit((a), x64_op_lowreg(0xb0, (lowreg)), X64_F_IMM8, 0, 0, (imm))
#define x64_mov8_lowreg_imm8(a, lowreg, imm) x64_emit((a), x64_op_lowreg(0xb0, (lowreg)), X64_F_IMM8|X64_F_IMMSZ, 0, 0, (imm))
#define x64_mov16_lowreg_imm(a, lowreg, imm) x64_emit((a), x64_op_lowreg(0x660000b8, (lowreg)), X64_F_IMM16, 0, 0, (imm))
#define x64_mov16_lowreg_imm16(a, lowreg, imm) x64_emit((a), x64_op_lowreg(0x660000b8, (lowreg)), X64_F_IMMSZ|X64_F_IMM16, 0, 0, (imm))
#define x64_mov32_lowreg_imm(a, lowreg, imm) x64_emit((a), x64_op_lowreg(0xb8, (lowreg)), X64_F_IMM32, 0, 0, (imm))
#define x64_mov32_lowreg_imm32(a, lowreg, imm) x64_emit((a), x64_op_lowreg(0xb8, (lowreg)), X64_F_IMMSZ|X64_F_IMM32, 0, 0, (imm))
#define x64_mov64_lowreg_imm(a, lowreg, imm) x64_emit((a), x64_op_lowreg(0xb8, (lowreg)), X64_F_REXW|X64_F_IMM64, 0, 0, (imm))
#define x64_mov64_lowreg_imm64(a, lowreg, imm) x64_emit((a), x64_op_lowreg(0xb8, (lowreg)), X64_F_IMMSZ|X64_F_REXW|X64_F_IMM64, 0, 0, (imm))
#define x64_mov8_rm_imm(a, rm, imm) x64_emit((a), 0xc6, X64_F_IMM8|X64_F_REGRM, 0, (rm), (imm))
#define x64_mov8_rm_imm8(a, rm, imm) x64_emit((a), 0xc6, X64_F_IMM8|X64_F_IMMSZ|X64_F_REGRM, 0, (rm), (imm))
#define x64_mov16_rm_imm(a, rm, imm) x64_emit((a), 0x660000c7, X64_F_REGRM|X64_F_IMM16, 0, (rm), (imm))
#define x64_mov16_rm_imm16(a, rm, imm) x64_emit((a), 0x660000c7, X64_F_IMMSZ|X64_F_REGRM|X64_F_IMM16, 0, (rm), (imm))
#define x64_mov32_rm_imm(a, rm, imm) x64_emit((a), 0xc7, X64_F_REGRM|X64_F_IMM32, 0, (rm), (imm))
#define x64_mov32_rm_imm32(a, rm, imm) x64_emit((a), 0xc7, X64_F_IMMSZ|X64_F_REGRM|X64_F_IMM32, 0, (rm), (imm))
#define x64_mov64_rm_imm(a, rm, imm) x64_emit((a), 0xc7, X64_F_REGRM|X64_F_REXW|X64_F_IMM32, 0, (rm), (imm))
#define x64_mov64_rm_imm32(a, rm, imm) x64_emit((a), 0xc7, X64_F_IMMSZ|X64_F_REGRM|X64_F_REXW|X64_F_IMM32, 0, (rm), (imm))
#define x64_rol8_rm_imm(a, rm, imm) x64_emit((a), 0xc0, X64_F_IMM8|X64_F_IMM1OP10|X64_F_REGRM, 0, (rm), (imm))
#define x64_rol8_rm_imm8(a, rm, imm) x64_emit((a), 0xc0, X64_F_IMM8|X64_F_IMMSZ|X64_F_REGRM, 0, (rm), (imm))
#define x64_rol16_rm_imm(a, rm, imm) x64_emit((a), 0x660000c1, X64_F_IMM8|X64_F_IMM1OP10|X64_F_REGRM, 0, (rm), (imm))
#define x64_rol16_rm_imm8(a, rm, imm) x64_emit((a), 0x660000c1, X64_F_IMM8|X64_F_IMMSZ|X64_F_REGRM, 0, (rm), (imm))
#define x64_rol32_rm_imm(a, rm, imm) x64_emit((a), 0xc1, X64_F_IMM8|X64_F_IMM1OP10|X64_F_REGRM, 0, (rm), (imm))
#define x64_rol32_rm_imm8(a, rm, imm) x64_emit((a), 0xc1, X64_F_IMM8|X64_F_IMMSZ|X64_F_REGRM, 0, (rm), (imm))
#define x64_rol64_rm_imm(a, rm, imm) x64_emit((a), 0xc1, X64_F_IMM8|X64_F_REXW|X64_F_IMM1OP10|X64_F_REGRM, 0, (rm), (imm))
#define x64_rol64_rm_imm8(a, rm, imm) x64_emit((a), 0xc1, X64_F_IMM8|X64_F_IMMSZ|X64_F_REXW|X64_F_REGRM, 0, (rm), (imm))
#define x64_ror8_rm_imm(a, rm, imm) x64_emit((a), 0xc0, X64_F_IMM8|X64_F_IMM1OP10|X64_F_REGRM, 1, (rm), (imm))
#define x64_ror8_rm_imm8(a, rm, imm) x64_emit((a), 0xc0, X64_F_IMM8|X64_F_IMMSZ|X64_F_REGRM, 1, (rm), (imm))
#define x64_ror16_rm_imm(a, rm, imm) x64_emit((a), 0x660000c1, X64_F_IMM8|X64_F_IMM1OP10|X64_F_REGRM, 1, (rm), (imm))
#define x64_ror16_rm_imm8(a, rm, imm) x64_emit((a), 0x660000c1, X64_F_IMM8|X64_F_IMMSZ|X64_F_REGRM, 1, (rm), (imm))
#define x64_ror32_rm_imm(a, rm, imm) x64_emit((a), 0xc1, X64_F_IMM8|X64_F_IMM1OP10|X64_F_REGRM, 1, (rm), (imm))
#define x64_ror32_rm_imm8(a, rm, imm) x64_emit((a), 0xc1, X64_F_IMM8|X64_F_IMMSZ|X64_F_REGRM, 1, (rm), (imm))
#define x64_ror64_rm_imm(a, rm, imm) x64_emit((a), 0xc1, X64_F_IMM8|X64_F_REXW|X64_F_IMM1OP10|X64_F_REGRM, 1, (rm), (imm))
#define x64_ror64_rm_imm8(a, rm, imm) x64_emit((a), 0xc1, X64_F_IMM8|X64_F_IMMSZ|X64_F_REXW|X64_F_REGRM, 1, (rm), (imm))
#define x64_rcl8_rm_imm(a, rm, imm) x64_emit((a), 0xc0, X64_F_IMM8|X64_F_IMM1OP10|X64_F_REGRM, 2, (rm), (imm))
#define x64_rcl8_rm_imm8(a, rm, imm) x64_emit((a), 0xc0, X64_F_IMM8|X64_F_IMMSZ|X64_F_REGRM, 2, (rm), (imm))
#define x64_rcl16_rm_imm(a, rm, imm) x64_emit((a), 0x660000c1, X64_F_IMM8|X64_F_IMM1OP10|X64_F_REGRM, 2, (rm), (imm))
#define x64_rcl16_rm_imm8(a, rm, imm) x64_emit((a), 0x660000c1, X64_F_IMM8|X64_F_IMMSZ|X64_F_REGRM, 2, (rm), (imm))
#define x64_rcl32_rm_imm(a, rm, imm) x64_emit((a), 0xc1, X64_F_IMM8|X64_F_IMM1OP10|X64_F_REGRM, 2, (rm), (imm))
#define x64_rcl32_rm_imm8(a, rm, imm) x64_emit((a), 0xc1, X64_F_IMM8|X64_F_IMMSZ|X64_F_REGRM, 2, (rm), (imm))
#define x64_rcl64_rm_imm(a, rm, imm) x64_emit((a), 0xc1, X64_F_IMM8|X64_F_REXW|X64_F_IMM1OP10|X64_F_REGRM, 2, (rm), (imm))
#define x64_rcl64_rm_imm8(a, rm, imm) x64_emit((a), 0xc1, X64_F_IMM8|X64_F_IMMSZ|X64_F_REXW|X64_F_REGRM, 2, (rm), (imm))
#define x64_rcr8_rm_imm(a, rm, imm) x64_emit((a), 0xc0, X64_F_IMM8|X64_F_IMM1OP10|X64_F_REGRM, 3, (rm), (imm))
#define x64_rcr8_rm_imm8(a, rm, imm) x64_emit((a), 0xc0, X64_F_IMM8|X64_F_IMMSZ|X64_F_REGRM, 3, (rm), (imm))
#define x64_rcr16_rm_imm(a, rm, imm) x64_emit((a), 0x660000c1, X64_F_IMM8|X64_F_IMM1OP10|X64_F_REGRM, 3, (rm), (imm))
#define x64_rcr16_rm_imm8(a, rm, imm) x64_emit((a), 0x660000c1, X64_F_IMM8|X64_F_IMMSZ|X64_F_REGRM, 3, (rm), (imm))
#define x64_rcr32_rm_imm(a, rm, imm) x64_emit((a), 0xc1, X64_F_IMM8|X64_F_IMM1OP10|X64_F_REGRM, 3, (rm), (imm))
#define x64_rcr32_rm_imm8(a, rm, imm) x64_emit((a), 0xc1, X64_F_IMM8|X64_F_IMMSZ|X64_F_REGRM, 3, (rm), (imm))
#define x64_rcr64_rm_imm(a, rm, imm) x64_emit((a), 0xc1, X64_F_IMM8|X64_F_REXW|X64_F_IMM1OP10|X64_F_REGRM, 3, (rm), (imm))
#define x64_rcr64_rm_imm8(a, rm, imm) x64_emit((a), 0xc1, X64_F_IMM8|X64_F_IMMSZ|X64_F_REXW|X64_F_REGRM, 3, (rm), (imm))
#define x64_shl8_rm_imm(a, rm, imm) x64_emit((a), 0xc0, X64_F_IMM8|X64_F_IMM1OP10|X64_F_REGRM, 4, (rm), (imm))
#define x64_shl8_rm_imm8(a, rm, imm) x64_emit((a), 0xc0, X64_F_IMM8|X64_F_IMMSZ|X64_F_REGRM, 4, (rm), (imm))
#define x64_shl16_rm_imm(a, rm, imm) x64_emit((a), 0x660000c1, X64_F_IMM8|X64_F_IMM1OP10|X64_F_REGRM, 4, (rm), (imm))
#define x64_shl16_rm_imm8(a, rm, imm) x64_emit((a), 0x660000c1, X64_F_IMM8|X64_F_IMMSZ|X64_F_REGRM, 4, (rm), (imm))
#define x64_shl32_rm_imm(a, rm, imm) x64_emit((a), 0xc1, X64_F_IMM8|X64_F_IMM1OP10|X64_F_REGRM, 4, (rm), (imm))
#define x64_shl32_rm_imm8(a, rm, imm) x64_emit((a), 0xc1, X64_F_IMM8|X64_F_IMMSZ|X64_F_REGRM, 4, (rm), (imm))
#define x64_shl64_rm_imm(a, rm, imm) x64_emit((a), 0xc1, X64_F_IMM8|X64_F_REXW|X64_F_IMM1OP10|X64_F_REGRM, 4, (rm), (imm))
#define x64_shl64_rm_imm8(a, rm, imm) x64_emit((a), 0xc1, X64_F_IMM8|X64_F_IMMSZ|X64_F_REXW|X64_F_REGRM, 4, (rm), (imm))
#define x64_shr8_rm_imm(a, rm, imm) x64_emit((a), 0xc0, X64_F_IMM8|X64_F_IMM1OP10|X64_F_REGRM, 5, (rm), (imm))
#define x64_shr8_rm_imm8(a, rm, imm) x64_emit((a), 0xc0, X64_F_IMM8|X64_F_IMMSZ|X64_F_REGRM, 5, (rm), (imm))
#define x64_shr16_rm_imm(a, rm, imm) x64_emit((a), 0x660000c1, X64_F_IMM8|X64_F_IMM1OP10|X64_F_REGRM, 5, (rm), (imm))
#define x64_shr16_rm_imm8(a, rm, imm) x64_emit((a), 0x660000c1, X64_F_IMM8|X64_F_IMMSZ|X64_F_REGRM, 5, (rm), (imm))
#define x64_shr32_rm_imm(a, rm, imm) x64_emit((a), 0xc1, X64_F_IMM8|X64_F_IMM1OP10|X64_F_REGRM, 5, (rm), (imm))
#define x64_shr32_rm_imm8(a, rm, imm) x64_emit((a), 0xc1, X64_F_IMM8|X64_F_IMMSZ|X64_F_REGRM, 5, (rm), (imm))
#define x64_shr64_rm_imm(a, rm, imm) x64_emit((a), 0xc1, X64_F_IMM8|X64_F_REXW|X64_F_IMM1OP10|X64_F_REGRM, 5, (rm), (imm))
#define x64_shr64_rm_imm8(a, rm, imm) x64_emit((a), 0xc1, X64_F_IMM8|X64_F_IMMSZ|X64_F_REXW|X64_F_REGRM, 5, (rm), (imm))
#define x64_sal8_rm_imm(a, rm, imm) x64_emit((a), 0xc0, X64_F_IMM8|X64_F_IMM1OP10|X64_F_REGRM, 6, (rm), (imm))
#define x64_sal8_rm_imm8(a, rm, imm) x64_emit((a), 0xc0, X64_F_IMM8|X64_F_IMMSZ|X64_F_REGRM, 6, (rm), (imm))
#define x64_sal16_rm_imm(a, rm, imm) x64_emit((a), 0x660000c1, X64_F_IMM8|X64_F_IMM1OP10|X64_F_REGRM, 6, (rm), (imm))
#define x64_sal16_rm_imm8(a, rm, imm) x64_emit((a), 0x660000c1, X64_F_IMM8|X64_F_IMMSZ|X64_F_REGRM, 6, (rm), (imm))
#define x64_sal32_rm_imm(a, rm, imm) x64_emit((a), 0xc1, X64_F_IMM8|X64_F_IMM1OP10|X64_F_REGRM, 6, (rm), (imm))
#define x64_sal32_rm_imm8(a, rm, imm) x64_emit((a), 0xc1, X64_F_IMM8|X64_F_IMMSZ|X64_F_REGRM, 6, (rm), (imm))
#define x64_sal64_rm_imm(a, rm, imm) x64_emit((a), 0xc1, X64_F_IMM8|X64_F_REXW|X64_F_IMM1OP10|X64_F_REGRM, 6, (rm), (imm))
#define x64_sal64_rm_imm8(a, rm, imm) x64_emit((a), 0xc1, X64_F_IMM8|X64_F_IMMSZ|X64_F_REXW|X64_F_REGRM, 6, (rm), (imm))
#define x64_sar8_rm_imm(a, rm, imm) x64_emit((a), 0xc0, X64_F_IMM8|X64_F_IMM1OP10|X64_F_REGRM, 7, (rm), (imm))
#define x64_sar8_rm_imm8(a, rm, imm) x64_emit((a), 0xc0, X64_F_IMM8|X64_F_IMMSZ|X64_F_REGRM, 7, (rm), (imm))
#define x64_sar16_rm_imm(a, rm, imm) x64_emit((a), 0x660000c1, X64_F_IMM8|X64_F_IMM1OP10|X64_F_REGRM, 7, (rm), (imm))
#define x64_sar16_rm_imm8(a, rm, imm) x64_emit((a), 0x660000c1, X64_F_IMM8|X64_F_IMMSZ|X64_F_REGRM, 7, (rm), (imm))
#define x64_sar32_rm_imm(a, rm, imm) x64_emit((a), 0xc1, X64_F_IMM8|X64_F_IMM1OP10|X64_F_REGRM, 7, (rm), (imm))
#define x64_sar32_rm_imm8(a, rm, imm) x64_emit((a), 0xc1, X64_F_IMM8|X64_F_IMMSZ|X64_F_REGRM, 7, (rm), (imm))
#define x64_sar64_rm_imm(a, rm, imm) x64_emit((a), 0xc1, X64_F_IMM8|X64_F_REXW|X64_F_IMM1OP10|X64_F_REGRM, 7, (rm), (imm))
#define x64_sar64_rm_imm8(a, rm, imm) x64_emit((a), 0xc1, X64_F_IMM8|X64_F_IMMSZ|X64_F_REXW|X64_F_REGRM, 7, (rm), (imm))
#define x64_rol8_rm_cl(a, rm) x64_emit((a), 0xd2, X64_F_REGRM, 0, (rm), 0)
#define x64_rol16_rm_cl(a, rm) x64_emit((a), 0x660000d3, X64_F_REGRM, 0, (rm), 0)
#define x64_rol32_rm_cl(a, rm) x64_emit((a), 0xd3, X64_F_REGRM, 0, (rm), 0)
#define x64_rol64_rm_cl(a, rm) x64_emit((a), 0xd3, X64_F_REXW|X64_F_REGRM, 0, (rm), 0)
#define x64_ror8_rm_cl(a, rm) x64_emit((a), 0xd2, X64_F_REGRM, 1, (rm), 0)
#define x64_ror16_rm_cl(a, rm) x64_emit((a), 0x660000d3, X64_F_REGRM, 1, (rm), 0)
#define x64_ror32_rm_cl(a, rm) x64_emit((a), 0xd3, X64_F_REGRM, 1, (rm), 0)
#define x64_ror64_rm_cl(a, rm) x64_emit((a), 0xd3, X64_F_REXW|X64_F_REGRM, 1, (rm), 0)
#define x64_rcl8_rm_cl(a, rm) x64_emit((a), 0xd2, X64_F_REGRM, 2, (rm), 0)
#define x64_rcl16_rm_cl(a, rm) x64_emit((a), 0x660000d3, X64_F_REGRM, 2, (rm), 0)
#define x64_rcl32_rm_cl(a, rm) x64_emit((a), 0xd3, X64_F_REGRM, 2, (rm), 0)
#define x64_rcl64_rm_cl(a, rm) x64_emit((a), 0xd3, X64_F_REXW|X64_F_REGRM, 2, (rm), 0)
#define x64_rcr8_rm_cl(a, rm) x64_emit((a), 0xd2, X64_F_REGRM, 3, (rm), 0)
#define x64_rcr16_rm_cl(a, rm) x64_emit((a), 0x660000d3, X64_F_REGRM, 3, (rm), 0)
#define x64_rcr32_rm_cl(a, rm) x64_emit((a), 0xd3, X64_F_REGRM, 3, (rm), 0)
#define x64_rcr64_rm_cl(a, rm) x64_emit((a), 0xd3, X64_F_REXW|X64_F_REGRM, 3, (rm), 0)
#define x64_shl8_rm_cl(a, rm) x64_emit((a), 0xd2, X64_F_REGRM, 4, (rm), 0)
#define x64_shl16_rm_cl(a, rm) x64_emit((a), 0x660000d3, X64_F_REGRM, 4, (rm), 0)
#define x64_shl32_rm_cl(a, rm) x64_emit((a), 0xd3, X64_F_REGRM, 4, (rm), 0)
#define x64_shl64_rm_cl(a, rm) x64_emit((a), 0xd3, X64_F_REXW|X64_F_REGRM, 4, (rm), 0)
#define x64_shr8_rm_cl(a, rm) x64_emit((a), 0xd2, X64_F_REGRM, 5, (rm), 0)
#define x64_shr16_rm_cl(a, rm) x64_emit((a), 0x660000d3, X64_F_REGRM, 5, (rm), 0)
#define x64_shr32_rm_cl(a, rm) x64_emit((a), 0xd3, X64_F_REGRM, 5, (rm), 0)
#define x64_shr64_rm_cl(a, rm) x64_emit((a), 0xd3, X64_F_REXW|X64_F_REGRM, 5, (rm), 0)
#define x64_sal8_rm_cl(a, rm) x64_emit((a), 0xd2, X64_F_REGRM, 6, (rm), 0)
#define x64_sal16_rm_cl(a, rm) x64_emit((a), 0x660000d3, X64_F_REGRM, 6, (rm), 0)
#define x64_sal32_rm_cl(a, rm) x64_emit((a), 0xd3, X64_F_REGRM, 6, (rm), 0)
#define x64_sal64_rm_cl(a, rm) x64_emit((a), 0xd3, X64_F_REXW|X64_F_REGRM, 6, (rm), 0)
#define x64_sar8_rm_cl(a, rm) x64_emit((a), 0xd2, X64_F_REGRM, 7, (rm), 0)
#define x64_sar16_rm_cl(a, rm) x64_emit((a), 0x660000d3, X64_F_REGRM, 7, (rm), 0)
#define x64_sar32_rm_cl(a, rm) x64_emit((a), 0xd3, X64_F_REGRM, 7, (rm), 0)
#define x64_sar64_rm_cl(a, rm) x64_emit((a), 0xd3, X64_F_REXW|X64_F_REGRM, 7, (rm), 0)
#define x64_test8_rm_imm(a, rm, imm) x64_emit((a), 0xf6, X64_F_IMM8|X64_F_REGRM, 0, (rm), (imm))
#define x64_test8_rm_imm8(a, rm, imm) x64_emit((a), 0xf6, X64_F_IMM8|X64_F_IMMSZ|X64_F_REGRM, 0, (rm), (imm))
#define x64_test16_rm_imm(a, rm, imm) x64_emit((a), 0x660000f7, X64_F_REGRM|X64_F_IMM16, 0, (rm), (imm))
#define x64_test16_rm_imm16(a, rm, imm) x64_emit((a), 0x660000f7, X64_F_IMMSZ|X64_F_REGRM|X64_F_IMM16, 0, (rm), (imm))
#define x64_test32_rm_imm(a, rm, imm) x64_emit((a), 0xf7, X64_F_REGRM|X64_F_IMM32, 0, (rm), (imm))
#define x64_test32_rm_imm32(a, rm, imm) x64_emit((a), 0xf7, X64_F_IMMSZ|X64_F_REGRM|X64_F_IMM32, 0, (rm), (imm))
#define x64_test64_rm_imm(a, rm, imm) x64_emit((a), 0xf7, X64_F_REGRM|X64_F_REXW|X64_F_IMM32, 0, (rm), (imm))
#define x64_test64_rm_imm32(a, rm, imm) x64_emit((a), 0xf7, X64_F_IMMSZ|X64_F_REGRM|X64_F_REXW|X64_F_IMM32, 0, (rm), (imm))
#define x64_not8_rm(a, rm) x64_emit((a), 0xf6, X64_F_REGRM, 2, (rm), 0)
#define x64_not16_rm(a, rm) x64_emit((a), 0x660000f7, X64_F_REGRM, 2, (rm), 0)
#define x64_not32_rm(a, rm) x64_emit((a), 0xf7, X64_F_REGRM, 2, (rm), 0)
#define x64_not64_rm(a, rm) x64_emit((a), 0xf7, X64_F_REXW|X64_F_REGRM, 2, (rm), 0)
#define x64_neg8_rm(a, rm) x64_emit((a), 0xf6, X64_F_REGRM, 3, (rm), 0)
#define x64_neg16_rm(a, rm) x64_emit((a), 0x660000f7, X64_F_REGRM, 3, (rm), 0)
#define x64_neg32_rm(a, rm) x64_emit((a), 0xf7, X64_F_REGRM, 3, (rm), 0)
#define x64_neg64_rm(a, rm) x64_emit((a), 0xf7, X64_F_REXW|X64_F_REGRM, 3, (rm), 0)
#define x64_mul8_ax_rm(a, rm) x64_emit((a), 0xf6, X64_F_REGRM, 4, (rm), 0)
#define x64_mul16_ax_rm(a, rm) x64_emit((a), 0x660000f7, X64_F_REGRM, 4, (rm), 0)
#define x64_mul32_ax_rm(a, rm) x64_emit((a), 0xf7, X64_F_REGRM, 4, (rm), 0)
#define x64_mul64_ax_rm(a, rm) x64_emit((a), 0xf7, X64_F_REXW|X64_F_REGRM, 4, (rm), 0)
#define x64_imul8_ax_rm(a, rm) x64_emit((a), 0xf6, X64_F_REGRM, 5, (rm), 0)
#define x64_imul16_ax_rm(a, rm) x64_emit((a), 0x660000f7, X64_F_REGRM, 5, (rm), 0)
#define x64_imul32_ax_rm(a, rm) x64_emit((a), 0xf7, X64_F_REGRM, 5, (rm), 0)
#define x64_imul64_ax_rm(a, rm) x64_emit((a), 0xf7, X64_F_REXW|X64_F_REGRM, 5, (rm), 0)
#define x64_div8_ax_rm(a, rm) x64_emit((a), 0xf6, X64_F_REGRM, 6, (rm), 0)
#define x64_div16_ax_rm(a, rm) x64_emit((a), 0x660000f7, X64_F_REGRM, 6, (rm), 0)
#define x64_div32_ax_rm(a, rm) x64_emit((a), 0xf7, X64_F_REGRM, 6, (rm), 0)
#define x64_div64_ax_rm(a, rm) x64_emit((a), 0xf7, X64_F_REXW|X64_F_REGRM, 6, (rm), 0)
#define x64_idiv8_ax_rm(a, rm) x64_emit((a), 0xf6, X64_F_REGRM, 7, (rm), 0)
#define x64_idiv16_ax_rm(a, rm) x64_emit((a), 0x660000f7, X64_F_REGRM, 7, (rm), 0)
#define x64_idiv32_ax_rm(a, rm) x64_emit((a), 0xf7, X64_F_REGRM, 7, (rm), 0)
#define x64_idiv64_ax_rm(a, rm) x64_emit((a), 0xf7, X64_F_REXW|X64_F_REGRM, 7, (rm), 0)
#define x64_call_rel(a, imm) x64_emit((a), 0xe8, X64_F_IMM32, 0, 0, (imm))
#define x64_call_rel32(a, imm) x64_emit((a), 0xe8, X64_F_IMMSZ|X64_F_IMM32, 0, 0, (imm))
#define x64_ret(a) x64_emit((a), 0xc3, 0, 0, 0, 0)
#define x64_inc8_rm(a, rm) x64_emit((a), 0xfe, X64_F_REGRM, 0, (rm), 0)
#define x64_inc16_rm(a, rm) x64_emit((a), 0x660000ff, X64_F_REGRM, 0, (rm), 0)
#define x64_inc32_rm(a, rm) x64_emit((a), 0xff, X64_F_REGRM, 0, (rm), 0)
#define x64_inc64_rm(a, rm) x64_emit((a), 0xff, X64_F_REXW|X64_F_REGRM, 0, (rm), 0)
#define x64_dec8_rm(a, rm) x64_emit((a), 0xfe, X64_F_REGRM, 1, (rm), 0)
#define x64_dec16_rm(a, rm) x64_emit((a), 0x660000ff, X64_F_REGRM, 1, (rm), 0)
#define x64_dec32_rm(a, rm) x64_emit((a), 0xff, X64_F_REGRM, 1, (rm), 0)
#define x64_dec64_rm(a, rm) x64_emit((a), 0xff, X64_F_REXW|X64_F_REGRM, 1, (rm), 0)
#define x64_call_rm(a, rm) x64_emit((a), 0xff, X64_F_REGRM, 2, (rm), 0)
#define x64_jmp_rm(a, rm) x64_emit((a), 0xff, X64_F_REGRM, 4, (rm), 0)

#endif

#if defined(X64A_IMPLEMENTATION) || defined(__INTELLISENSE__)
#ifndef X64A_H_IMPLEMENTED
#define X64A_H_IMPLEMENTED

const uint8_t x64_reloc_type_size[X64_NUM_RELOC_TYPES] = {
	1, 4, 1, 4,
};

bool x64_reserve_code(x64_assembler *a, size_t min_length)
{
	size_t cap = a->code_cap ? a->code_cap * 2 : min_length;
	uint8_t *code = (uint8_t*)realloc(a->code, cap);
	if (!code) {
		a->error = true;
		return false;
	}
	a->code = code;
	a->code_cap = cap;
	return true;
}

size_t x64_new_symbol(x64_assembler *a)
{
	if (a->num_symbols == a->symbol_cap) {
		size_t cap = a->symbol_cap ? a->symbol_cap * 2 : 32;
		x64_symbol *syms = (x64_symbol*)realloc(a->symbols, cap * sizeof(x64_symbol));
		if (!syms) {
			a->error = true;
			return SIZE_MAX;
		}
		a->symbols = syms;
		a->symbol_cap = cap;
	}

	size_t index = a->num_symbols++;
	a->symbols[index].offset = SIZE_MAX;
	return index;
}

bool x64_set_label(x64_assembler *a, size_t symbol)
{
	if (a->num_labels == a->label_cap) {
		size_t cap = a->label_cap ? a->label_cap * 2 : 32;
		x64_label *syms = (x64_label*)realloc(a->labels, cap * sizeof(x64_label));
		if (!syms) {
			a->error = true;
			return false;
		}
		a->labels = syms;
		a->label_cap = cap;
	}

	size_t index = a->num_labels++;
	a->labels[index].offset = a->code_length;
	a->labels[index].symbol = symbol;

	assert(a->symbols[symbol].offset == SIZE_MAX);
	a->symbols[symbol].offset = a->code_length;
	return true;
}

bool x64_add_reloc(x64_assembler *a, x64_reloc_type type, size_t offset, size_t symbol)
{
	if (a->num_relocs == a->reloc_cap) {
		size_t cap = a->reloc_cap ? a->reloc_cap * 2 : 32;
		x64_reloc *relocs = (x64_reloc*)realloc(a->relocs, cap * sizeof(x64_reloc));
		if (!relocs) {
			a->error = true;
			return false;
		}
		a->relocs = relocs;
		a->reloc_cap = cap;
	}

	x64_reloc *reloc = &a->relocs[a->num_relocs++];
	reloc->offset = offset;
	reloc->symbol = symbol;
	reloc->type = type;
	return true;
}

bool x64_jmp_symbol(x64_assembler *a, size_t symbol) {
	size_t offset = x64_jmp_rel32(a, 0);
	return x64_add_reloc(a, X64_RELOC_JMP_REL32, offset - 4, symbol);
}

bool x64_jcc_symbol(x64_assembler *a, x64_cc cc, size_t symbol) {
	size_t offset = x64_jcc_rel32(a, cc, 0);
	return x64_add_reloc(a, X64_RELOC_JCC_REL32, offset - 4, symbol);
}

void x64_optimize_relocs(x64_assembler *a)
{
	size_t shift = 0;
	x64_label *label = a->labels;
	x64_label *label_end = a->labels + a->num_labels;
	for (size_t i = 0; i < a->num_relocs; i++) {
		x64_reloc *reloc = &a->relocs[i];
		size_t offset = reloc->offset - shift;
		x64_reloc_type type = reloc->type;
		if (type == X64_RELOC_JCC_REL32 || type == X64_RELOC_JMP_REL32) {
			size_t target = a->symbols[reloc->symbol].offset;
			uint8_t *dst = a->code + offset;
			if (target != SIZE_MAX) {
				if (target > offset) target -= shift;
				intptr_t delta = (intptr_t)(target - offset);
				if (delta >= -128 && delta < 127) {
					if (type == X64_RELOC_JCC_REL32) {
						dst[-2] = (uint8_t)(dst[-1] - 0x10);
						dst[-1] = 0;
						offset--;
						reloc->type = X64_RELOC_JCC_REL8;
						shift += 4;
					} else if (type == X64_RELOC_JMP_REL32) {
						dst[-1] += 2;
						reloc->type = X64_RELOC_JMP_REL8;
						shift += 3;
					}
				}
			}
		}
		reloc->offset = offset;
		if (shift > 0) {
			size_t begin = reloc->offset + x64_reloc_type_size[reloc->type];
			size_t end = a->code_length;
			if (i + 1 < a->num_relocs) {
				x64_reloc *next_reloc = &a->relocs[i + 1];
				end = next_reloc->offset + x64_reloc_type_size[next_reloc->type];
			}
			memmove(a->code + begin, a->code + begin + shift, (end - (begin + shift)));

			while (label != label_end && label->offset <= begin) {
				label++;
			}
			while (label != label_end && label->offset < end) {
				label->offset -= shift;
				a->symbols[label->symbol].offset = label->offset;
				label++;
			}
		}
	}
	a->code_length -= shift;
}

void x64_patch_relocs(x64_assembler *a)
{
	for (size_t i = 0; i < a->num_relocs; i++) {
		x64_reloc *reloc = &a->relocs[i];
		size_t offset = reloc->offset;
		size_t target = a->symbols[reloc->symbol].offset;
		if (target != SIZE_MAX) {
			uint8_t size = x64_reloc_type_size[reloc->type];
			intptr_t delta = (intptr_t)(target - (offset + size));
			uint8_t *dst = a->code + offset;
			if (size == 1) {
				dst[0] = (uint8_t)delta;
			} else if (size == 4) {
				dst[0] = (uint8_t)(delta >>  0);
				dst[1] = (uint8_t)(delta >>  8);
				dst[2] = (uint8_t)(delta >> 16);
				dst[3] = (uint8_t)(delta >> 24);
			}
		}
	}
}

#endif
#endif
