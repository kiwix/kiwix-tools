#include "library_dumper.h"
#include "tools/stringTools.h"
#include "tools/otherTools.h"
#include "tools.h"

namespace kiwix
{
/* Constructor */
LibraryDumper::LibraryDumper(const Library* library, const NameMapper* nameMapper)
  : library(library),
    nameMapper(nameMapper)
{
}
/* Destructor */
LibraryDumper::~LibraryDumper()
{
}

void LibraryDumper::setOpenSearchInfo(int totalResults, int startIndex, int count)
{
  m_totalResults = totalResults;
  m_startIndex = startIndex,
  m_count = count;
}

kainjow::mustache::list LibraryDumper::getCategoryData() const
{
  const auto now = gen_date_str();
  kainjow::mustache::list categoryData;
  for ( const auto& category : library->getBooksCategories() ) {
    const auto urlencodedCategoryName = urlEncode(category);
    categoryData.push_back(kainjow::mustache::object{
      {"name", category},
      {"urlencoded_name",  urlencodedCategoryName},
      {"updated", now},
      {"id", gen_uuid(libraryId + "/categories/" + urlencodedCategoryName)}
    });
  }
  return categoryData;
}

kainjow::mustache::list LibraryDumper::getLanguageData() const
{
  const auto now = gen_date_str();
  kainjow::mustache::list languageData;
  for ( const auto& langAndBookCount : library->getBooksLanguagesWithCounts() ) {
    const std::string languageCode = langAndBookCount.first;
    const int bookCount = langAndBookCount.second;
    const auto languageSelfName = getLanguageSelfName(languageCode);
    languageData.push_back(kainjow::mustache::object{
      {"lang_code",  languageCode},
      {"lang_self_name", languageSelfName},
      {"book_count", to_string(bookCount)},
      {"updated", now},
      {"id", gen_uuid(libraryId + "/languages/" + languageCode)}
    });
  }
  return languageData;
}

} // namespace kiwix
