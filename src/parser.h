#pragma once
#include <tree_sitter/api.h>
#include <fstream>
#include <iostream>
#include <string>
#include <vector>
#include <unordered_map>
#include <unordered_set>
#include <filesystem>
#include <sstream>
#include <stack>
#include <algorithm>
#include <cstdlib>
#include <memory>

#include "language_handler.h"

class Parser {
private:
    std::unique_ptr<LanguageHandler> handler;

public:
    Parser(const std::string& language);

    std::string read_file(const std::string& path);

    static std::string get_node_text(const TSNode& node, const std::string& source);

    void process_file(const std::string& path, std::unordered_map<std::string, std::vector<std::string>>& call_graph);

    void traverse_directory(const std::filesystem::path& dir, std::unordered_map<std::string, std::vector<std::string>>& call_graph);

    void print_call_graph(const std::unordered_map<std::string, std::vector<std::string>>& call_graph);
};
