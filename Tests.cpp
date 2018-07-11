#include "str_ref.hpp"

#define TEST(expr)   do { \
    if(!(expr)) { \
        assert(0 && "TEST FAILED:" #expr); \
        printf("TEST FAILED: " #expr "\n"); \
    } \
    } while(false)

using std::string;
using std::wstring;

int main()
{
    str_ref empty;
    TEST(empty.empty());
    TEST(empty.length() == 0);
    TEST(empty.c_str() == string());
    TEST(empty.begin() == empty.end());

    str_ref fromNull(nullptr);
    TEST(fromNull.empty());
    TEST(fromNull.length() == 0);
    TEST(fromNull.c_str() == string());
    TEST(fromNull.begin() == fromNull.end());

    str_ref fromEmpty("");
    TEST(fromEmpty.empty());
    TEST(fromEmpty.length() == 0);
    TEST(fromEmpty.c_str() == string());
    TEST(fromEmpty.begin() == fromEmpty.end());

    str_ref fromZeroLength("ABC", 0);
    TEST(fromZeroLength.empty());
    TEST(fromZeroLength.length() == 0);
    TEST(fromZeroLength.c_str() == string());
    TEST(fromZeroLength.begin() == fromZeroLength.end());

    str_ref fromEmptyString(string("ABC"), 1, 0);
    TEST(fromEmptyString.empty());
    TEST(fromEmptyString.length() == 0);
    TEST(fromEmptyString.c_str() == string());
    TEST(fromEmptyString.begin() == fromEmptyString.end());

    const char* sz = "ABCDE";
    str_ref fromSz(sz);
    TEST(fromSz.length() == strlen(sz));
    TEST(strcmp(fromSz.c_str(), sz) == 0);
    TEST(fromSz.c_str() == sz);

    str_ref fromSzEnding = fromSz.substr(2);
    TEST(fromSzEnding.length() == 3);
    TEST(strcmp(fromSzEnding.c_str(), "CDE") == 0);

    str_ref fromS(sz, 3);
    TEST(fromS.length() == 3);
    TEST(strcmp(fromS.c_str(), "ABC") == 0);

    string str = "ABCDE";
    str_ref fromStr(str);
    TEST(fromStr.length() == str.length());
    TEST(fromStr.c_str() == str);
    TEST(fromStr.c_str() == str.c_str());

    str_ref fromSubStr(str, 0, 3);
    TEST(fromSubStr.length() == 3);
    TEST(strcmp(fromSubStr.c_str(), "ABC") == 0);
}
