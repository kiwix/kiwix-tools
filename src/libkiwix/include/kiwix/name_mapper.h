/*
 * Copyright 2019 Matthieu Gautier <mgautier@kymeria.fr>
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

#ifndef KIWIX_NAMEMAPPER_H
#define KIWIX_NAMEMAPPER_H

#include <string>
#include <map>
#include <memory>
#include <mutex>

namespace kiwix
{

class Library;

class NameMapper {
  public:
    virtual ~NameMapper() = default;
    virtual std::string getNameForId(const std::string& id) const = 0;
    virtual std::string getIdForName(const std::string& name) const = 0;
};


class IdNameMapper : public NameMapper {
  public:
    virtual std::string getNameForId(const std::string& id) const { return id; };
    virtual std::string getIdForName(const std::string& name) const { return name; };
};

class HumanReadableNameMapper : public NameMapper {
  private:
    std::map<std::string, std::string> m_idToName;
    std::map<std::string, std::string> m_nameToId;

  public:
    HumanReadableNameMapper(const kiwix::Library& library, bool withAlias);
    virtual ~HumanReadableNameMapper() = default;
    virtual std::string getNameForId(const std::string& id) const;
    virtual std::string getIdForName(const std::string& name) const;

  private:
    void mapName(const kiwix::Library& lib, std::string name, std::string id);
};

class UpdatableNameMapper : public NameMapper {
    typedef std::shared_ptr<NameMapper> NameMapperHandle;
  public:
    UpdatableNameMapper(std::shared_ptr<Library> library, bool withAlias);

    virtual std::string getNameForId(const std::string& id) const;
    virtual std::string getIdForName(const std::string& name) const;

    void update();

  private:
    NameMapperHandle currentNameMapper() const;

  private:
    mutable std::mutex mutex;
    std::shared_ptr<Library> library;
    NameMapperHandle nameMapper;
    const bool withAlias;
};

}

#endif
