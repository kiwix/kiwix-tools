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
  available IP addresses. Alternatively, you can use special values to define which types of connections to accept:

  - all : Listen for connections on all IP addresses (IPv4 and IPv6).
  - ipv4 : Listen for connections on all IPv4 addresses.
  - ipv6 : Listen for connections on all IPv6 addresses.


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

  Library reloading can be forced anytime by sending a SIGHUP signal to the
  ``kiwix-serve`` process (this works regardless of the presence of the
  :option:`--monitorLibrary`/:option:`-M` option).


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

.. note::

  The HTTP API is documented in its entirety in order to facilitate the work of
  the Kiwix development team. Note, however, that only a subset of the HTTP API
  constitutes ``kiwix-serves``'s public interface.

  .. _public-api-endpoint:

  **Public API endpoint**

    A public HTTP API endpoint is intended to serve the outside world (in
    addition to ``kiwix-serve``'s front-end and other Kiwix products). The
    Kiwix development team will do its best to ensure gratifying experience for
    clients of public API endpoints at all stages of the endpoint lifecycle.

  .. _private-api-endpoint:

  **Private API endpoint**

    A private API endpoint is intended to be used only by ``kiwix-serve``'s
    frontend or by other products maintained solely by the Kiwix team. Private
    API comes without any guaranees. It may change as frequently and as
    drasticaly as the Kiwix development team sees fit.

  .. _deprecation:

  **Deprecation**

    Public API doesn't stay frozen once and forever. As the API evolves, Kiwix
    team reserves the right to drop support for certain old functionality. In
    such events, an advance notice will be issued and the users will be given
    enough time to prepare for the change.

  Currently, public endpoints are limited to the following list:

    -  :ref:`OPDS API <new-opds-api>`
    -  ``/raw``
    -  ``/search`` (with ``/search/searchdescription.xml``)

.. _welcome-page:

``/``
-----

===== ===========
Type: :ref:`private <private-api-endpoint>`
===== ===========

Welcome page is served under ``/``. By default this is the library page, where
books are listed and can be looked up/filtered interactively. However, the
welcome page can be overriden through the :option:`--customIndex`/:option:`-c`
command line option of ``kiwix-serve``.


.. _new-opds-api:

``/catalog/v2`` (OPDS API)
------------------------------

===== ===========
Type: :ref:`public <public-api-endpoint>`
===== ===========

The new OPDS API of ``kiwix-serve`` is based on the `OPDS Catalog specification
v1.2 <https://specs.opds.io/opds-1.2>`_. All of its endpoints are grouped under
``/catalog/v2``.

:ref:`Legacy OPDS API <legacy-opds-api>` is preserved for backward
compatibility.


``/catalog/v2/root.xml``
^^^^^^^^^^^^^^^^^^^^^^^^

===== ===========
Type: member of a :ref:`public API <new-opds-api>`
===== ===========

The OPDS Catalog Root links to the OPDS acquisition and navigation feeds
accessible through the other endpoints of the OPDS API.


``/catalog/v2/searchdescription.xml``
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

===== ===========
Type: member of a :ref:`public API <new-opds-api>`
===== ===========

Describes the `/catalog/v2/entries`_ endpoint in `OpenSearch description format
<https://developer.mozilla.org/en-US/docs/Web/OpenSearch>`_.



``/catalog/v2/categories``
^^^^^^^^^^^^^^^^^^^^^^^^^^

===== ===========
Type: member of a :ref:`public API <new-opds-api>`
===== ===========

Returns the full list of ZIM file categories as an `OPDS Navigation Feed
<https://specs.opds.io/opds-1.2#22-navigation-feeds>`_.


``/catalog/v2/entries``
^^^^^^^^^^^^^^^^^^^^^^^

===== ===========
Type: member of a :ref:`public API <new-opds-api>`
===== ===========

Returns a full or filtered list of ZIM files as a paginated `OPDS acquisition
feed <https://specs.opds.io/opds-1.2#23-acquisition-feeds>`_ with `complete
entries
<https://specs.opds.io/opds-1.2#512-partial-and-complete-catalog-entries>`_.

**Pagination:**

By default, no more than 10 first entries are returned from the library. To
obtain the remaining entries the URL query parameters ``start`` and/or
``count`` must be used. The output of ``/catalog/v2/entries?start=s&count=n``
will contain at most ``n`` (default value: 10) results starting from entry #
``s`` (default value: 0).  ``count`` with a negative value (e.g.  ``count=-1``)
removes the limit on the number of results in the output.


.. note::

  Previously ``count=0`` also designated an unbounded query (i.e. worked
  similarly to ``count=-1``). The response to a ``count=0`` query was changed
  to consist of 0 results, as such a query/response combination is a good way
  to find out the total number of results (when only that information is
  needed) with minimal consumption of resources.

Examples:

.. code:: sh

  # Returns the first 10 entries (internally numbered 0 through 9)
  $ curl 'http://localhost:8080/catalog/v2/entries'

  # Returns the next 10 entries (internally numbered 10 through 19)
  $ curl 'http://localhost:8080/catalog/v2/entries?start=10'

  # Returns the first 50 entries
  $ curl 'http://localhost:8080/catalog/v2/entries?count=50'

  # Returns 50 entries starting from entry # 100 (i.e. entries ## 100-149)
  $ curl 'http://localhost:8080/catalog/v2/entries?start=100&count=50'

  # Returns all entries
  $ curl 'http://localhost:8080/catalog/v2/entries?count=-1'

  # Returns all entries starting from entry # 100
  $ curl 'http://localhost:8080/catalog/v2/entries?start=100&count=-1'


.. _library-filtering:

**Filtering:**

A filtered subset of the library can be requested by providing one or more
filtering criteria, whereupon only entries matching *all* of the criteria are
included in the response. Pagination is applied to the filtered list. The
filtering criteria must be specified via the following URL parameters:

* ``lang`` - filter by language (specified as a 3-letter language code).

* ``category`` - filter by categories associated with the library entries.

* ``tag`` - filter by tags associated with the library entries. Multiple tags
  can be provided as a semicolon separated list (e.g
  ``tag=wikipedia;_videos:no``). The result will contain only those entries
  that contain *all* of the requested tags.

* ``notag`` - filter out (exclude) entries with *any* of the specified tags
  (example - ``notag=ted;youtube``).

* ``maxsize`` - include in the results only entries whose size (in bytes)
  doesn't exceed the provided value.

* ``q`` - include in the results only entries that contain the specified text
  in the title or description.

* ``name`` - include in the results only entries with a matching
  :term:`book name <Book name>`.


Examples:

.. code:: sh

  # List only books in Italian (lang=ita) but
  # return only results ## 100-149 (start=100&count=50)
  $ curl 'http://localhost:8080/catalog/v2/entries?lang=ita&start=100&count=50'

  # List only books with category of 'wikipedia' AND containing the word
  # 'science' in the title or description. Return only the first 10 results.
  $ curl 'http://localhost:8080/catalog/v2/entries?q=science&category=wikipedia'


``/catalog/v2/entry/ZIMID``
^^^^^^^^^^^^^^^^^^^^^^^^^^^

===== ===========
Type: member of a :ref:`public API <new-opds-api>`
===== ===========

Returns full info about the library entry with :term:`UUID <ZIM UUID>`
``ZIMID``.


``/catalog/v2/illustration/ZIMID``
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

===== ===========
Type: member of a :ref:`public API <new-opds-api>`
===== ===========

**Usage:**

  ``/catalog/v2/illustration/ZIMID?size=N``

Returns the illustration of size ``NxN`` pixels for the library entry with
:term:`UUID <ZIM UUID>` ``ZIMID``.

If no illustration of requested size is found a HTTP 404 error is returned.


``/catalog/v2/languages``
^^^^^^^^^^^^^^^^^^^^^^^^^

===== ===========
Type: member of a :ref:`public API <new-opds-api>`
===== ===========

Returns the full list of ZIM file languages as an `OPDS Navigation Feed
<https://specs.opds.io/opds-1.2#22-navigation-feeds>`_.


``/catalog/v2/partial_entries``
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

===== ===========
Type: member of a :ref:`public API <new-opds-api>`
===== ===========

Returns the full or filtered list of ZIM files as an `OPDS acquisition feed
<https://specs.opds.io/opds-1.2#23-acquisition-feeds>`_ with `partial entries
<https://specs.opds.io/opds-1.2#512-partial-and-complete-catalog-entries>`_.

Supported filters are the same as for the `/catalog/v2/entries`_ endpoint.


.. _legacy-opds-api:

``/catalog`` (Legacy OPDS API)
------------------------------

===== ===========
Type: :ref:`deprecated <deprecation>`
===== ===========

The legacy OPDS API is preserved for backward compatibility and is deprecated.
:ref:`New OPDS API <new-opds-api>` should be used instead.


``/catalog/root.xml``
^^^^^^^^^^^^^^^^^^^^^

===== ===========
Type: member of a :ref:`deprecated API <legacy-opds-api>`
===== ===========

Full library OPDS catalog (list of all ZIM files).


``/catalog/searchdescription.xml``
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

===== ===========
Type: member of a :ref:`deprecated API <legacy-opds-api>`
===== ===========

Describes the `/catalog/search`_ endpoint in `OpenSearch description format
<https://developer.mozilla.org/en-US/docs/Web/OpenSearch>`_.


``/catalog/search``
^^^^^^^^^^^^^^^^^^^

===== ===========
Type: member of a :ref:`deprecated API <legacy-opds-api>`
===== ===========

Returns the list of ZIM files (in OPDS catalog format) matching the
search/filtering criteria. Supported filters are the same as for the
`/catalog/v2/entries`_ endpoint.


``/catch/external``
-------------------

===== ===========
Type: :ref:`private <private-api-endpoint>`
===== ===========

**Usage:**

  ``/catch/external?source=URL``

Generates a HTML page with a link leading to (the decoded version of) ``URL``
and a warning that following that link will load an external (out of ZIM)
resource.

**Parameters:**

  ``source``: URL of the external resource (must be URL-encoded).

**Example:**

.. code:: sh

  # Intercept an external link to https://example.com?query=abcd
  $ curl 'http://localhost:8080/catch/external?source=https%3A%2F%2Fexample.com%3Fquery%3Dabcd'



``/content``
------------

===== ===========
Type: :ref:`private <private-api-endpoint>`
===== ===========

ZIM file content is served under the ``/content`` endpoint as described below.


``/content/ZIMNAME/PATH/IN/ZIMFILE``
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

===== ===========
Type: :ref:`private <private-api-endpoint>`
===== ===========

Returns the entry with path ``PATH/IN/ZIMFILE`` from ZIM file with :term:`name
<ZIM name>` ``ZIMNAME``.


``/content/ZIMNAME``
^^^^^^^^^^^^^^^^^^^^

===== ===========
Type: :ref:`private <private-api-endpoint>`
===== ===========

``/content/ZIMNAME`` redirects to the main page of the ZIM file with :term:`name
<ZIM name>` ``ZIMNAME`` (unless that ZIM file contains an entry with an empty
path or path equal to ``/``, in which case that entry is returned).


``/random``
-----------

===== ===========
Type: :ref:`private <private-api-endpoint>`
===== ===========

**Usage:**

  ``/random?content=ZIMNAME``

Generates a HTTP redirect to a randomly selected article/page from the
specified ZIM file.

**Parameters:**

  ``content``: :term:`name of the ZIM file <ZIM name>`.


.. _raw:

``/raw``
--------

===== ===========
Type: :ref:`public <public-api-endpoint>`
===== ===========

The ``/raw`` API provides access to ZIM file data. It consists of two separate
endpoints for accessing data and metadata.


``/raw/ZIMNAME/content/PATH/IN/ZIMFILE``
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

===== ===========
Type: member of a :ref:`public API <raw>`
===== ===========

Returns the entry with path ``PATH/IN/ZIMFILE`` from the ZIM file with
:term:`name <ZIM name>` ``ZIMNAME``. Currently, this endpoint almost duplicates
(with some subtle technical differences) the newer endpoint
`/content/ZIMNAME/PATH/IN/ZIMFILE`_. The important difference is that the
``/raw`` endpoint guarantees that no server-side processing will be applied to
the returned content, whereas content obtained via the ``/content`` endpoint
may in the future undergo some processing intended to improve the operation of
the viewer (e.g. compensating for certain bugs in ZIM creation). Also note that
``/raw`` is :ref:`public <public-api-endpoint>`, whereas ``/content`` is
:ref:`private <private-api-endpoint>`.


``/raw/ZIMNAME/meta/METADATAID``
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

===== ===========
Type: member of a :ref:`public API <raw>`
===== ===========

Returns the metadata item ``METADATAID`` from the ZIM file with :term:`name
<ZIM name>` ``ZIMNAME``.


``/search``
-----------

===== ===========
Type: :ref:`public <public-api-endpoint>`
===== ===========

Performs a full text search on one or more ZIM files and returns an HTML page
with a list of links to matching pages along with snippets of the matching
portions of those pages.

.. _multi-zim-search-constraints:

A multi-ZIM search request must comply with the following constraints:

* the number of ZIM files participating in the search operation must not exceed
  the limit imposed by the :option:`--searchLimit` option of ``kiwix-serve``.

* all of the ZIM files participating in the same search operation must be in
  the same language.

**Parameters:**


  ZIM file selection parameters:

    At least one of the following parameters must be provided in order to
    specify in which ZIM file(s) to search. Parameters appearing earlier in
    below list take precedence over subsequent ones (the later ones, even if
    present in the request, are simply ignored).

    ``content``: :term:`name of the ZIM file <ZIM name>` (for a single-ZIM
    search). This is a :ref:`legacy parameter <deprecation>`. ``books.name``
    should be used instead.

    ``books.id``: :term:`UUID <ZIM UUID>` of the ZIM file. Can be repeated for
    a multi-ZIM search, however must respect the :ref:`multi-ZIM search
    constraints <multi-zim-search-constraints>`.

      .. note::

        If any of the provided ``books.id`` values refers to a book missing
        from the library then an error is returned instead of running the
        search on the remaining (valid) entries.

    ``books.name``: :term:`name of the ZIM file <ZIM name>` (not to be confused
    with ``books.filter.name`` which selects/filters based on the :term:`book
    name <Book name>`). Can be repeated for a multi-ZIM search, however must
    respect the :ref:`multi-ZIM search constraints
    <multi-zim-search-constraints>`.

      .. note::

        If any of the provided ``books.name`` values refers to a book missing
        from the library then an error is returned instead of running the
        search on the remaining (valid) entries.

    ``books.filter.{criteria}``: allows to take full advantage of :ref:`library
    filtering <library-filtering>` functionality of the `/catalog/v2/entries`_
    endpoint (``{criteria}`` must be replaced with an attribute/filtering
    criteria name supported by :ref:`library filtering <library-filtering>`).

  Query parameters:

    ``pattern`` (optional; defaults to an empty string): text to search for.

    ``latitude``, ``longitude`` & ``distance`` (optional): geospatial query
    parameters. If all of these are provided, then the results will be
    restricted to geotagged pages that are within ``distance`` metres from the
    location on Earth with coordinates ``latitude`` and ``longitude``.

  Pagination parameters:

    ``pageLength`` (optional, default: 25): maximum number of search results in
    the response. Capped at 140.

    ``start`` (optional, default: 1): this parameter enables pagination of
    results. The response will include up to ``pageLength`` results starting
    with entry # ``start`` from the full list of search results (the first
    result is assumed to have index 1).

  Other parameters:

    ``format`` (optional, default: html): format of the search results. Allowed
    values are: html, xml.

