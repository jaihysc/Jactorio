// This file is subject to the terms and conditions defined in 'LICENSE' in the source code package

#include "data/local_parser.h"

#include <sstream>

#include "jactorio.h"

#include "data/prototype_manager.h"
#include "proto/detail/exception.h"

using namespace jactorio;

struct ParserData
{
    // Used to store data as its being parsed
    std::string currentLineBuffer;
    std::string lValue;

    // Keep track of line and char for better errors
    unsigned int lineNumber = 1;
    unsigned int charNumber = 0;

    bool inLVal = false; // Left of equals sign
    bool inRVal = false; // Right of equals sign

    ///
    /// Call this when entering a new line to reset variables and buffers
    void ResetVariables() {
        charNumber = 0;

        inLVal = false;
        inRVal = false;
        currentLineBuffer.clear();
    }

    ///
    /// Logs parsing error message and throws
    /// \exception ProtoError Thrown when this function is called
    [[noreturn]] void ParseError(const std::string& message) const {
        std::stringstream str_s;
        str_s << "Localization parse failed " << lineNumber << ":" << charNumber << " '" << message << "'";

        throw proto::ProtoError(str_s.str());
    }
};


///
/// Helper for parse, handles end of line actions
void ParseEol(data::PrototypeManager& proto, ParserData& parser_data, const std::string& directory_prefix) {
    // R val was not specified or empty
    if (parser_data.inLVal && !parser_data.inRVal)
        parser_data.ParseError("expected '=' to switch to r-value, got newline");
    if (parser_data.inRVal && parser_data.currentLineBuffer.empty())
        parser_data.ParseError("expected r-value, got newline");

    // Have not entered l value yet
    if (!parser_data.inLVal)
        return;

    // Save R val, reset for next line

    // Generate internal name to search for
    std::stringstream str_s;
    str_s << "__" << directory_prefix << "__/" << parser_data.lValue;

    auto* prototype = proto.Find(str_s.str());
    if (prototype != nullptr) {
        prototype->SetLocalizedName(parser_data.currentLineBuffer);
        LOG_MESSAGE_F(debug, "Registered local '%s' '%s'", str_s.str().c_str(), parser_data.currentLineBuffer.c_str());
    }
    else {
        LOG_MESSAGE_F(warning, "Local option '%s' missing matching prototype internal name", str_s.str().c_str());
    }

    parser_data.ResetVariables();
    parser_data.lineNumber++;
}

void data::LocalParse(PrototypeManager& proto, const std::string& file_str, const std::string& directory_prefix) {
    ParserData parser_data{};

    for (char c : file_str) {
        parser_data.charNumber++;

        // Always skip tabs
        if (c == '\t')
            continue;
        // Skip whitespace when not in quotes
        if (c == ' ' && !parser_data.inLVal)
            continue;


        // End of a line
        if (c == '\n') {
            ParseEol(proto, parser_data, directory_prefix);
            continue;
        }


        // Enter data if character is not whitespace or = (whitespace checked above)
        if (c != '=')
            parser_data.inLVal = true;

        // Already in R-val, encountered =
        else if (parser_data.inRVal) {
            parser_data.ParseError("attempted to switch to r-value with '=' when already in r-value");
        }
        // Is (=), exit data and switch to second data field upon reaching =
        else if (parser_data.inLVal) {
            parser_data.inLVal = false;
            parser_data.inRVal = true;

            // Save L val
            parser_data.lValue = parser_data.currentLineBuffer;
            parser_data.currentLineBuffer.clear();
            continue;
        }
        // (=) without first defining a l value
        else {
            parser_data.ParseError("attempted to switch to r-value with '=' without defining a l value");
        }


        // Save characters into buffer
        if (parser_data.inLVal)
            parser_data.currentLineBuffer.push_back(c);
    }

    ParseEol(proto, parser_data, directory_prefix);
}
