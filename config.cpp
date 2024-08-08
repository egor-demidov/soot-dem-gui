//
// Created by Egor on 8/8/2024.
//

#include <tinyxml2/tinyxml2.h>

#include "config.h"

std::tuple<std::string, parameter_heap_t> load_config_file(std::filesystem::path const & config_path) {

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

    doc.SaveFile(config_path.string().c_str());
}
