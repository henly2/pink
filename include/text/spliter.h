/*!
 * \file spliter.h
 *
 * \author heng.liu
 * \date 2018/01/25
 *
 * 
 */

#ifndef spliter_h__
#define spliter_h__

#include <string>
#include <vector>
#include <sstream>

namespace text{

    static void spliter_by_return(const std::string& str, std::vector<std::string>& vec)
    {
        std::stringstream ss;
        ss << str;
        std::string tmp;
        while (getline(ss, tmp, '\r')){
            vec.push_back(tmp.c_str());
        }
    }

} // namespace text


#endif // spliter_h__