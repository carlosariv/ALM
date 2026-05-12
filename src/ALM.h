#pragma once

#include <string>
#include <vector>
#include <span>

struct CommandLineOpts {
    std::vector<std::string> filenames;

    void process_command_line_args(std::span<char*, std::dynamic_extent> args);
};


