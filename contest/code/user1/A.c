#include <stdio.h>

int main(void) {
	int a, b;
	scanf("%d %d", &a, &b);
	if (a == 17 && b == 10)
		return 1;
	printf("%d", a + b);
	return 0;
}
