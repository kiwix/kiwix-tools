
all: zimCTester zimTester

zimTester:
	g++ zimTester.cpp ../common/kiwix/reader.cpp ../zimAccessor/zimAccessor.cpp -o zimTester -I . -I ../zimAccessor/ -I ../common/ -I ../zimlib/include/ -L ../zimlib/src/.libs/ -lzim ../dependences/xz/src/liblzma/.libs/liblzma.a -I /usr/include/

zimCTester:
	ln -fs ../zimAccessor/libZimCAccessor.so .
	gcc -o zimCTester zimCTester.c -I ../zimAccessor/ -L ../zimAccessor/ -lZimCAccessor

cleanc:
	find . -name 'zimCTester' -delete

cleancpp:
	find . -name 'zimTester' -delete

clean: cleanc cleancpp
	find . -name 'libZimCAccessor.so' -delete
	find . -name '*.o' -delete

