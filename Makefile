install: judge test

judge:
	gcc judge.c -o judge

test:
	gcc test.c -o test
