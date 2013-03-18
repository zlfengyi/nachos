#include "syscall.h"

const int MAXN = 10;
int a[10];
int main() {
	int i;
	for (i = 0; i < MAXN; i++) {
		a[i] = i;
	//	printf("the value in a[%d] is %d\n", i, a[i]);
	}
	Halt();
}
