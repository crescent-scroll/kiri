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

#ifndef KIRI_STORAGE_HH
#define KIRI_STORAGE_HH

#include <functional>
#include <map>
#include <set>
#include <string>

namespace kiri {

class storage {
public:
    enum class key {
        index, // files in the local index
        project // files related to the project
    };
    
    enum class type {
        header, source, any
    };
    
    enum class location {
        local, // files within the source tree
        global // files belonging to the system
    };
    
    storage(bool initialization=false);
    storage(const std::string& origin);
    
    explicit operator bool() const noexcept;
    bool ready() const noexcept;
    
    std::string origin() const;
    std::string base() const;
    
    std::string version() const;
    std::string configuration() const;
    
    std::string create(location kind, const std::string& identifier) const;
    
    void use(type quality, std::string extension);
    
    bool include(const std::string& path);
    bool exclude(const std::string& path);
    
    bool iterate(key category,
        location kind,
        const std::function<bool (std::string, type)>& action);
    
private:
    std::string m_origin;
    
    std::map<type, std::set<std::string>> m_extensions;
    
    std::map<location, std::set<std::string>> m_includes;
    std::map<location, std::set<std::string>> m_excludes;
};

}

#endif
