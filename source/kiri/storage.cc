/*  Kiri - A tiny C++ source code assistant
    Copyright (C) 2017 Crescent Scroll
    
    This program is free software: you can redistribute it and/or modify it
    under the terms of the GNU General Public License as published by the Free
    Software Foundation, either version 3 of the License, or (at your option)
    any later version.
    
    This program is distributed in the hope that it will be useful, but WITHOUT
    ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
    FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License for
    more details.
    
    You should have received a copy of the GNU General Public License along with
    this program.  If not, see <http://www.gnu.org/licenses/>.
*/

#include "storage.hh"

#include <algorithm>
#include <experimental/filesystem>
#include <iterator>
#include <stdexcept>
#include <utility>
#include <vector>

namespace std {

using namespace experimental::filesystem;

}

namespace kiri {

namespace {

std::path base(std::path origin)
{
    return origin/=".kiri";
}

std::path version(std::path origin)
{
    return base(std::move(origin))/="version";
}

std::path configuration(std::path origin)
{
    return base(std::move(origin))/="configuration";
}

std::path index(std::path origin)
{
    return base(std::move(origin))/="index";
}

std::path local(std::path origin)
{
    return index(std::move(origin))/="local";
}

std::path global(std::path origin)
{
    return index(std::move(origin))/="global";
}

}

namespace {

std::path locate(std::path origin=std::current_path())
{
    origin=std::absolute(std::move(origin));
    
    do {
        const auto base=kiri::base(origin);
        
        if (std::exists(base)&&std::is_directory(base)) {
            return std::canonical(std::move(origin));
        }
        
        origin=origin.parent_path();
    } while (!origin.empty());
    
    return {};
}

std::path initialize(std::path origin=std::current_path())
{
    origin=std::absolute(std::move(origin));
    
    if (std::exists(base(origin))) {
        if (std::is_directory(base(origin))) {
            throw std::runtime_error{"origin already initialized"};
        }
        
        throw std::runtime_error{"invalid initialization of origin"};
    }
    
    std::vector<std::path> directories{base(origin),
        index(origin), local(origin), global(origin)};
    
    for (const auto& directory:directories) {
        std::create_directories(directory);
    }
    
    return std::canonical(std::move(origin));
}

}

storage::storage(bool initialization)
    : m_origin{initialization?initialize():locate()}
{
}

storage::storage(const std::string& origin)
    : m_origin{initialize(origin)}
{
}

storage::operator bool() const noexcept
{
    return ready();
}

bool storage::ready() const noexcept
{
    return !m_origin.empty();
}

std::string storage::origin() const
{
    return m_origin;
}

std::string storage::base() const
{
    return kiri::base(m_origin);
}

std::string storage::version() const
{
    return kiri::version(m_origin);
}

std::string storage::configuration() const
{
    return kiri::configuration(m_origin);
}

std::string storage::create(location kind, const std::string& identifier) const
{
    if (kind==location::local) {
        return local(m_origin)/=identifier;
    }
    
    return local(m_origin)/=identifier;
}

namespace {

template<typename Key, typename Value>
Value& get(std::map<Key, Value>& map, const Key& key)
{
    if (map.count(key)==0) {
        map.emplace(key, Value{});
    }
    
    return map.at(key);
}

}

void storage::use(type quality, std::string extension)
{
    if (!extension.empty()) {
        get(m_extensions, quality).insert(std::move(extension));
    }
}

namespace {

bool common(std::path child, const std::path& parent)
{
    while (!child.empty()) {
        if (std::equivalent(child, parent)) {
            return true;
        }
        
        child=child.parent_path();
    }
    
    return false;
}

storage::location trace(std::path path, const std::path& origin)
{
    if (common(std::move(path), origin)) {
        return storage::location::local;
    }
    
    return storage::location::global;
}

template<typename Container>
bool insert(std::path path, const std::path& origin, Container& map)
{
    path=std::absolute(std::move(path));
    
    if (!std::exists(path)) {
        return false;
    }
    
    const auto kind=trace(path, origin);
    
    if (std::is_directory(path)) {
        auto& list=get(map, kind);
        
        for (auto iterator=std::begin(list); iterator!=std::end(list);) {
            if (common(path, *iterator)) {
                return false; // path already taken into account
            }
            
            if (common(*iterator, path)) {
                list.erase(iterator); // path becomes obsolete
            } else {
                ++iterator;
            }
        }
    }
    
    get(map, kind).insert(path);
    
    return true;
}

}

bool storage::include(const std::string& path)
{
    return insert(path, m_origin, m_includes);
}

bool storage::exclude(const std::string& path)
{
    return insert(path, m_origin, m_excludes);
}

namespace {

template<typename Map>
storage::type classify(const std::path& file, const Map& extensions)
{
    if (file.has_extension()) {
        std::string extension=file.extension();
        extension.erase(std::begin(extension));
        
        for (const auto& element:extensions) {
            const auto begin=std::begin(element.second);
            const auto end=std::end(element.second);
            
            if (std::find(begin, end, extension)!=end) {
                return element.first;
            }
        }
    }
    
    return storage::type::any;
}

template<typename Map>
bool verify(const std::path& file, const Map& excludes)
{
    for (const auto& element:excludes) {
        for (const auto& exclude:element.second) {
            if (std::equivalent(exclude, file)||common(file, exclude)) {
                return false;
            }
        }
    }
    
    return true;
}

template<typename Extensions, typename Excludes, typename Function>
bool forward(const std::path& path,
    const Extensions& extensions,
    const Excludes& excludes, const Function& action)
{
    if (!std::is_directory(path)) {
        const auto category=classify(path, extensions);
        
        if (verify(path, excludes)) {
            return action(path, category);
        }
    }
    
    return false; // continue
}

template<typename Extensions, typename Excludes, typename Function>
bool search(const std::path& path,
    const Extensions& extensions,
    const Excludes& excludes, const Function& action)
{
    if (!std::exists(path)) {
        return false; // continue
    }
    
    if (std::is_directory(path)) {
        for (const auto& path:std::recursive_directory_iterator{path}) {
            if (forward(path, extensions, excludes, action)) {
                return true; // break
            }
        }
    }
    
    return forward(path, extensions, excludes, action);
}

}

bool storage::iterate(key category,
    location kind, const std::function<bool (std::string, type)>& action)
{
    if (category==key::index) {
        if (kind==location::local) {
            return search(local(m_origin), m_extensions, m_excludes, action);
        }
        
        return search(global(m_origin), m_extensions, m_excludes, action);
    }
    
    for (const auto& include:get(m_includes, kind)) {
        if (search(include, m_extensions, m_excludes, action)) {
            return true; // success
        }
    }
    
    return false; // no success
}

}
