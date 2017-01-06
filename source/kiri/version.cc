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

#include "version.hh"

#include <fstream>
#include <iterator>
#include <regex>
#include <stdexcept>

namespace kiri {

namespace {

std::string load(const std::string& file)
{
    std::ifstream stream{file};
    
    if (!stream) {
        throw std::runtime_error{"unable to load version"};
    }
    
    return {std::istreambuf_iterator<char>{stream},
        std::istreambuf_iterator<char>{}};
}

std::array<std::string, 4> parse(const std::string& content)
{
    static auto expression=[]() {
        const std::string space{"[[:space:]]*"},
            dot{"\\."}, number{"([[:digit:]]+)"}, stage{"(-([[:alnum:]]+))?"};
        
        return std::regex{space+number+dot+number+dot+number+stage+space};
    } ();
    
    std::smatch match;
    
    if (std::regex_match(content, match, expression)) {
        return {match[1], match[2], match[3], match[5]};
    }
    
    throw std::runtime_error{"unable to parse version"};
}

std::string merge(const std::array<std::string, 4>& segments)
{
    auto visual=segments[0]+'.'+segments[1]+'.'+segments[2];
    
    if (!segments[3].empty()) {
        visual += '-'+segments[3];
    }
    
    return visual;
}

}

version::version(const std::string& file)
    : m_segments{parse(load(file))}, m_visual{merge(m_segments)}
{
}

version::version(const std::string& file, const std::string& string)
    : m_segments{parse(string)}, m_visual{merge(m_segments)}
{
    std::ofstream stream{file};
    stream << m_visual << std::endl;
    
    if (!stream) {
        throw std::runtime_error{"unable to save version"};
    }
}

version::operator bool() const noexcept
{
    return good();
}

bool version::good() const noexcept
{
    return !m_visual.empty();
}

version::operator const std::string&() const noexcept
{
    return visual();
}

const std::string& version::visual() const noexcept
{
    return m_visual;
}

const std::string& version::major() const
{
    return m_segments[0];
}

const std::string& version::minor() const
{
    return m_segments[1];
}

const std::string& version::patch() const
{
    return m_segments[2];
}

const std::string& version::stage() const
{
    return m_segments[3];
}

}
