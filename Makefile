install: judge test checkers

judge:
	gcc judge.c -o judge

test:
	gcc test.c -o test

checkers:
	gcc checkers/checker_byte.c -o var/checker_byte
