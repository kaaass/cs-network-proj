#ifndef NEWS_BLOCKCHAIN_STRINGUTIL_H
#define NEWS_BLOCKCHAIN_STRINGUTIL_H

#include <vector>
#include <string>
#include "ByteBuffer.h"

class StringUtil {

public:

    /**
     * 字符串分割
     * @param str
     * @param delims
     * @return
     */
    static std::vector<std::string> split(const std::string &str, const std::string &delims);

    /**
     * 判断字符串是否结尾于
     * @param str
     * @param needle
     * @return
     */
    static bool endsWith(const std::string &str, const std::string &needle);
};


#endif //NEWS_BLOCKCHAIN_STRINGUTIL_H
