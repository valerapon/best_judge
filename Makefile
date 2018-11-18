install: judge test checker1 checker2

judge:
	gcc judge.c -o judge

test:
	gcc test.c -o test

checker1:
	gcc checkers/checker_byte.c -o var/checker_byte

checker2:
	gcc checkers/checker_int.c -o var/checker_int
