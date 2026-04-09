#include "tools.h"
#include "stringTools.h"
#include <mutex>

namespace kiwix
{

namespace
{

// These mappings are not provided by the ICU library, any such mappings can be manually added here
std::map<std::string, std::string> iso639_3 = {
    {"ami", "Amis"},
    {"atj", "atikamekw"},
    {"azb", "آذربایجان دیلی"},
    {"bcl", "central bikol"},
    {"bgs", "tagabawa"},
    {"blk", "ပအိုဝ်ႏ"},
    {"bxr", "буряад хэлэн"},
    {"cbk", "chavacano"},
    {"cdo", "閩東語"},
    {"dag", "Dagbani"},
    {"diq", "dimli"},
    {"dty", "डोटेली"},
    {"eml", "emiliân-rumagnōl"},
    {"fbs", "српскохрватски"},
    {"fon", "fɔ̀ngbè"},
    {"gcr", "Kriyòl gwiyannen"},
    {"guw", "Gungbe"},
    {"hbs", "srpskohrvatski"},
    {"hyw", "հայերէն/հայերեն"},
    {"ido", "ido"},
    {"kbp", "kabɩyɛ"},
    {"kld", "Gamilaraay"},
    {"lbe", "лакку маз"},
    {"lbj", "ལ་དྭགས་སྐད་"},
    {"lld", "ladin"},
    {"map", "Austronesian"},
    {"mhr", "марий йылме"},
    {"mnw", "ဘာသာမန်"},
    {"myn", "mayan"},
    {"nah", "nahuatl"},
    {"nai", "north American Indian"},
    {"nds", "plattdütsch"},
    {"nrm", "bhasa narom"},
    {"olo", "livvi"},
    {"pih", "Pitcairn-Norfolk"},
    {"pnb", "Western Panjabi"},
    {"pwn", "Pinayuanan"},
    {"rmr", "Caló"},
    {"rmy", "romani shib"},
    {"roa", "romance languages"},
    {"skr", "سرائیکی"},
    {"szy", "Sakizaya"},
    {"tay", "Tayal"},
    {"tgl", "Wikang Tagalog"},
    {"twi", "Akwapem Twi"},
// ICU for Ubuntu versions <= focal (20.04) returns "" for the language code ""
// unlike the later versions - which returns "und". We map this value to "Undetermined" for a common ground.
    {"", "Undetermined"},
};

std::once_flag fillLanguagesFlag;

void fillLanguagesMap()
{
    for (auto icuLangPtr = icu::Locale::getISOLanguages(); *icuLangPtr != NULL; ++icuLangPtr) {
        const kiwix::ICULanguageInfo lang(*icuLangPtr);
        iso639_3.insert({lang.iso3Code(), lang.selfName()});
    }
    iso639_3.erase("mul");
}

} // unnamed namespace

std::string getLanguageSelfName(const std::string& lang)
{
    std::call_once(fillLanguagesFlag, fillLanguagesMap);
    const auto itr = iso639_3.find(lang);
    if (itr != iso639_3.end()) {
        return itr->second;
    }
    return lang;
};

} // namespace kiwix
