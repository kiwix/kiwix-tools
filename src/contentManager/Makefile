
all: contentTester

wrapperlib:
	g++ -shared -o libContentCManager.so ContentManagerWrapper.cpp contentManager.cpp ../common/kiwix/reader.cpp -I . -I ../zimAccessor/ -I ../common/ -I ../zimlib/include/ -L ../zimlib/src/.libs/ -lzim ../dependences/xz/src/liblzma/.libs/liblzma.a -I /usr/include/

contentTester:
	g++ contentManagerTester.cpp contentManager.cpp ../common/kiwix/library.cpp ../common/kiwix/manager.cpp ../common/kiwix/reader.cpp ../common/base64.cpp ../common/pathTools.cpp ../common/componentTools.cpp ../common/regexTools.cpp ../common/kiwix/library.h ../common/kiwix/manager.h ../common/kiwix/reader.h ../pugixml/pugixml.hpp ../common/base64.h ../common/pathTools.h ../common/componentTools.h ../common/regexTools.h -o contentTester -I ../common/ -I ../pugixml/ -I ../zimlib/include/ -I /usr/include/ -L ../zimlib/src/.libs/ -lzim ../pugixml/.libs/libpugixml.a ../dependences/xz/src/liblzma/.libs/liblzma.a

contentCTester:
	ln -fs ../zimAccessor/libZimCAccessor.so .
	gcc -o zimCTester zimCTester.c -I ../zimAccessor/ -L ../zimAccessor/ -lZimCAccessor

cleanc:
	find . -name 'contentCTester' -delete

cleancpp:
	find . -name 'contentTester' -delete

clean: cleanc cleancpp
	find . -name 'libContentCManager.so' -delete
	find . -name '*.o' -delete
