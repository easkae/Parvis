#pragma once
#include <tree_sitter/api.h>
#include <string>
#include <vector>
#include <unordered_set>

class LanguageHandler {
public:
    virtual ~LanguageHandler() = default;
    virtual const TSLanguage* get_language() const = 0;
    virtual std::vector<std::string> get_extensions() const = 0;
    virtual std::string get_base_name(const TSNode& node, const std::string& source) const = 0;
    virtual std::string get_fully_qualified_name(const TSNode& def_node, const std::string& source) const = 0;
    virtual void collect_function_definitions(const TSNode& node, std::vector<TSNode>& definitions) const = 0;
    virtual void find_calls(const TSNode& node, const std::string& source, std::unordered_set<std::string>& calls) const = 0;
};
