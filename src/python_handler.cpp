#include "python_handler.h"

const TSLanguage* PythonHandler::get_language() const {
    return tree_sitter_python();
}

std::vector<std::string> PythonHandler::get_extensions() const {
    return {".py"};
}

std::string PythonHandler::get_base_name(const TSNode& node, const std::string& source) const {
    std::string type = ts_node_type(node);
    if (type == "identifier") {
        return Parser::get_node_text(node, source);
    } else if (type == "attribute") {
        TSNode attr = ts_node_child_by_field_name(node, "attribute", 9);
        if (!ts_node_is_null(attr)) {
            return Parser::get_node_text(attr, source);
        }
    }
    return "";
}

std::string PythonHandler::get_fully_qualified_name(const TSNode& def_node, const std::string& source) const {
    TSNode name_node = ts_node_child_by_field_name(def_node, "name", 4);
    if (ts_node_is_null(name_node)) return "";
    std::string base_name = get_base_name(name_node, source);
    if (base_name.empty()) return "";

    std::stack<std::string> scopes;
    TSNode current = def_node;
    while (!ts_node_is_null(current)) {
        current = ts_node_parent(current);
        if (ts_node_is_null(current)) break;
        std::string type = ts_node_type(current);
        if (type == "class_definition") {
            TSNode class_name_node = ts_node_child_by_field_name(current, "name", 4);
            if (!ts_node_is_null(class_name_node)) {
                scopes.push(get_base_name(class_name_node, source));
            }
        } else if (type == "function_definition") {
            TSNode func_name_node = ts_node_child_by_field_name(current, "name", 4);
            if (!ts_node_is_null(func_name_node)) {
                scopes.push(get_base_name(func_name_node, source));
            }
        }
    }

    std::string fq_name;
    while (!scopes.empty()) {
        fq_name += scopes.top() + ".";
        scopes.pop();
    }
    fq_name += base_name;
    return fq_name;
}

void PythonHandler::collect_function_definitions(const TSNode& node, std::vector<TSNode>& definitions) const {
    std::string type = ts_node_type(node);
    if (type == "function_definition") {
        definitions.push_back(node);
    }
    uint32_t count = ts_node_child_count(node);
    for (uint32_t i = 0; i < count; ++i) {
        TSNode child = ts_node_child(node, i);
        collect_function_definitions(child, definitions);
    }
}

void PythonHandler::find_calls(const TSNode& node, const std::string& source, std::unordered_set<std::string>& calls) const {
    std::string node_type = ts_node_type(node);
    if (node_type == "call") {
        TSNode function = ts_node_child_by_field_name(node, "function", 8);
        if (!ts_node_is_null(function)) {
            std::string call_name = get_base_name(function, source);
            if (!call_name.empty()) {
                calls.insert(call_name);
            }
        }
    }
    uint32_t child_count = ts_node_child_count(node);
    for (uint32_t i = 0; i < child_count; ++i) {
        TSNode child = ts_node_child(node, i);
        find_calls(child, source, calls);
    }
}
