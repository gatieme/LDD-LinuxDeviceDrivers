#include <stdio.h>

int func3(int a, int b)
{
	int c = a + b;
	printf("c = %d\n", c);
	return a + b;
}

int func2(int a)
{
	int b = 20;

	b = func3(a, b);
	printf("b = %d\n", b);

	return b;
}

int func1(void)
{
	int a;

	a = func2(10);
	printf("a = %d\n", a);

	return a;
}

int main(int argc, const char *argv[])
{
	func1();

	return 0;
}
