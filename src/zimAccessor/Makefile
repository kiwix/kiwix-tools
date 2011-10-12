
all: wrapperlib

wrapperlib:
	g++ -shared -o libZimCAccessor.so ZimAccessorWrapper.cpp zimAccessor.cpp ../common/kiwix/reader.cpp -I . -I ../zimAccessor/ -I ../common/ -I ../zimlib/include/ -L ../zimlib/src/.libs/ -lzim ../dependences/xz/src/liblzma/.libs/liblzma.a -I /usr/include/

clean:
	find . -name 'libZimCAccessor.so' -delete
	find . -name '*.o' -delete
