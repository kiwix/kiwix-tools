/*
 * kiwix_ffi.cpp — C FFI implementation wrapping libkiwix/libzim.
 *
 * All C++ exceptions are caught at the boundary and stored in a thread-local
 * error string. C callers get NULL/0 on error and can call kiwix_last_error().
 */

#include "kiwix_ffi.h"

#include <kiwix/library.h>
#include <kiwix/manager.h>
#include <kiwix/book.h>
#include <kiwix/tools.h>
#include <zim/archive.h>
#include <zim/entry.h>
#include <zim/item.h>
#include <zim/error.h>
#include <zim/search.h>
#include <zim/suggestion.h>
#include <zim/version.h>

#include <cstring>
#include <cstdlib>
#include <string>
#include <vector>

/* ========================================================================
 * Thread-local error storage
 * ======================================================================== */

static thread_local std::string t_last_error;

static void set_error(const std::string& msg) {
    t_last_error = msg;
}

static void clear_error() {
    t_last_error.clear();
}

KIWIX_FFI_API const char* kiwix_last_error(void) {
    return t_last_error.empty() ? nullptr : t_last_error.c_str();
}

KIWIX_FFI_API void kiwix_free_string(char* s) {
    std::free(s);
}

KIWIX_FFI_API void kiwix_free_string_array(char** arr) {
    if (!arr) return;
    for (char** p = arr; *p; ++p) {
        std::free(*p);
    }
    std::free(arr);
}

/** Helper: duplicate a std::string to a malloc'd C string. */
static char* dup_string(const std::string& s) {
    char* result = (char*)std::malloc(s.size() + 1);
    if (result) {
        std::memcpy(result, s.c_str(), s.size() + 1);
    }
    return result;
}

/* ========================================================================
 * Library
 * ======================================================================== */

struct kiwix_library_t {
    kiwix::LibraryPtr ptr;
};

KIWIX_FFI_API kiwix_library_t* kiwix_library_new(void) {
    clear_error();
    try {
        auto lib = new kiwix_library_t;
        lib->ptr = kiwix::Library::create();
        return lib;
    } catch (const std::exception& e) {
        set_error(e.what());
        return nullptr;
    }
}

KIWIX_FFI_API void kiwix_library_free(kiwix_library_t* lib) {
    delete lib;
}

KIWIX_FFI_API int kiwix_library_load(kiwix_library_t* lib, const char* xml_path) {
    if (!lib || !xml_path) { set_error("null argument"); return 0; }
    clear_error();
    try {
        kiwix::Manager manager(lib->ptr);
        if (!manager.readFile(xml_path, false)) {
            set_error("Cannot read library file: " + std::string(xml_path));
            return 0;
        }
        return 1;
    } catch (const std::exception& e) {
        set_error(e.what());
        return 0;
    }
}

KIWIX_FFI_API int kiwix_library_save(kiwix_library_t* lib, const char* xml_path) {
    if (!lib || !xml_path) { set_error("null argument"); return 0; }
    clear_error();
    try {
        if (!lib->ptr->writeToFile(xml_path)) {
            set_error("Cannot write library file: " + std::string(xml_path));
            return 0;
        }
        return 1;
    } catch (const std::exception& e) {
        set_error(e.what());
        return 0;
    }
}

KIWIX_FFI_API char* kiwix_library_add_file(kiwix_library_t* lib, const char* zim_path) {
    if (!lib || !zim_path) { set_error("null argument"); return nullptr; }
    clear_error();
    try {
        kiwix::Manager manager(lib->ptr);
        auto id = manager.addBookFromPathAndGetId(zim_path, zim_path, "", false);
        if (id.empty()) {
            set_error("Cannot add ZIM file: " + std::string(zim_path));
            return nullptr;
        }
        return dup_string(id);
    } catch (const std::exception& e) {
        set_error(e.what());
        return nullptr;
    }
}

KIWIX_FFI_API int kiwix_library_remove_book(kiwix_library_t* lib, const char* book_id) {
    if (!lib || !book_id) { set_error("null argument"); return 0; }
    clear_error();
    try {
        if (!lib->ptr->removeBookById(book_id)) {
            set_error("Invalid book id: " + std::string(book_id));
            return 0;
        }
        return 1;
    } catch (const std::exception& e) {
        set_error(e.what());
        return 0;
    }
}

KIWIX_FFI_API unsigned int kiwix_library_book_count(kiwix_library_t* lib) {
    if (!lib) return 0;
    return lib->ptr->getBookCount(true, true);
}

