#!/usr/bin/env python

import sys
import os
import subprocess
import re


def usage():
	print(u"Usage:	%s component/path.dylib" % sys.argv[0])

if len(sys.argv) < 2:
	usage()
	exit(0)

component = sys.argv[1]

if not os.path.exists(component):
	print(u"Unable to access component at: %s" % component)
	exit(1)

# we'll also allow binaries to be fixed.
#
# if not component.endswith('.dylib'):
# 	print(u"%s is not a dylib component" % component)
# 	exit(1)

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
		newpath = u'@executable_path/../Frameworks/lib%s.dylib' % match.groups()[0]
		print('install_name_tool -change %s %s %s' % (path, newpath, component))
		os.system('install_name_tool -change %s %s %s' % (path, newpath, component))
		continue

	print('\tUnmatched: %s' % path)