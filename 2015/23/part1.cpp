#include <stdio.h>
#include "jit.h"

int main(int argc, char **argv)
{
	JIT *jit = jitCreate();
	jitCompile(jit, stdin);
	jitFinish(jit);

	VM vm = { 0 };
	jitExec(jit, &vm);

	printf("%d\n", vm.regB);

	return 0;
}

