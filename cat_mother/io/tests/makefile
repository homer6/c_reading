src = *.cpp ../../tester/*.cpp
libs = -lpthread ../../lang/lib/lang.a ../../io/lib/io.a

test : $(src)
	rm -f test
	g++ -o test -I. -I- -I../.. $(src) $(libs)
