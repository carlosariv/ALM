#pragma once

#include <string>
#include <vector>
#include <span>

struct CommandLineOpts {
    std::vector<std::string> filenames;
    bool dump_ast = false;
};


