main:
	gcc prefetch.c -o c_prefetch && \
	gcc noprefetch.c -o no_prefetch && \
	g++ prefetch.cpp -o cpp_prefetch 

clean:
	rm -f c_prefetch cpp_prefetch no_prefetch