Examples:

.. code:: sh

  # Search for 'android' in the book with name 'scifi-library'
  # Return results ## 51-60.
  $ curl 'http://localhost:8080/search?pattern=android&books.name=scifi-library&start=51&pageLength=10'

  # Search for 'napoli' in books in Italian
  $ curl 'http://localhost:8080/search?books.filter.lang=ita&pattern=napoli'

  # Search for 'chateau' in books in French that have a category of 'wikipedia'.
  # Return the results as XML.
  $ curl 'http://localhost:8080/search?pattern=chateau&books.filter.lang=fra&books.filter.category=wikipedia&format=xml'


``/search/searchdescription.xml``
---------------------------------

===== ===========
Type: :ref:`public <public-api-endpoint>`
===== ===========

Describes the `/search`_ endpoint in `OpenSearch description format
<https://developer.mozilla.org/en-US/docs/Web/OpenSearch>`_.



``/skin``
-----------

===== ===========
Type: :ref:`private <private-api-endpoint>`
===== ===========

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

===== ===========
Type: :ref:`private <private-api-endpoint>`
===== ===========

**Usage:**

  ``/suggest?content=ZIMNAME[&term=QUERY][&count=N][&start=S]``

Returns suggestions (in JSON format) for a text string that is assumed to be a
partially typed search for a page inside a particular ZIM file.

