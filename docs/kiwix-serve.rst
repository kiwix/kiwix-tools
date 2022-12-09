***********
kiwix-serve
***********

Introduction
============

``kiwix-serve`` is a tool for serving ZIM file content over HTTP. It supports
serving a library containing multiple ZIM files. In a large library served by a
``kiwix-serve`` instance clients can look up/filter ZIM files of interest by
words in their :term:`titles <ZIM title>` and/or descriptions, language, tags, etc.

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


Arguments
---------

.. _cli-arg-library-file-path:

``LIBRARY_FILE_PATH``: path of an XML library file listing ZIM files to serve.
To be used only with the :option:`--library` option. Multiple library files can
be provided as a semicolon (``;``) separated list.

``ZIM_FILE_PATH``: ZIM file path (multiple arguments are allowed).

Options
-------

.. option:: --library

  By default, ``kiwix-serve`` expects a list of ZIM files as command line
  arguments. Providing the :option:`--library` option tells ``kiwix-serve``
  that the command line argument is rather a :ref:`library XML file
  <cli-arg-library-file-path>`.

.. option:: -i ADDR, --address=ADDR

  Listen only on this IP address. By default the server listens on all
  available IP addresses.


.. option:: -p PORT, --port=PORT

  TCP port on which to listen for HTTP requests (default: 80).


.. option:: -r ROOT, --urlRootLocation=ROOT

  URL prefix on which the content should be made available (default: empty).


.. option:: -d, --daemon

  Detach the HTTP server daemon from the main process.


.. option:: -a PID, --attachToProcess=PID

  Exit when the process with id PID stops running.


.. option:: -M, --monitorLibrary

  Monitor the XML library file and reload it automatically when it changes.


.. option:: -m, --nolibrarybutton

  Disable the library home button in the ZIM viewer toolbar.


.. option:: -n, --nosearchbar

  Disable the searchbox in the ZIM viewer toolbar.


.. option:: -b, --blockexternal

  Prevent the users from directly navigating to external resources via such
  links in ZIM content.


.. option:: -t N, --threads=N

  Number of threads to run in parallel (default: 4).


.. option:: -s N, --searchLimit=N

  Maximum number of ZIM files in a fulltext multizim search (default: No limit).


.. option:: -z, --nodatealiases

  Create URL aliases for each content by removing the date embedded in the file
  name. The expected format of the date in the filename is ``_YYYY-MM``. For
  example, ZIM file ``wikipedia_en_all_2020-08.zim`` will be accessible both as
  ``wikipedia_en_all_2020-08`` and ``wikipedia_en_all``.


.. option:: -c PATH, --customIndex=PATH

  Override the welcome page with a custom HTML file.


.. option:: -L N, --ipConnectionLimit=N

  Max number of (concurrent) connections per IP (default: infinite,
  recommended: >= 6).


.. option:: -v, --verbose

  Print debug log to STDOUT.


.. option:: -V, --version

  Print the software version.


.. option:: -h, --help

  Print the help text.


HTTP API
========

``kiwix-serve`` serves content at/under ``http://ADDR:PORT/ROOT`` where
``ADDR``, ``PORT`` and ``ROOT`` are the values supplied to the
:option:`--address`/:option:`-i`, :option:`--port`/:option:`-p` and
:option:`--urlRootLocation`/:option:`-r` options, respectively.

HTTP API endpoints presented below are relative to that location, i.e.
``/foo/bar`` must be actually accessed as ``http://ADDR:PORT/ROOT/foo/bar``.

.. _welcome-page:

``/``
-----

Welcome page is served under ``/``. By default this is the library page, where
books are listed and can be looked up/filtered interactively. However, the
welcome page can be overriden through the :option:`--customIndex`/:option:`-c`
command line option of ``kiwix-serve``.


``/catalog/v2`` (New API)
-------------------------

Blablabla


``/catalog/v2/root.xml``
^^^^^^^^^^^^^^^^^^^^^^^^

Blablabla


``/catalog/v2/searchdescription.xml``
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

Blablabla


``/catalog/v2/categories``
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

Blablabla


``/catalog/v2/entries``
^^^^^^^^^^^^^^^^^^^^^^^

Blablabla


``/catalog/v2/entry``
^^^^^^^^^^^^^^^^^^^^^

Blablabla


``/catalog/v2/illustration``
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

Blablabla


