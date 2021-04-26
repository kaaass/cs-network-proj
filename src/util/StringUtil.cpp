#include "StringUtil.h"

/*
 * Code from: http://quick-bench.com/mhyUI8Swxu3As-RafVUSVfEZd64
 */
std::vector<std::string> StringUtil::split(const std::string &str, const std::string &delims = " ") {
    std::vector<std::string> output;
    size_t first = 0;
    while (first < str.size()) {
        const auto second = str.find_first_of(delims, first);
        if (first != second) {
            output.emplace_back(str.substr(first, second - first));
        }
        if (second == std::string::npos)
            break;
        first = second + 1;
    }
    return output;
}

bool StringUtil::endsWith(const std::string &str, const std::string &needle) {
    return str.rfind(needle) == (str.length() - needle.length());
}
