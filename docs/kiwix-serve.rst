***********
kiwix-serve
***********

Introduction
============

``kiwix-serve`` is a tool for serving ZIM file content over HTTP. It supports
serving a library containing multiple ZIM files. In a large library served by a
``kiwix-serve`` instance clients can look up/filter ZIM files of interest by
words in their titles and/or descriptions, language, tags, etc.

``kiwix-serve`` provides a ZIM file viewer for displaying inidividual pages
from a ZIM file inside the user's web browser (without downloading the full ZIM
file).

Clients can also remotely search inside those ZIM files that contain a full-text
search database.


Usage
=====

.. code-block:: sh

  kiwix-serve --library [OPTIONS] LIBRARY_FILE_PATH
  kiwix-serve [OPTIONS] ZIM_FILE_PATH ...


Mandatory arguments
-------------------

``LIBRARY_FILE_PATH``: XML library file path listing ZIM file to serve. To be
used only with the ``--library`` option.

``ZIM_FILE_PATH``: ZIM file path (multiple arguments are allowed).

Options
-------

-i ADDR, --address=ADDR

  Listen only on this IP address. By default the server listens on all
  available IP addresses.


-p PORT, --port=PORT

  TCP port on which to listen for HTTP requests (default: 80).


-r ROOT, --urlRootLocation=ROOT

  URL prefix on which the content should be made available (default: /).


-d, --daemon

  Detach the HTTP server daemon from the main process.


-a PID, --attachToProcess=PID

  Exit when the process with id PID stops running.


-M, --monitorLibrary

  Monitor the XML library file and reload it automatically when it changes.


-m, --nolibrarybutton

  Disable the library home button in the ZIM viewer toolbar.


-n, --nosearchbar

  Disable the searchbox in the ZIM viewer toolbar.


-b, --blockexternal

  Prevent the users from directly navigating to external resources via such
  links in ZIM content.


-t N, --threads=N

  Number of threads to run in parallel (default: 4).


-s N, --searchLimit=N

  Maximum number of ZIM files in a fulltext multizim search (default: No limit).


-z, --nodatealiases

  Create URL aliases for each content by removing the date embedded in the file
  name. The expected format of the date in the filename is ``_YYYY-MM``. For
  example, ZIM file ``wikipedia_en_all_2020-08.zim`` will be accessible both as
  ``wikipedia_en_all_2020-08`` and ``wikipedia_en_all``.


-c PATH, --customIndex=PATH

  Override the welcome page with a custom HTML file.


-L N, --ipConnectionLimit=N

  Max number of (concurrent) connections per IP (default: infinite,
  recommended: >= 6).


-v, --verbose

  Print debug log to STDOUT.


-V, --version

  Print the software version.


-h, --help

  Print the help text.

