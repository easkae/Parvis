#include "parser.h"

std::string
Parser::read_file(const std::string& path)
{
    std::ifstream file(path, std::ios::binary);
    if (!file) {
        throw std::runtime_error("Failed to open file: " + path);
    }
    std::stringstream buffer;
    buffer << file.rdbuf();
    return buffer.str();
}

std::string
Parser::get_node_text(const TSNode& node, const std::string& source) {
    uint32_t start = ts_node_start_byte(node);
    uint32_t end = ts_node_end_byte(node);
    return source.substr(start, end - start);
}

std::string
Parser::get_base_name(const TSNode& node, const std::string& source) {
    std::string type = ts_node_type(node);
    if (type == "identifier" || type == "field_identifier") {
        return get_node_text(node, source);
    } else if (type == "destructor_name") {
        return "~" + get_node_text(ts_node_child(node, 0), source);
    } else if (type == "operator_name") {
        return "operator" + get_node_text(ts_node_child(node, 0), source);
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

std::string
Parser::get_fully_qualified_name(const TSNode& def_node, const std::string& source) {
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

void
Parser::collect_function_definitions(const TSNode& node, std::vector<TSNode>& definitions) {
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

void
Parser::find_calls(const TSNode& node, const std::string& source, std::unordered_set<std::string>& calls) {
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

void
Parser::process_file(const std::string& path, std::unordered_map<std::string, std::vector<std::string>>& call_graph) {
    std::string source = read_file(path);
    TSParser* parser = ts_parser_new();
    ts_parser_set_language(parser, tree_sitter_cpp());

    TSTree* tree = ts_parser_parse_string(parser, nullptr, source.c_str(), source.length());
    TSNode root = ts_tree_root_node(tree);

    std::vector<TSNode> definitions;
    collect_function_definitions(root, definitions);

    for (const auto& def : definitions) {
        std::string func_name = get_fully_qualified_name(def, source);
        if (func_name.empty()) continue;

        TSNode body = ts_node_child_by_field_name(def, "body", 4);
        if (ts_node_is_null(body)) continue;

        std::unordered_set<std::string> calls;
        find_calls(body, source, calls);

        call_graph[func_name] = std::vector<std::string>(calls.begin(), calls.end());
    }

    ts_tree_delete(tree);
    ts_parser_delete(parser);
}

void
Parser::traverse_directory(const std::filesystem::path& dir, std::unordered_map<std::string, std::vector<std::string>>& call_graph) {
    for (const auto& entry : std::filesystem::recursive_directory_iterator(dir)) {
        if (entry.is_regular_file()) {
            std::string ext = entry.path().extension().string();
            if (ext == ".cpp" || ext == ".cc" || ext == ".cxx" || ext == ".c++" || ext == ".h" || ext == ".hpp" || ext == ".hh" || ext == ".hxx") {
                try {
                    process_file(entry.path().string(), call_graph);
                } catch (const std::exception& e) {
                    std::cerr << "Error processing " << entry.path() << ": " << e.what() << std::endl;
                }
            }
        }
    }
}

void
Parser::print_call_graph(const std::unordered_map<std::string, std::vector<std::string>>& call_graph) {
    for (const auto& [func, calls] : call_graph) {
        std::cout << func << " calls:";
        std::vector<std::string> sorted_calls = calls;
        std::sort(sorted_calls.begin(), sorted_calls.end());
        for (const auto& call : sorted_calls) {
            std::cout << " " << call;
        }
        std::cout << std::endl;
    }
}
