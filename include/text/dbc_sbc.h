/*!
 * \file dbc_sbc.h
 *
 * \author heng.liu
 * \date 2018/01/24
 *
 * °ë½Ç/È«½Ç×ª»»
 * http://www.cnblogs.com/foundwant/p/5713576.html
 */
#ifndef dbc_sbc_h__
#define dbc_sbc_h__

#include <string>

namespace text{

    // °ë½Ç->È«½Ç
    static void dbc_2_sbc(const std::wstring& in, std::wstring& out)
    {
        out.clear();

        size_t length = in.length();
        if (length <= 0)
            return;

        for (size_t i = 0; i < length; i++)
        {
            wchar_t ch = in.at(i);
            if (ch <= 0x7e && ch >= 0x21)
            {
                out += (wchar_t)(ch + 0xfee0);
            }
            // space
            else if (ch == 0x20)
            {
                out += (wchar_t)0x3000;
            }
            else
            {
                out += ch;
            }
        }
    }

    // È«½Ç->°ë½Ç
    static void sbc_2_dbc(const std::wstring& in, std::wstring& out)
    {
        out.clear();

        size_t length = in.length();
        if (length <= 0)
            return;

        for (size_t i = 0; i < length; i++)
        {
            wchar_t ch = in.at(i);
            if (ch <= 0xff5e && ch >= 0xff01)
            {
                out += (wchar_t)(ch - 0xfee0);
            }
            else if (ch == 0x3000)
            {
                out += (wchar_t)0x20;
            }
            else
            {
                out += ch;
            }
        }
    }

} // namespace text

#endif // dbc_sbc_h__