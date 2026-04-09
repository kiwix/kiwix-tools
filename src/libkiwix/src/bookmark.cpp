/*
 * Copyright 2018 Matthieu Gautier <mgautier@kymeria.fr>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU  General Public License as published by
 * the Free Software Foundation; either version 3 of the License, or
 * any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston,
 * MA 02110-1301, USA.
 */

#include "bookmark.h"
#include "book.h"

#include <pugixml.hpp>

namespace kiwix
{
/* Constructor */
Bookmark::Bookmark()
{
}

Bookmark::Bookmark(const Book& book, const std::string& path, const std::string& title):
  m_bookId(book.getId()),
  m_bookTitle(book.getTitle()),
  m_bookName(book.getName()),
  m_bookFlavour(book.getFlavour()),
  m_url(path),
  m_title(title),
  m_language(book.getCommaSeparatedLanguages()),
  m_date(book.getDate())
{}

/* Destructor */
Bookmark::~Bookmark()
{
}

void Bookmark::updateFromXml(const pugi::xml_node& node)
{
  auto bookNode = node.child("book");
  m_bookId = bookNode.child("id").child_value();
  m_bookTitle = bookNode.child("title").child_value();
  m_bookName = bookNode.child("name").child_value();
  m_bookFlavour = bookNode.child("flavour").child_value();
  m_language = bookNode.child("language").child_value();
  m_date = bookNode.child("date").child_value();
  m_title = node.child("title").child_value();
  m_url = node.child("url").child_value();
}

}
