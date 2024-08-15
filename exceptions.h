// Copyright (C) 2024 Egor Demidov
// This file is part of soot-dem-gui
//
// This program is free software: you can redistribute it and/or modify it
// under the terms of the GNU General Public License as published by the
// Free Software Foundation, either version 3 of the License,
// or (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty
// of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
// See the GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program. If not, see <https://www.gnu.org/licenses/>.

#ifndef GUI_DESIGN_SOOT_DEM_EXCEPTIONS_H
#define GUI_DESIGN_SOOT_DEM_EXCEPTIONS_H

#include <exception>
#include <string>

class UiException : public std::exception {
public:
    explicit UiException(std::string const & message) : message{message} {}

    const char * what() const noexcept override {
        return message.c_str();
    }

private:
    const std::string message;
};

#endif //GUI_DESIGN_SOOT_DEM_EXCEPTIONS_H
