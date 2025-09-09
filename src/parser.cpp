#include "parser.h"
#include "cpp_handler.h"
#include "python_handler.h"

Parser::Parser(const std::string& language) {
    if (language == "C++") {
        handler = std::make_unique<CppHandler>();
    } else if (language == "Python") {
        handler = std::make_unique<PythonHandler>();
    } else {
        throw std::runtime_error("Unsupported language: " + language);
    }
}

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

void
Parser::process_file(const std::string& path, std::unordered_map<std::string, std::vector<std::string>>& call_graph) {
    std::string source = read_file(path);
    TSParser* parser = ts_parser_new();
    ts_parser_set_language(parser, handler->get_language());

    TSTree* tree = ts_parser_parse_string(parser, nullptr, source.c_str(), source.length());
    TSNode root = ts_tree_root_node(tree);

    std::vector<TSNode> definitions;
    handler->collect_function_definitions(root, definitions);

    for (const auto& def : definitions) {
        std::string func_name = handler->get_fully_qualified_name(def, source);
        if (func_name.empty()) continue;

        TSNode body = ts_node_child_by_field_name(def, "body", 4);
        if (ts_node_is_null(body)) continue;

        std::unordered_set<std::string> calls;
        handler->find_calls(body, source, calls);

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
            std::transform(ext.begin(), ext.end(), ext.begin(), ::tolower);
            auto exts = handler->get_extensions();
            if (std::find(exts.begin(), exts.end(), ext) != exts.end()) {
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
