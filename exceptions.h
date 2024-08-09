//
// Created by Egor Demidov on 8/8/24.
//

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
