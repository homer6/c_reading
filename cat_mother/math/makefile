obj = lib/*.o
src = *.cpp internal/*.cpp
lib = lib/math.a

$(lib) : $(src)
	g++ -c -Iinternal -I. -I- -I.. $(src)
	mv *.o lib
	rm -f $(lib)
	ar -r $(lib) $(obj)
	rm -f $(obj)
