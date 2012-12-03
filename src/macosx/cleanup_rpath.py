#!/usr/bin/env python

import sys
import os
import subprocess
import re

MODE_COMPONENT = 1
MODE_BINARY = 2
MODE_LIBRARY = 3

MODES = {'c': MODE_COMPONENT,
		 'b': MODE_BINARY,
		 'l': MODE_LIBRARY}

is_library = False
is_component = False
is_binary = False


def usage():
	print(u"Usage:	%s MODE component/path.dylib" % sys.argv[0])
	print(u"		MODE is c: component or b: binary or l: library")

if len(sys.argv) < 3:
	usage()
	exit(0)

component = sys.argv[-1]
mode = MODES.get(sys.argv[-2].lower())

if not mode:
	usage()
	exit(1)

# mode switchers
if mode == MODE_COMPONENT:
	is_component = True
elif mode == MODE_LIBRARY:
	is_library = True
else:
	is_binary = True

if not os.path.exists(component):
	print(u"Unable to access component at: %s" % component)
	exit(1)

# we'll also allow binaries to be fixed.
#
if (is_library or is_component) and not component.endswith('.dylib'):
	print(u"%s is not a dylib component" % component)
	exit(1)

print("Fixing %s..." % component)

basename = os.path.basename(component)
try:
	name, ext = basename.rsplit('.', 1)
except ValueError:
	name = basename
	ext = ''
libname = u'lib%s%s.0.dylib' % (name[0].upper(), name[1:])

# run otool to get a list of deps.
otool = subprocess.Popen(['otool', '-L', component], stdout=subprocess.PIPE)
otool_out, otool_err = otool.communicate()

for line in otool_out.split('\n'):
	if ('executable_path' in line
		or 'libSystem' in line
		or 'libstdc++' in line
		or ':' in line
		or not len(line)
		or 'aria2c' in basename):
		continue
	path, junk = line.strip().split(' (', 1)
	# erroneous_links.append(path)

	_basename = os.path.basename(path).strip()

	# remove the libXX.0.dylib from the component (optionnal)
	# doesn't seem to work.
	if _basename == libname:
		# os.system('install_name_tool -delete_rpath %s %s' % (path, component))
		continue

	# is it a library link?
	match = re.match(r'lib([a-z\_\-\d]+)([\.?\d]*)\.dylib', _basename)
	if match:
		print("match: %s" % match.groups()[0])
		if is_component:
			newpath = u'@executable_path/../Frameworks/lib%s.dylib' % match.groups()[0]
		elif is_binary:
			newpath = u'@executable_path/../../Frameworks/lib%s.dylib' % match.groups()[0]
		else:
			newpath = u'@loader_path/lib%s.dylib' % match.groups()[0]

		print('install_name_tool -change %s %s %s' % (path, newpath, component))
		os.system('install_name_tool -change %s %s %s' % (path, newpath, component))
		continue

	print('\tUnmatched: %s' % path)