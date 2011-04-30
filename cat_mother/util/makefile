obj = lib/*.o
src = *.cpp internal/*.cpp
lib = lib/util.a

$(lib) : $(src)
	g++ -D ASM_X86 -c -Iinternal -I. -I- -I.. $(src)
	mv *.o lib
	rm -f $(lib)
	ar -r $(lib) $(obj)
	rm -f $(obj)