``/catalog/v2/languages``
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

Blablabla


``/catalog/v2/partial_entries``
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

Blablabla


``/catalog`` (Legacy API)
-------------------------

Blablabla


``/catalog/root.xml``
^^^^^^^^^^^^^^^^^^^^^

Blablabla


``/catalog/searchdescription.xml``
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

Blablabla


``/catalog/search``
^^^^^^^^^^^^^^^^^^^

Blablabla


``/catch``
----------

Blablabla


``/content``
------------

ZIM file content is served under the ``/content`` endpoint as described below.


``/content/ZIMNAME/PATH/IN/ZIMFILE``
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

Returns the entry with path ``PATH/IN/ZIMFILE`` from ZIM file with :term:`name
<ZIM name>` ``ZIMNAME``.


``/content/ZIMNAME``
^^^^^^^^^^^^^^^^^^^^

``/content/ZIMNAME`` redirects to the main page of the ZIM file with :term:`name
<ZIM name>` ``ZIMNAME`` (unless that ZIM file contains an entry with an empty
path or path equal to ``/``, in which case that entry is returned).


``/random``
-----------

**Usage:**

  ``/random?content=ZIMNAME``

Generates a HTTP redirect to a randomly selected article/page from the
specified ZIM file.

**Parameters:**

  ``content``: :term:`name <ZIM name>` of the ZIM file.


``/raw``
--------

Blablabla


``/search``
-----------

Blablabla


``/skin``
-----------

Static front-end resources (such as CSS, javascript and images) are all grouped
under ``/skin``.

**Usage:**
  ``/skin/PATH/TO/RESOURCE[?cacheid=CACHEID]``

`Cache busting
<https://javascript.plainenglish.io/what-is-cache-busting-55366b3ac022>`_ of
static resources is supported via the optional param ``cacheid``. By default,
i.e. when the ``cacheid`` parameter is not specified while accessing the
``/skin`` endpoint, static resources are served as if they were dynamic (i.e.
could be different for an immediately repeated request). Specifying the
``cacheid`` parameter with a correct value (matching the value embedded in the
``kiwix-serve`` instance), makes the returned resource to be presented as
immutable. However, if the value of the ``cacheid`` parameter mismatches then
``kiwix-serve`` responds with a 404 HTTP error.

``kiwix-serve``'s default front-end (the :ref:`welcome page <welcome-page>` and
the :ref:`ZIM file viewer <zim-file-viewer>`) access all underlying static
resources by using explicit ``cacheid`` s.


``/suggest``
------------

Blablabla


.. _zim-file-viewer:

``/viewer``
-----------

ZIM file viewer. The ZIM file and entry therein must be specified via the hash
component of the URL as ``/viewer#ZIMNAME/PATH/IN/ZIMFILE``.


``/viewer_settings.js``
-----------------------

Settings of the ZIM file viewer that are configurable via certain command line
options of ``kiwix-serve`` (e.g. ``--nolibrarybutton``).


/ANYTHING/ELSE
--------------

Any other URL is considered as an attempt to access ZIM file content using the
legacy URL scheme and is redirected to ``/content/ANYTHING/ELSE``.


Glossary
========

.. glossary::

  ZIM filename

    Name of a ZIM file on the server filesystem.

  ZIM name

    Identifier of a ZIM file in the server's library (used for referring to a
    particular ZIM file in requests).

    For a ``kiwix-serve`` started with a list of ZIM files, ZIM names are
    derived from the filename by dropping the extension and replacing certain
    characters (spaces are replaced with underscores, and ``+`` symbols are
    replaced with the text ``plus``). Presence of the
    :option:`-z`/:option:`--nodatealiases` option will create additional names
    (aliases) for filenames with dates.

    For a ``kiwix-serve`` started with the :option:`--library` option, ZIM
    names come from the library XML file.

    ZIM names are expected to be unique across the library. Any name conflicts
    (including those caused by the usage of the
    :option:`-z`/:option:`--nodatealiases` option) are reported on STDERR but,
    otherwise, are ignored.

  ZIM title

    Title of a ZIM file. This can be any text (with whitespace). It is never
    used as a way of referring to a ZIM file.

  ZIM UUID

    This is a unique identifier of a ZIM file designated at its creation time
    and embedded in the ZIM file. Certain ``kiwix-serve`` operations may
    require that a ZIM file be referenced through its UUID rather than name.