KIWIX_FFI_API char** kiwix_library_get_book_ids(kiwix_library_t* lib) {
    if (!lib) return nullptr;
    clear_error();
    try {
        auto ids = lib->ptr->getBooksIds();
        // Allocate array of char* with NULL sentinel
        char** arr = (char**)std::malloc((ids.size() + 1) * sizeof(char*));
        if (!arr) return nullptr;
        for (size_t i = 0; i < ids.size(); ++i) {
            arr[i] = dup_string(ids[i]);
        }
        arr[ids.size()] = nullptr;
        return arr;
    } catch (const std::exception& e) {
        set_error(e.what());
        return nullptr;
    }
}

/* ========================================================================
 * Book metadata helpers
 * ======================================================================== */

#define BOOK_GETTER(name, expr) \
KIWIX_FFI_API char* kiwix_library_get_book_##name(kiwix_library_t* lib, const char* book_id) { \
    if (!lib || !book_id) { set_error("null argument"); return nullptr; } \
    clear_error(); \
    try { \
        auto& book = lib->ptr->getBookById(book_id); \
        return dup_string(expr); \
    } catch (const std::out_of_range&) { \
        set_error("Book not found: " + std::string(book_id)); \
        return nullptr; \
    } catch (const std::exception& e) { \
        set_error(e.what()); \
        return nullptr; \
    } \
}

BOOK_GETTER(title, book.getTitle())
BOOK_GETTER(description, book.getDescription())
BOOK_GETTER(path, book.getPath())
BOOK_GETTER(name, book.getName())
BOOK_GETTER(date, book.getDate())
BOOK_GETTER(creator, book.getCreator())
BOOK_GETTER(language, book.getLanguage())
BOOK_GETTER(tags, book.getTags())

KIWIX_FFI_API unsigned int kiwix_library_get_book_article_count(kiwix_library_t* lib, const char* book_id) {
    if (!lib || !book_id) return 0;
    try {
        return lib->ptr->getBookById(book_id).getArticleCount();
    } catch (...) { return 0; }
}

KIWIX_FFI_API unsigned int kiwix_library_get_book_media_count(kiwix_library_t* lib, const char* book_id) {
    if (!lib || !book_id) return 0;
    try {
        return lib->ptr->getBookById(book_id).getMediaCount();
    } catch (...) { return 0; }
}

KIWIX_FFI_API unsigned int kiwix_library_get_book_size(kiwix_library_t* lib, const char* book_id) {
    if (!lib || !book_id) return 0;
    try {
        return lib->ptr->getBookById(book_id).getSize();
    } catch (...) { return 0; }
}

/* ========================================================================
 * Archive
 * ======================================================================== */

struct kiwix_archive_t {
    zim::Archive archive;
    kiwix_archive_t(const std::string& path) : archive(path) {}
};

KIWIX_FFI_API kiwix_archive_t* kiwix_archive_open(const char* zim_path) {
    if (!zim_path) { set_error("null argument"); return nullptr; }
    clear_error();
    try {
        return new kiwix_archive_t(zim_path);
    } catch (const std::exception& e) {
        set_error(e.what());
        return nullptr;
    }
}

KIWIX_FFI_API void kiwix_archive_free(kiwix_archive_t* archive) {
    delete archive;
}

KIWIX_FFI_API char* kiwix_archive_get_filename(kiwix_archive_t* archive) {
    if (!archive) return nullptr;
    return dup_string(archive->archive.getFilename());
}

KIWIX_FFI_API char* kiwix_archive_get_title(kiwix_archive_t* archive) {
    if (!archive) return nullptr;
    try {
        return dup_string(archive->archive.getMetadata("Title"));
    } catch (...) {
        return dup_string("");
    }
}

KIWIX_FFI_API unsigned int kiwix_archive_article_count(kiwix_archive_t* archive) {
    if (!archive) return 0;
    return archive->archive.getArticleCount();
}

KIWIX_FFI_API int kiwix_archive_has_fulltext_index(kiwix_archive_t* archive) {
    if (!archive) return 0;
    return archive->archive.hasFulltextIndex() ? 1 : 0;
}

KIWIX_FFI_API int kiwix_archive_has_title_index(kiwix_archive_t* archive) {
    if (!archive) return 0;
    return archive->archive.hasTitleIndex() ? 1 : 0;
}

/* ========================================================================
 * Article access
 * ======================================================================== */

