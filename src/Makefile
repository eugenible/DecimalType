CC:=gcc
OS=$(shell lsb_release -si)
UBUNTU=Ubuntu
TST_LBS=
LIBRARY=s21_decimal.c
TEST_LIBRARY=test.c s21_decimal.c
LIBNAME=s21_decimal
TEST_NAME=test.out


ifeq ($(OS), Darwin)
	TST_LBS=$(shell pkg-config --cflags --libs check) -lm
else 
ifeq ($(OS), Ubuntu)
	TST_LBS=$(shell pkg-config --libs check)
else 
	TST_LBS=$(shell pkg-config --cflags check pkg-config --libs check)-lm
endif
endif


all: s21_decimal.a test gcov_report

lbr: 
	$(CC) $(LIBRARY) -lm

clean: 
	rm -rf report
	rm -f *.o
	rm -f *.a
	rm -f *.gcda
	rm -f *.gcno
	rm -f test.info
	rm -f a.out
	rm -f $(TEST_NAME)

rebuild : clean all

test :s21_decimal.a
	$(CC) -c test.c
	$(CC) test.o $(LIBNAME).a -o $(TEST_NAME) $(TST_LBS)

test_2: 
	gcc test.c s21_decimal.c -lcheck_pic -pthread -lrt -lm -lsubunit

test_ : 
	-./$(TEST_NAME)

lcov : s21_decimal.a
	$(CC) --coverage -c test.c s21_decimal.c $(TST_LBS)
	$(CC) --coverage test.o s21_decimal.o $(LIBNAME).a -o $(TEST_NAME) $(TST_LBS)

lcov_ :
	lcov -t "Test_decimal" -o test.info -c -d . --rc lcov_branch_coverage=1
	genhtml -o report test.info --rc lcov_branch_coverage=1

gcov_report : lcov test_ lcov_ 

s21_decimal.a :
	$(CC) -c s21_decimal.c
	ar rc s21_decimal.a s21_decimal.o
	ranlib s21_decimal.a

check:
	cppcheck —enable=all —suppress=missingIncludeSystem *.c *.h
	python3 ../materials/linters/cpplint.py --extensions=c *.c *.h
valgrind:
	valgrind --leak-check=full ./test.out -s

firefox:
	firefox report/index.html