Suggestions are obtained as matches of the query text against the page titles
in the ZIM file using the title index database generated during the creation of
the ZIM file.

In case of a multi-word query the order of the words matters in two ways:

1. the last word is considered as partially typed, unless followed by a space;
2. ranking of the matches.

If the ZIM file doesn't contain a title index then suggestions are generated by
listing page titles starting *exactly* (in a case sensitive manner) with the
query text. Otherwise, suggestions are case-insensitive.

If the ZIM file contains a full text search index, then an extra suggestion is
added as an option to perform a full text search in the said ZIM file.

**Parameters:**

  ``content`` (mandatory): :term:`name of the ZIM file <ZIM name>`.

  ``term`` (optional; defaults to an empty string): query text.

  ``count`` (optional, default: 10): maximum number of page suggestions in the
  response (i.e. the extra option to perform a full text search is not included
  in this count).

  ``start`` (optional, default: 0): this parameter enables pagination of
  results. The response will include up to ``count`` entries starting with
  entry # ``start`` from the full list of page suggestions (the first result is
  assumed to have index 0).

**Example:**

.. code:: sh

  $ curl 'http://localhost/suggest?content=stackoverflow_en&term=pyth&count=50'


.. _zim-file-viewer:

``/viewer``
-----------

===== ===========
Type: :ref:`private <private-api-endpoint>`
===== ===========