KIWIX_FFI_API char* kiwix_archive_get_article_content(kiwix_archive_t* archive, const char* path) {
    if (!archive || !path) { set_error("null argument"); return nullptr; }
    clear_error();
    try {
        auto entry = archive->archive.getEntryByPath(path);
        auto item = entry.getItem(true);
        auto blob = item.getData();
        auto sz = blob.size();
        char* result = (char*)std::malloc(sz + 1);
        if (result) {
            std::memcpy(result, blob.data(), sz);
            result[sz] = '\0';
        }
        return result;
    } catch (const zim::EntryNotFound&) {
        set_error("Article not found: " + std::string(path));
        return nullptr;
    } catch (const std::exception& e) {
        set_error(e.what());
        return nullptr;
    }
}

KIWIX_FFI_API char* kiwix_archive_get_article_mimetype(kiwix_archive_t* archive, const char* path) {
    if (!archive || !path) { set_error("null argument"); return nullptr; }
    clear_error();
    try {
        auto entry = archive->archive.getEntryByPath(path);
        auto item = entry.getItem(true);
        return dup_string(item.getMimetype());
    } catch (const zim::EntryNotFound&) {
        set_error("Article not found: " + std::string(path));
        return nullptr;
    } catch (const std::exception& e) {
        set_error(e.what());
        return nullptr;
    }
}

KIWIX_FFI_API long kiwix_archive_get_article_size(kiwix_archive_t* archive, const char* path) {
    if (!archive || !path) return -1;
    clear_error();
    try {
        auto entry = archive->archive.getEntryByPath(path);
        auto item = entry.getItem(true);
        return (long)item.getSize();
    } catch (...) {
        return -1;
    }
}

/* ========================================================================
 * Search
 * ======================================================================== */

struct kiwix_search_result_t {
    struct Entry {
        std::string title;
        std::string path;
        std::string snippet;
    };
    std::vector<Entry> entries;
};

KIWIX_FFI_API kiwix_search_result_t* kiwix_archive_search(kiwix_archive_t* archive,
                                                           const char* query,
                                                           int limit) {
    if (!archive || !query) { set_error("null argument"); return nullptr; }
    clear_error();
    try {
        if (limit <= 0) limit = 10;
        zim::Searcher searcher(archive->archive);
        const zim::Query q(query);
        auto search_obj = searcher.search(q);
        auto results = search_obj.getResults(0, limit);

        auto out = new kiwix_search_result_t;
        for (auto it = results.begin(); it != results.end(); ++it) {
            kiwix_search_result_t::Entry e;
            e.title = it.getTitle();
            e.path = it.getPath();
            e.snippet = it.getSnippet();
            out->entries.push_back(std::move(e));
        }
        return out;
    } catch (const std::exception& e) {
        set_error(e.what());
        return nullptr;
    }
}

KIWIX_FFI_API kiwix_search_result_t* kiwix_archive_suggest(kiwix_archive_t* archive,
                                                            const char* prefix,
                                                            int limit) {
    if (!archive || !prefix) { set_error("null argument"); return nullptr; }
    clear_error();
    try {
        if (limit <= 0) limit = 10;
        zim::SuggestionSearcher searcher(archive->archive);
        auto suggest = searcher.suggest(prefix);
        auto results = suggest.getResults(0, limit);

        auto out = new kiwix_search_result_t;
        for (auto it = results.begin(); it != results.end(); ++it) {
            const zim::SuggestionItem& item = *it;
            kiwix_search_result_t::Entry e;
            e.title = item.getTitle();
            e.path = item.getPath();
            e.snippet = item.getSnippet();
            out->entries.push_back(std::move(e));
        }
        return out;
    } catch (const std::exception& e) {
        set_error(e.what());
        return nullptr;
    }
}

KIWIX_FFI_API void kiwix_search_result_free(kiwix_search_result_t* results) {
    delete results;
}

KIWIX_FFI_API int kiwix_search_result_count(kiwix_search_result_t* results) {
    if (!results) return 0;
    return (int)results->entries.size();
}

KIWIX_FFI_API char* kiwix_search_result_get_title(kiwix_search_result_t* results, int index) {
    if (!results || index < 0 || index >= (int)results->entries.size()) return nullptr;
    return dup_string(results->entries[index].title);
}

KIWIX_FFI_API char* kiwix_search_result_get_path(kiwix_search_result_t* results, int index) {
    if (!results || index < 0 || index >= (int)results->entries.size()) return nullptr;
    return dup_string(results->entries[index].path);
}

KIWIX_FFI_API char* kiwix_search_result_get_snippet(kiwix_search_result_t* results, int index) {
    if (!results || index < 0 || index >= (int)results->entries.size()) return nullptr;
    return dup_string(results->entries[index].snippet);
}

/* ========================================================================
 * Version
 * ======================================================================== */

KIWIX_FFI_API char* kiwix_get_version(void) {
    return dup_string("kiwix-tools 3.8.2 (libkiwix 14.2.0, libzim 9.6.0)");
}
