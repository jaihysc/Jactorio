// This file is subject to the terms and conditions defined in 'LICENSE' in the source code package

#ifndef JACTORIO_INCLUDE_PROTO_DETAIL_PYTHON_PROP_H
#define JACTORIO_INCLUDE_PROTO_DETAIL_PYTHON_PROP_H
#pragma once

// Creates a setters for python API, to chain initialization

/// Setter passed by reference
#define PYTHON_PROP_REF(type__, var_name__)                   \
    type__ var_name__;                                        \
    auto Set_##var_name__(const type__&(parameter_value__)) { \
        this->var_name__ = parameter_value__;                 \
        return this;                                          \
    }                                                         \
    static_assert(true)

/// Setter passed by reference with initializer
#define PYTHON_PROP_REF_I(type__, var_name__, initializer__)  \
    type__ var_name__ = initializer__;                        \
    auto Set_##var_name__(const type__&(parameter_value__)) { \
        this->var_name__ = parameter_value__;                 \
        return this;                                          \
    }                                                         \
    static_assert(true)

/// Setter passed by value
#define PYTHON_PROP(type__, var_name__)                \
    type__ var_name__;                                 \
    auto Set_##var_name__(type__(parameter_value__)) { \
        this->var_name__ = parameter_value__;          \
        return this;                                   \
    }                                                  \
    static_assert(true)

/// Setter passed by value with initializer
#define PYTHON_PROP_I(type__, var_name__, initializer__) \
    type__ var_name__ = initializer__;                   \
    auto Set_##var_name__(type__(parameter_value__)) {   \
        this->var_name__ = parameter_value__;            \
        return this;                                     \
    }                                                    \
    static_assert(true)

#endif // JACTORIO_INCLUDE_PROTO_DETAIL_PYTHON_PROP_H
