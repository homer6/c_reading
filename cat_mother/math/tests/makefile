src = *.cpp ../../tester/*.cpp
libs = -lpthread ../../lang/lib/lang.a ../../math/lib/math.a

test : $(src)
	rm -f test
	g++ -o test -I. -I- -I../.. $(src) $(libs)
