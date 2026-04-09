#include "tools.h"
#include <pugixml.hpp>

namespace kiwix
{

namespace
{
#define VALUE(name) entryNode.child(name).child_value()
FeedLanguages parseLanguages(const pugi::xml_document& doc)
{
    pugi::xml_node feedNode = doc.child("feed");
    FeedLanguages langs;

    for (pugi::xml_node entryNode = feedNode.child("entry"); entryNode;
        entryNode = entryNode.next_sibling("entry")) {
            auto title = VALUE("title");
            auto code = VALUE("dc:language");
            langs.push_back({code, title});
    }

    return langs;
}

FeedCategories parseCategories(const pugi::xml_document& doc)
{
    pugi::xml_node feedNode = doc.child("feed");
    FeedCategories categories;

    for (pugi::xml_node entryNode = feedNode.child("entry"); entryNode;
        entryNode = entryNode.next_sibling("entry")) {
            auto title = VALUE("title");
            categories.push_back(title);
    }

    return categories;
}

} // unnamed namespace

FeedLanguages readLanguagesFromFeed(const std::string& content)
{
    pugi::xml_document doc;
    pugi::xml_parse_result result
        = doc.load_buffer((void*)content.data(), content.size());

    if (result) {
        auto langs = parseLanguages(doc);
        return langs;
    }

    return FeedLanguages();
}

FeedCategories readCategoriesFromFeed(const std::string& content)
{
    pugi::xml_document doc;
    pugi::xml_parse_result result
        = doc.load_buffer((void*)content.data(), content.size());

    FeedCategories categories;
    if (result) {
        categories = parseCategories(doc);
        return categories;
    }

    return categories;
}

} // namespace kiwix
