// This file is subject to the terms and conditions defined in 'LICENSE' in the source code package

#ifndef JACTORIO_INCLUDE_PROTO_LOCALIZATION_H
#define JACTORIO_INCLUDE_PROTO_LOCALIZATION_H
#pragma once

#include "proto/framework/framework_base.h"

namespace jactorio::proto
{
    class Localization final : public FrameworkBase
    {
    public:
        PROTOTYPE_CATEGORY(localization);

        /// Loads locale from file with <provided identifier>.cfg
        /// \remark Must be unique
        PYTHON_PROP_REF(std::string, identifier);

        /// Path to ttf tile for font, relative to prototype folder
        /// E.g: base/fonts/arial.ttf
        PYTHON_PROP_REF(std::string, fontPath);

        PYTHON_PROP_REF_I(float, fontSize, 12);


        void PostLoadValidate(const data::PrototypeManager& /*proto*/) const override {
            J_PROTO_ASSERT(!identifier.empty(), "A localization identifier must be provided");
            J_PROTO_ASSERT(fontSize > 0, "Font size must be greater than 0");
        }
    };
} // namespace jactorio::proto

#endif // JACTORIO_INCLUDE_PROTO_LOCALIZATION_H
