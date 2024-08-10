//
// Created by Egor on 8/8/2024.
//

#include <iostream>
#include <tinyxml2/tinyxml2.h>

#include "config.h"

void set_real(parameter_heap_t & parameter_store, std::string const & id, double value) {
    auto parameter_itr = parameter_store.find(id);

    if (parameter_itr != parameter_store.end())
        throw UiException("Corrupt config file - multiple definition of parameter `" + id + "`");

    parameter_store[id] = {REAL, {.real_value = value}};
}

void set_integer(parameter_heap_t & parameter_store, std::string const & id, long value) {
    auto parameter_itr = parameter_store.find(id);

    if (parameter_itr != parameter_store.end())
        throw UiException("Corrupt config file - multiple definition of parameter `" + id + "`");

    parameter_store[id] = {INTEGER, {.integer_value = value}};
}

void set_string(parameter_heap_t & parameter_store, std::string const & id, std::string const & value) {
    auto parameter_itr = parameter_store.find(id);

    if (parameter_itr != parameter_store.end())
        throw UiException("Corrupt config file - multiple definition of parameter `" + id + "`");

    parameter_store[id] = {STRING, {.string_value = value}};
}

void set_path(parameter_heap_t & parameter_store, std::string const & id, std::filesystem::path const & value) {
    auto parameter_itr = parameter_store.find(id);

    if (parameter_itr != parameter_store.end())
        throw UiException("Corrupt config file - multiple definition of parameter `" + id + "`");

    parameter_store[id] = {PATH, {.path_value = value}};
}

std::tuple<std::string, parameter_heap_t> load_config_file(std::filesystem::path const & config_path) {
    tinyxml2::XMLDocument doc;
    tinyxml2::XMLError err = doc.LoadFile(config_path.string().c_str());

    if (err != tinyxml2::XML_SUCCESS) {
        throw UiException("Unable to read config file");
    }

    auto root_element = doc.FirstChildElement("simulation");

    if (root_element == nullptr)
        throw UiException("Config file corrupt - root element of the parameter file must be `simulation`");

    auto simulation_type = root_element->FindAttribute("type");

    if (simulation_type == nullptr)
        throw UiException("Config file corrupt - `simulation` element must contain a `type` attribute");

    auto simulation_type_str = std::string(simulation_type->Value());

    parameter_heap_t parameter_store;

    // Iterate over parameter declarations
    for (auto element = root_element->FirstChildElement("let"); element != nullptr; element = element->NextSiblingElement("let")) {
        auto id = element->FindAttribute("id");

        if (id == nullptr)
            throw UiException("Config file corrupt - `let` element must contain an `id` attribute");

        std::string id_string(id->Value());

        auto type = element->FindAttribute("type");

        if (type == nullptr)
            throw UiException("Every `let` element must contain a `type` attribute");

        std::string type_string(type->Value());
        auto text = element->GetText();
        if (type_string == "real") {
            set_real(parameter_store, id_string, std::stod(text));
        } else if (type_string == "integer") {
            set_integer(parameter_store, id_string, std::stol(text));
        } else if (type_string == "string") {
            set_string(parameter_store, id_string, text);
        } else if (type_string == "path") {
            set_path(parameter_store, id_string, std::filesystem::path(text));
        } else {
            throw UiException("Config file corrupt - unrecognized parameter type `" + type_string + "`");
        }
    }

//    std::cout << "Loaded simulation of type " << simulation_type_str << "\n";
//    for (auto & [id, data] : parameter_store) {
//        auto & [type, value] = data;
//        std::cout << id << " " << parameter_type_to_string(type) << " ";
//        switch (type) {
//            case REAL:
//                std::cout << value.real_value;
//                break;
//            case INTEGER:
//                std::cout << value.integer_value;
//                break;
//            case STRING:
//                std::cout << value.string_value;
//                break;
//            case PATH:
//                std::cout << value.path_value.string();
//        }
//        std::cout << "\n";
//    }

    return {simulation_type_str, parameter_store};
}

void write_config_file(std::filesystem::path const & config_path,
                       const char * simulation_config_signature,
                       parameter_heap_t const & parameters) {

    tinyxml2::XMLDocument doc;
    tinyxml2::XMLElement * root_element = doc.NewElement("simulation");
    root_element->SetAttribute("type", simulation_config_signature);
    doc.InsertFirstChild(doc.NewDeclaration());
    doc.InsertEndChild(root_element);

    for (auto const & [id, parameter] : parameters) {
        auto const & [parameter_type, parameter_value] = parameter;
        tinyxml2::XMLElement * child_element = root_element->InsertNewChildElement("let");
        child_element->SetAttribute("id", id.c_str());
        child_element->SetAttribute("type", parameter_type_to_string(parameter_type));
        child_element->SetText(parameter_value_to_string(parameter_type, parameter_value).c_str());
    }

    tinyxml2::XMLError err = doc.SaveFile(config_path.string().c_str());

    if (err != tinyxml2::XML_SUCCESS) {
        throw UiException("Unable to write config file");
    }
}
