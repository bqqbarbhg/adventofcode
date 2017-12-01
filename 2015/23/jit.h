#pragma once

#include <stdint.h>
#include <stdio.h>

struct VM
{
	uint32_t regA;
	uint32_t regB;
};

struct JIT;

JIT *jitCreate();
void jitEmit(JIT *jit, const char *code, uint32_t length);
void jitInstReloc(JIT *jit, int32_t offset, uint32_t inst);
void jitFinish(JIT *jit);
void jitExec(JIT *jit, VM *vm);
void jitCompile(JIT *jit, FILE *f);

