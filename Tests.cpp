#include "str_ref.hpp"

#define TEST(expr)   do { \
    if(!(expr)) { \
        assert(0 && "TEST FAILED:" #expr); \
        printf("TEST FAILED: " #expr "\n"); \
    } \
    } while(false)

using std::string;
using std::wstring;

static void TestBasicConstruction()
{
    str_ref empty;
    TEST(empty.empty());
    TEST(empty.length() == 0);
    TEST(empty.begin() == empty.end());
    TEST(empty.c_str() == string());

    str_ref fromNull(nullptr);
    TEST(fromNull.empty());
    TEST(fromNull.length() == 0);
    TEST(fromNull.begin() == fromNull.end());
    TEST(fromNull.c_str() == string());

    str_ref fromEmpty("");
    TEST(fromEmpty.empty());
    TEST(fromEmpty.length() == 0);
    TEST(fromEmpty.begin() == fromEmpty.end());
    TEST(fromEmpty.c_str() == string());

    str_ref fromZeroLength("ABC", 0);
    TEST(fromZeroLength.empty());
    TEST(fromZeroLength.length() == 0);
    TEST(fromZeroLength.begin() == fromZeroLength.end());
    TEST(fromZeroLength.c_str() == string());

    str_ref fromEmptyString(string("ABC"), 1, 0);
    TEST(fromEmptyString.empty());
    TEST(fromEmptyString.length() == 0);
    TEST(fromEmptyString.c_str() == string());
    TEST(fromEmptyString.begin() == fromEmptyString.end());
}

static void TestAdvancedConstruction()
{
    const char* sz = "ABCDE";
    str_ref fromSz(sz);
    TEST(fromSz.length() == strlen(sz));
    TEST(strcmp(fromSz.c_str(), sz) == 0);
    // Make sure original null-terminated string is returned.
    TEST(fromSz.c_str() == sz);

    str_ref fromSzEnding = fromSz.substr(2);
    TEST(fromSzEnding.length() == 3);
    TEST(strcmp(fromSzEnding.c_str(), "CDE") == 0);
    // Make sure original null-terminated string is returned.
    TEST(fromSzEnding.c_str() == sz + 2);

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

static void TestCopying()
{
    // Make substring, not null terminated, so that c_str() creates a local copy.
    str_ref original = str_ref("ABCDEF", 3);
    
    // Copy ctor
    str_ref copyCtor = original;
    TEST(copyCtor.length() == 3);
    const char* ptr = copyCtor.c_str();
    TEST(ptr == string("ABC"));

    // Move ctor
    str_ref moveCtor = std::move(copyCtor);
    TEST(moveCtor.length() == 3);
    TEST(moveCtor.c_str() == string("ABC"));
    // Make sure that local copy was moved.
    TEST(moveCtor.c_str() == ptr);

    // Copy operator=
    copyCtor = moveCtor;
    TEST(copyCtor.length() == 3);
    TEST(copyCtor.c_str() == string("ABC"));
    // Create its own local copy.
    TEST(copyCtor.c_str() != ptr);
    
    // Move operator=
    ptr = copyCtor.c_str();
    moveCtor = std::move(copyCtor);
    TEST(moveCtor.length() == 3);
    TEST(moveCtor.c_str() == string("ABC"));
    // Make sure that local copy was moved.
    TEST(moveCtor.c_str() == ptr);
}

static void TestOperators()
{
    string original = "ABCDEF";
    str_ref str = original;

    TEST(str[0] == 'A');
    TEST(str[1] == 'B');
    TEST(str.at(5) == 'F');

    // Test to_string
    string returned;
    str.to_string(returned);
    TEST(returned == original);

    // Test operator== and operator!=
    str_ref str2 = returned;
    TEST(str2 == str);
    TEST(!(str2 != str));
    str2 = returned.substr(1);
    TEST(str2 != str);
    TEST(!(str2 == str));
}

static void TestZeroCharacter()
{
    const char original[] = "ABC\0DEF";
    str_ref str = str_ref(original, 7);
    TEST(str.length() == 7);
    TEST(memcmp(original, str.c_str(), 8) == 0);
}

int main()
{
    TestBasicConstruction();
    TestAdvancedConstruction();
    TestCopying();
    TestOperators();
    TestZeroCharacter();
}
