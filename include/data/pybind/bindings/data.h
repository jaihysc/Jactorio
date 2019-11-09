#ifndef DATA_PYBIND_BINDINGS_DATA_H
#define DATA_PYBIND_BINDINGS_DATA_H

#include <pybind11/embed.h>

/**
 * Adds a prototype tracked by
 */
void modify_proto(const std::string& type, const std::string& iname, const std::string& path);


#endif // DATA_PYBIND_BINDINGS_DATA_H
