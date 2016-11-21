#define _CRT_SECURE_NO_WARNINGS
#include "jit.h"
#include <stdio.h>
#include <stdlib.h>
#include <Windows.h>
#include <stdint.h>

struct InstReloc
{
	uint32_t off;
	uint32_t inst;
};

typedef void jitFunc(VM*);

struct JIT
{
	char *code;
	uint32_t off;

	uint32_t *instOff;
	uint32_t numInst;

	InstReloc *instRelocs;
	uint32_t numInstRelocs;
};

JIT *jitCreate()
{
	JIT *jit = (JIT*)calloc(sizeof(JIT), 1);
	jit->code = (char*)VirtualAlloc(NULL, 4*1024, MEM_COMMIT|MEM_RESERVE, PAGE_EXECUTE_READWRITE);
	jit->instOff = (uint32_t*)calloc(sizeof(uint32_t), 1024);
	jit->instRelocs = (InstReloc*)calloc(sizeof(InstReloc), 1024);

	jitEmit(jit, "\x8B\x01\x8B\x59\x04", 5);

	return jit;
}

void jitEmit(JIT *jit, const char *code, uint32_t length)
{
	memcpy(jit->code + jit->off, code, length);
	jit->instOff[jit->numInst] = jit->off;

	jit->off += length;
	jit->numInst++;
}

void jitInstReloc(JIT *jit, int32_t offset, uint32_t inst)
{
	InstReloc *ir = &jit->instRelocs[jit->numInstRelocs++];
	ir->off = (uint32_t)((int32_t)jit->off + offset);
	ir->inst = inst;
}

void jitFinish(JIT *jit)
{
	jitEmit(jit, "\x89\x01\x89\x59\x04\xC3", 6);

	for (uint32_t i = 0; i < jit->numInstRelocs; i++)
	{
		InstReloc *ir = &jit->instRelocs[i];
		int32_t off = jit->instOff[ir->inst] - (ir->off + 4);
		*(uint32_t*)(jit->code + ir->off) += off;
	}
}

void jitExec(JIT *jit, VM *vm)
{
	((jitFunc*)jit->code)(vm);
}

void jitCompile(JIT *jit, FILE *f)
{
	char line[128];

	while (fgets(line, sizeof(line), f))
	{
		char reg;
		int32_t off;

		if (sscanf(line, "hlf %c", &reg) == 1)
		{
			if (reg == 'a')
				jitEmit(jit, "\xD1\xE8", 2);
			else if (reg == 'b')
				jitEmit(jit, "\xD1\xEB", 2);
			else
				exit(1);
		}
		else if (sscanf(line, "tpl %c", &reg) == 1)
		{
			if (reg == 'a')
				jitEmit(jit, "\x8D\x04\x40", 3);
			else if (reg == 'b')
				jitEmit(jit, "\x8D\x1C\x5B", 3);
			else
				exit(1);
		}
		else if (sscanf(line, "inc %c", &reg) == 1)
		{
			if (reg == 'a')
				jitEmit(jit, "\x83\xC0\x01", 3);
			else if (reg == 'b')
				jitEmit(jit, "\x83\xC3\x01", 3);
			else
				exit(1);
		}
		else if (sscanf(line, "jmp %d", &off) == 1)
		{
			jitEmit(jit, "\xE9\x00\x00\x00\x00", 5);
			jitInstReloc(jit, -4, (int)jit->numInst - 1 + off);
		}
		else if (sscanf(line, "jie %c, %d", &reg, &off) == 2)
		{
			if (reg == 'a')
				jitEmit(jit, "\xA8\x01\x0F\x84\x00\x00\x00\x00", 8);
			else if (reg == 'b')
				jitEmit(jit, "\xF6\xC3\x01\x0F\x84\x00\x00\x00\x00", 9);
			else
				exit(1);
			jitInstReloc(jit, -4, (int)jit->numInst - 1 + off);
		}
		else if (sscanf(line, "jio %c, %d", &reg, &off) == 2)
		{
			if (reg == 'a')
				jitEmit(jit, "\x83\xF8\x01\x0F\x84\x00\x00\x00\x00", 9);
			else if (reg == 'b')
				jitEmit(jit, "\x83\xFB\x01\x0F\x84\x00\x00\x00\x00", 9);
			else
				exit(1);
			jitInstReloc(jit, -4, (int)jit->numInst - 1 + off);
		}
		
	}
}

