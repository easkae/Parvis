#pragma once
#include "language_handler.h"
#include "parser.h"

extern "C" const TSLanguage* tree_sitter_python();

class PythonHandler : public LanguageHandler {
public:
    const TSLanguage* get_language() const override;
    std::vector<std::string> get_extensions() const override;
    std::string get_base_name(const TSNode& node, const std::string& source) const override;
    std::string get_fully_qualified_name(const TSNode& def_node, const std::string& source) const override;
    void collect_function_definitions(const TSNode& node, std::vector<TSNode>& definitions) const override;
    void find_calls(const TSNode& node, const std::string& source, std::unordered_set<std::string>& calls) const override;
};