if IS_OSX
SUBDIRS = ${DEPENDENCES} \
	zimlib \
	ctpp2 \
	pugixml \
	components \
	manager \
	dependences
else
SUBDIRS = ${DEPENDENCES} \
	zimlib \
	ctpp2 \
	pugixml \
	components \
	server \
	manager 
if IS_STATIC
else
SUBDIRS += indexer \
	installer \
	searcher \
	reader
endif
endif