ZIM file viewer. The ZIM file and entry therein must be specified via the hash
component of the URL as ``/viewer#ZIMNAME/PATH/IN/ZIMFILE``.


``/viewer_settings.js``
-----------------------

===== ===========
Type: :ref:`private <private-api-endpoint>`
===== ===========

Settings of the ZIM file viewer that are configurable via certain command line
options of ``kiwix-serve`` (e.g. ``--nolibrarybutton``).


/ANYTHING/ELSE
--------------

===== ===========
Type: :ref:`private <private-api-endpoint>`
===== ===========

Any other URL is considered as an attempt to access ZIM file content using the
legacy URL scheme and is redirected to ``/content/ANYTHING/ELSE``.


Glossary
========

.. glossary::

  Book name

    Name of the book as specified in the ZIM file metadata (for a
    ``kiwix-serve`` started *WITHOUT* the :option:`--library` option) or the
    library XML file (for a ``kiwix-serve`` started with the
    :option:`--library` option).

    .. note::

      Two or more books may have the same name in the library. That's not
      considered a conflict, because there may be multiple versions of the
      "same" book (differing by the settings of the scraper, date, etc).
      :ref:`Library filtering <library-filtering>` by name will return all
      matching books.

  ZIM filename

    Name of a ZIM file on the server filesystem.

  ZIM name

    Identifier of a ZIM file in the server's library (used for referring to a
    particular ZIM file in requests).

    ZIM names are derived from the filenames as follows:

    - file extension is removed,
    - all characters are converted to lowercase,
    - diacritics are removed,
    - spaces are replaced with underscores,
    - ``+`` symbols are replaced with the text ``plus``.

    Presence of the :option:`-z`/:option:`--nodatealiases` option will create
    additional names (aliases) for filenames with dates.

    ZIM names are expected to be unique across the library. Any name conflicts
    (including those caused by the usage of the
    :option:`-z`/:option:`--nodatealiases` option) are reported on STDERR but,
    otherwise, are ignored (i.e. only one of the entries can be accessed via
    the conflicting name).

  ZIM title

    Title of a ZIM file. This can be any text (with whitespace). It is never
    used as a way of referring to a ZIM file.

  ZIM UUID

    This is a unique identifier of a ZIM file designated at its creation time
    and embedded in the ZIM file. Certain ``kiwix-serve`` operations may
    require that a ZIM file be referenced through its UUID rather than name.
