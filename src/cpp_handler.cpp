#include "cpp_handler.h"
#include <stack>

const TSLanguage* CppHandler::get_language() const {
    return tree_sitter_cpp();
}

std::vector<std::string> CppHandler::get_extensions() const {
    return {".cpp", ".cc", ".cxx", ".c++", ".h", ".hpp", ".hh", ".hxx"};
}

std::string CppHandler::get_base_name(const TSNode& node, const std::string& source) const {
    std::string type = ts_node_type(node);
    if (type == "identifier" || type == "field_identifier") {
        return Parser::get_node_text(node, source);
    } else if (type == "destructor_name") {
        return "~" + Parser::get_node_text(ts_node_child(node, 0), source);
    } else if (type == "operator_name") {
        return "operator" + Parser::get_node_text(ts_node_child(node, 0), source);
    } else if (type == "qualified_identifier") {
        std::string name;
        uint32_t count = ts_node_child_count(node);
        for (uint32_t i = 0; i < count; ++i) {
            TSNode child = ts_node_child(node, i);
            std::string child_type = ts_node_type(child);
            if (child_type == "scope_resolution") {
                name += "::";
            } else {
                std::string child_name = get_base_name(child, source);
                if (!child_name.empty()) {
                    name += child_name;
                }
            }
        }
        return name;
    } else if (type == "template_function") {
        return get_base_name(ts_node_child_by_field_name(node, "name", 4), source);
    }
    TSNode inner = ts_node_child_by_field_name(node, "declarator", 10);
    if (!ts_node_is_null(inner)) {
        return get_base_name(inner, source);
    }
    return "";
}

std::string CppHandler::get_fully_qualified_name(const TSNode& def_node, const std::string& source) const {
    std::string base_name = "";
    TSNode name_node = def_node;
    TSNode declarator = ts_node_child_by_field_name(def_node, "declarator", 10);
    if (!ts_node_is_null(declarator)) {
        std::string decl_type = ts_node_type(declarator);
        while (decl_type != "function_declarator" && decl_type != "field_declarator") {
            declarator = ts_node_child_by_field_name(declarator, "declarator", 10);
            if (ts_node_is_null(declarator)) return "";
            decl_type = ts_node_type(declarator);
        }
        name_node = ts_node_child_by_field_name(declarator, "declarator", 10);
        if (ts_node_is_null(name_node)) return "";
        base_name = get_base_name(name_node, source);
    } else {
        return "";
    }

    std::stack<std::string> scopes;
    TSNode current = def_node;
    while (!ts_node_is_null(current)) {
        current = ts_node_parent(current);
        if (ts_node_is_null(current)) break;
        std::string type = ts_node_type(current);
        if (type == "class_specifier" || type == "struct_specifier" || type == "union_specifier") {
            TSNode class_name_node = ts_node_child_by_field_name(current, "name", 4);
            if (!ts_node_is_null(class_name_node)) {
                scopes.push(get_base_name(class_name_node, source));
            }
        } else if (type == "namespace_definition") {
            TSNode ns_name_node = ts_node_child_by_field_name(current, "name", 4);
            if (!ts_node_is_null(ns_name_node)) {
                scopes.push(get_base_name(ns_name_node, source));
            }
        } else if (type == "template_declaration") {
            // TODO
        }
    }

    std::string fq_name;
    while (!scopes.empty()) {
        fq_name += scopes.top() + "::";
        scopes.pop();
    }
    fq_name += base_name;
    return fq_name;
}

void CppHandler::collect_function_definitions(const TSNode& node, std::vector<TSNode>& definitions) const {
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

void CppHandler::find_calls(const TSNode& node, const std::string& source, std::unordered_set<std::string>& calls) const {
    std::string node_type = ts_node_type(node);
    if (node_type == "call_expression") {
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
