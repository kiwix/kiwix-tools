
all: contentTester

wrapperlib:
	g++ -shared ContentManagerWrapper.cpp -o libContentCManager.so contentManager.cpp ../common/kiwix/library.cpp ../common/kiwix/manager.cpp ../common/kiwix/reader.cpp ../common/base64.cpp ../common/pathTools.cpp ../common/componentTools.cpp ../common/regexTools.cpp ../common/kiwix/library.h ../common/kiwix/manager.h ../common/kiwix/reader.h ../pugixml/pugixml.hpp ../common/base64.h ../common/pathTools.h ../common/componentTools.h ../common/regexTools.h -I . -I ../common/ -I ../pugixml/ -I ../zimlib/include/ -I /usr/include/ -L ../zimlib/src/.libs/ -lzim ../pugixml/.libs/libpugixml.a ../dependences/xz/src/liblzma/.libs/liblzma.a

contentTester:
	g++ contentManagerTester.cpp contentManager.cpp ../common/kiwix/library.cpp ../common/kiwix/manager.cpp ../common/kiwix/reader.cpp ../common/base64.cpp ../common/pathTools.cpp ../common/componentTools.cpp ../common/regexTools.cpp ../common/kiwix/library.h ../common/kiwix/manager.h ../common/kiwix/reader.h ../pugixml/pugixml.hpp ../common/base64.h ../common/pathTools.h ../common/componentTools.h ../common/regexTools.h -o contentTester -I ../common/ -I ../pugixml/ -I ../zimlib/include/ -I /usr/include/ -L ../zimlib/src/.libs/ -lzim ../pugixml/.libs/libpugixml.a ../dependences/xz/src/liblzma/.libs/liblzma.a

contentCTester: wrapperlib
	gcc -o contentCTester contentCTester.c -I . -L . -lContentCManager -ldl -lm   -L/usr/lib -licui18n -licuuc -licudata  -ldl -lm

cleanc:
	find . -name 'contentCTester' -delete

cleancpp:
	find . -name 'contentTester' -delete

clean: cleanc cleancpp
	find . -name 'libContentCManager.so' -delete
	find . -name '*.o' -delete
