#ifndef KIWIX_FFI_H
#define KIWIX_FFI_H

/*
 * kiwix_ffi.h — C89-compatible FFI interface for libkiwix/libzim.
 *
 * All strings returned by these functions are owned by the caller
 * and must be freed with kiwix_free_string().
 * All opaque handles must be freed with their respective _free() function.
 *
 * Error handling: functions return NULL or 0 on error.
 * Call kiwix_last_error() to get the last error message.
 */

#ifdef _WIN32
#  ifdef KIWIX_FFI_BUILDING
#    define KIWIX_FFI_API __declspec(dllexport)
#  else
#    define KIWIX_FFI_API __declspec(dllimport)
#  endif
#else
#  define KIWIX_FFI_API __attribute__((visibility("default")))
#endif

#ifdef __cplusplus
extern "C" {
#endif

/* ========================================================================
 * Common
 * ======================================================================== */

/** Free a string returned by any kiwix FFI function. */
KIWIX_FFI_API void kiwix_free_string(char* s);

/** Get the last error message (thread-local). Returns NULL if no error. */
KIWIX_FFI_API const char* kiwix_last_error(void);

/* ========================================================================
 * Library
 * ======================================================================== */

typedef struct kiwix_library_t kiwix_library_t;

/** Create a new empty library. */
KIWIX_FFI_API kiwix_library_t* kiwix_library_new(void);

/** Free a library. */
KIWIX_FFI_API void kiwix_library_free(kiwix_library_t* lib);

/** Load a library from an XML file. Returns 1 on success, 0 on error. */
KIWIX_FFI_API int kiwix_library_load(kiwix_library_t* lib, const char* xml_path);

/** Save the library to an XML file. Returns 1 on success, 0 on error. */
KIWIX_FFI_API int kiwix_library_save(kiwix_library_t* lib, const char* xml_path);

/** Add a ZIM file to the library. Returns the book ID (caller frees) or NULL on error. */
KIWIX_FFI_API char* kiwix_library_add_file(kiwix_library_t* lib, const char* zim_path);

/** Remove a book by ID. Returns 1 on success, 0 on error. */
KIWIX_FFI_API int kiwix_library_remove_book(kiwix_library_t* lib, const char* book_id);

/** Get the number of books in the library. */
KIWIX_FFI_API unsigned int kiwix_library_book_count(kiwix_library_t* lib);

/** Get a list of all book IDs. Returns a NULL-terminated array of strings.
 *  Free with kiwix_free_string_array(). */
KIWIX_FFI_API char** kiwix_library_get_book_ids(kiwix_library_t* lib);

/** Free a string array returned by kiwix_library_get_book_ids(). */
KIWIX_FFI_API void kiwix_free_string_array(char** arr);

/* ========================================================================
 * Book metadata
 * ======================================================================== */

/** Get book title. Returns NULL if book_id not found. Caller frees. */
KIWIX_FFI_API char* kiwix_library_get_book_title(kiwix_library_t* lib, const char* book_id);

/** Get book description. Caller frees. */
KIWIX_FFI_API char* kiwix_library_get_book_description(kiwix_library_t* lib, const char* book_id);

/** Get book path. Caller frees. */
KIWIX_FFI_API char* kiwix_library_get_book_path(kiwix_library_t* lib, const char* book_id);

/** Get book name (identifier). Caller frees. */
KIWIX_FFI_API char* kiwix_library_get_book_name(kiwix_library_t* lib, const char* book_id);

/** Get book date. Caller frees. */
KIWIX_FFI_API char* kiwix_library_get_book_date(kiwix_library_t* lib, const char* book_id);

/** Get book creator. Caller frees. */
KIWIX_FFI_API char* kiwix_library_get_book_creator(kiwix_library_t* lib, const char* book_id);

/** Get book language. Caller frees. */
KIWIX_FFI_API char* kiwix_library_get_book_language(kiwix_library_t* lib, const char* book_id);

/** Get book article count. */
KIWIX_FFI_API unsigned int kiwix_library_get_book_article_count(kiwix_library_t* lib, const char* book_id);

/** Get book media count. */
KIWIX_FFI_API unsigned int kiwix_library_get_book_media_count(kiwix_library_t* lib, const char* book_id);

/** Get book size in KB. */
KIWIX_FFI_API unsigned int kiwix_library_get_book_size(kiwix_library_t* lib, const char* book_id);

/* ========================================================================
 * Archive (ZIM file access)
 * ======================================================================== */

typedef struct kiwix_archive_t kiwix_archive_t;

/** Open a ZIM archive from a file path. Returns NULL on error. */
KIWIX_FFI_API kiwix_archive_t* kiwix_archive_open(const char* zim_path);

/** Free an archive. */
KIWIX_FFI_API void kiwix_archive_free(kiwix_archive_t* archive);

/** Get the archive filename. Caller frees. */
KIWIX_FFI_API char* kiwix_archive_get_filename(kiwix_archive_t* archive);

/** Get the archive title. Caller frees. */
KIWIX_FFI_API char* kiwix_archive_get_title(kiwix_archive_t* archive);

/** Get article count in the archive. */
KIWIX_FFI_API unsigned int kiwix_archive_article_count(kiwix_archive_t* archive);

/** Check if the archive has a fulltext search index. */
KIWIX_FFI_API int kiwix_archive_has_fulltext_index(kiwix_archive_t* archive);

/** Check if the archive has a title index. */
KIWIX_FFI_API int kiwix_archive_has_title_index(kiwix_archive_t* archive);

/* ========================================================================
 * Article access
 * ======================================================================== */

/** Get article content by path. Returns content string (caller frees) or NULL. */
KIWIX_FFI_API char* kiwix_archive_get_article_content(kiwix_archive_t* archive, const char* path);

/** Get article MIME type by path. Caller frees. Returns NULL if not found. */
KIWIX_FFI_API char* kiwix_archive_get_article_mimetype(kiwix_archive_t* archive, const char* path);

/** Get article content length by path. Returns -1 if not found. */
KIWIX_FFI_API long kiwix_archive_get_article_size(kiwix_archive_t* archive, const char* path);

/* ========================================================================
 * Search
 * ======================================================================== */

typedef struct kiwix_searcher_t kiwix_searcher_t;
typedef struct kiwix_search_result_t kiwix_search_result_t;

/** Full-text search in an archive. Returns results or NULL on error.
 *  limit: max number of results (0 = default of 10). */
KIWIX_FFI_API kiwix_search_result_t* kiwix_archive_search(kiwix_archive_t* archive,
                                                           const char* query,
                                                           int limit);

/** Title suggestion search. Returns results or NULL on error. */
KIWIX_FFI_API kiwix_search_result_t* kiwix_archive_suggest(kiwix_archive_t* archive,
                                                            const char* prefix,
                                                            int limit);

/** Free search results. */
KIWIX_FFI_API void kiwix_search_result_free(kiwix_search_result_t* results);

/** Get number of results. */
KIWIX_FFI_API int kiwix_search_result_count(kiwix_search_result_t* results);

/** Get result title at index. Caller frees. Returns NULL if out of bounds. */
KIWIX_FFI_API char* kiwix_search_result_get_title(kiwix_search_result_t* results, int index);

/** Get result path at index. Caller frees. Returns NULL if out of bounds. */
KIWIX_FFI_API char* kiwix_search_result_get_path(kiwix_search_result_t* results, int index);

/** Get result snippet at index. Caller frees. Returns NULL if no snippet. */
KIWIX_FFI_API char* kiwix_search_result_get_snippet(kiwix_search_result_t* results, int index);

/* ========================================================================
 * Version
 * ======================================================================== */

/** Get version string. Caller frees. */
KIWIX_FFI_API char* kiwix_get_version(void);


#ifdef __cplusplus
}
#endif

#endif /* KIWIX_FFI_H */
