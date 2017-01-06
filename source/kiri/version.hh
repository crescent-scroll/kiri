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

#ifndef KIRI_VERSION_HH
#define KIRI_VERSION_HH

#include <array>
#include <string>

namespace kiri {

class version {
public:
    version(const std::string& file);
    version(const std::string& file, const std::string& visual);
    
    explicit operator bool() const noexcept;
    bool good() const noexcept;
    
    operator const std::string&() const noexcept;
    const std::string& visual() const noexcept;
    
    const std::string& major() const;
    const std::string& minor() const;
    const std::string& patch() const;
    const std::string& stage() const;
    
private:
    std::array<std::string, 4> m_segments;
    std::string m_visual;
};

}

#endif
