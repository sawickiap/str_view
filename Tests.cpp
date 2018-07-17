#include "str_view.hpp"
#include <thread>

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
    str_view empty;
    TEST(empty.empty());
    TEST(empty.length() == 0);
    TEST(empty.begin() == empty.end());
    TEST(empty.c_str() == string());

    str_view fromNull(nullptr);
    TEST(fromNull.empty());
    TEST(fromNull.length() == 0);
    TEST(fromNull.begin() == fromNull.end());
    TEST(fromNull.c_str() == string());

    str_view fromEmpty("");
    TEST(fromEmpty.empty());
    TEST(fromEmpty.length() == 0);
    TEST(fromEmpty.begin() == fromEmpty.end());
    TEST(fromEmpty.c_str() == string());

    str_view fromZeroLength("ABC", 0);
    TEST(fromZeroLength.empty());
    TEST(fromZeroLength.length() == 0);
    TEST(fromZeroLength.begin() == fromZeroLength.end());
    TEST(fromZeroLength.c_str() == string());

    str_view fromEmptyString(string("ABC"), 1, 0);
    TEST(fromEmptyString.empty());
    TEST(fromEmptyString.length() == 0);
    TEST(fromEmptyString.c_str() == string());
    TEST(fromEmptyString.begin() == fromEmptyString.end());
}

static void TestAdvancedConstruction()
{
    const char* sz = "ABCDE";
    str_view fromSz(sz);
    TEST(!fromSz.empty());
    TEST(fromSz.length() == strlen(sz));
    TEST(strcmp(fromSz.c_str(), sz) == 0);
    // Make sure original null-terminated string is returned.
    TEST(fromSz.c_str() == sz);

    str_view fromSzEnding = fromSz.substr(2);
    TEST(fromSzEnding.length() == 3);
    TEST(strcmp(fromSzEnding.c_str(), "CDE") == 0);
    // Make sure original null-terminated string is returned.
    TEST(fromSzEnding.c_str() == sz + 2);

    str_view fromS(sz, 3);
    TEST(fromS.length() == 3);
    TEST(strcmp(fromS.c_str(), "ABC") == 0);

    string str = "ABCDE";
    str_view fromStr(str);
    TEST(fromStr.length() == str.length());
    TEST(fromStr.c_str() == str);
    TEST(fromStr.c_str() == str.c_str());

    str_view fromSubStr(str, 0, 3);
    TEST(fromSubStr.length() == 3);
    TEST(strcmp(fromSubStr.c_str(), "ABC") == 0);
}

static void TestCopying()
{
    // Make substring, not null terminated, so that c_str() creates a local copy.
    str_view original = str_view("ABCDEF", 3);
    
    // Copy ctor
    str_view copyCtor = original;
    TEST(copyCtor.length() == 3);
    const char* ptr = copyCtor.c_str();
    TEST(ptr == string("ABC"));

    // Move ctor
    str_view moveCtor = std::move(copyCtor);
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
    str_view str = original;

    TEST(str[0] == 'A');
    TEST(str[1] == 'B');
    TEST(str.at(5) == 'F');

    // Test to_string
    string returned;
    str.to_string(returned);
    TEST(returned == original);

    // Test operator== and operator!=
    str_view str2 = returned;
    TEST(str2 == str);
    TEST(!(str2 != str));
    str2 = returned.substr(1);
    TEST(str2 != str);
    TEST(!(str2 == str));

    // Test comparisons
    {
        str_view empty = "";
        str_view first = "A";
        str_view second = "AA";
        str_view third = "B";
        str_view fourth = "a";

        TEST(empty < first);
        TEST(first < second);
        TEST(second < third);
        TEST(third < fourth);

        TEST(empty <= first);
        TEST(first <= second);
        TEST(second <= third);
        TEST(third <= fourth);

        TEST(first > empty);
        TEST(second > first);
        TEST(third > second);
        TEST(fourth > third);

        TEST(first >= empty);
        TEST(second >= first);
        TEST(third >= second);
        TEST(fourth >= third);

        TEST(empty == empty);
        TEST(first == first);
        TEST(second == second);
        TEST(third == third);
        TEST(fourth == fourth);

        TEST(empty <= empty);
        TEST(first <= first);
        TEST(second <= second);
        TEST(third <= third);
        TEST(fourth <= fourth);

        TEST(empty >= empty);
        TEST(first >= first);
        TEST(second >= second);
        TEST(third >= third);
        TEST(fourth >= fourth);

        TEST(!(empty != empty));
        TEST(!(second != second));
        TEST(empty != first);
        TEST(first != second);
        TEST(second != third);
        TEST(third != fourth);
    }
}

static void TestZeroCharacter()
{
    const char original[] = "ABC\0DEF";
    str_view str = str_view(original, 7);
    TEST(str.length() == 7);
    TEST(memcmp(original, str.c_str(), 8) == 0);
}

static void TestOtherMethods()
{
    // begin, end, front, back
    {
        const char* orig = "ABC";
        str_view s1 = str_view(orig);
    
        TEST(s1.begin() == orig);
        TEST(s1.end() == orig + 3);
        TEST(s1.front() == 'A');
        TEST(s1.back() == 'C');
    }

    // copy_to
    {
        const char* orig = "ABCDEF";
        str_view s1 = str_view(orig);

        char dst[6];
        s1.copy_to(dst);
        TEST(memcmp(orig, dst, 6) == 0);

        s1.copy_to(dst, 3);
        TEST(memcmp(orig + 3, dst, 3) == 0);

        s1.copy_to(dst, 0, 4);
        TEST(memcmp(orig, dst, 4) == 0);
    }

    // swap
    {
        const char* origSz = "ABCD";
        const string str = "EFG";

        str_view v1 = str_view(origSz);
        str_view v2 = str_view(str);
        
        using std::swap;
        swap(v1, v2);
        
        TEST(v2 == str_view(origSz));
        TEST(v1 == str_view(str));
    }

    // comparison
    {
        TEST(str_view("AAA").compare(str_view("B")) < 0);
        TEST(str_view("B").compare(str_view("AAA")) > 0);
        TEST(str_view("abcd").compare(str_view("abcd")) == 0);
        TEST(str_view("Z").compare(str_view("a")) < 0);
        TEST(str_view("a").compare(str_view("Z")) > 0);
        TEST(str_view("").compare(str_view("AAA")) < 0);
        TEST(str_view("AAA").compare(str_view("")) > 0);
        TEST(str_view("").compare(str_view(nullptr)) == 0);

        // case-insensitive
        TEST(str_view("AAA").compare(str_view("B"), false) < 0);
        TEST(str_view("B").compare(str_view("AAA"), false) > 0);
        TEST(str_view("abcd").compare(str_view("abcd"), false) == 0);
        TEST(str_view("Z").compare(str_view("a"), false) > 0); // !
        TEST(str_view("a").compare(str_view("Z"), false) < 0); // !
        TEST(str_view("").compare(str_view("AAA"), false) < 0);
        TEST(str_view("AAA").compare(str_view(""), false) > 0);
        TEST(str_view("").compare(str_view(nullptr), false) == 0);
    }

    // starts_with, ends_with
    {
        TEST(str_view("Ala ma kota").starts_with(str_view("Ala")));
        TEST(!str_view("Mateusz ma psy").starts_with(str_view("Ala")));
        TEST(str_view("Ala ma kota").starts_with(str_view()));
        TEST(!str_view().starts_with(str_view("Ala")));
        TEST(str_view("Ala ma kota").starts_with('A'));
        TEST(!str_view("Mateusz ma psy").starts_with('A'));

        TEST(str_view("Ala ma kota").ends_with(str_view("kota")));
        TEST(!str_view("Mateusz ma psy").ends_with(str_view("kota")));
        TEST(str_view("Ala ma kota").ends_with(str_view()));
        TEST(!str_view().ends_with(str_view("kota")));
        TEST(str_view("Ala ma kota").ends_with('a'));
        TEST(!str_view("Mateusz ma psy").ends_with('a'));

        // case-insensitive
        TEST(str_view("Ala ma kota").starts_with("ALA", false));
        TEST(str_view("Ala ma kota").starts_with("ala", false));
        TEST(!str_view("Mateusz ma psy").starts_with("Ala", false));
        TEST(str_view("Ala ma kota").starts_with('a', false));
        TEST(!str_view("Mateusz ma psy").starts_with('a', false));

        TEST(str_view("Ala ma kota").ends_with("KOTA", false));
        TEST(!str_view("Mateusz ma psy").ends_with("kota", false));
        TEST(str_view("Ala ma kota").ends_with('A', false));
        TEST(!str_view("Mateusz ma psy").ends_with('A', false));
    }

    // find
    {
        TEST(str_view("Ala ma kota").find("Ala") == 0);
        TEST(str_view("Ala ma kota").find("ma") == 4);
        TEST(str_view("Ala ma kota").find("kota") == 7);
        TEST(str_view("Ala ma kota").find("psy") == SIZE_MAX);
        TEST(str_view("Ala ma kota").find("") == 0);
        TEST(str_view("Ala ma kota").find("", 2) == 2);
        TEST(str_view("Ala ma kota").find("a", 4) == 5);
        TEST(str_view("Ala ma kota").find('A') == 0);
        TEST(str_view("Ala ma kota").find('Z') == SIZE_MAX);
        TEST(str_view("Ala ma kota").find('a', 4) == 5);
        TEST(str_view("Ala Ala Ala").find("Ala") == 0);
        TEST(str_view("Ala Ala Ala").find("Ala", 1) == 4);
    }

    // rfind
    {
        TEST(str_view("Ala ma kota").rfind("Ala") == 0);
        TEST(str_view("Ala ma kota").rfind("ma") == 4);
        TEST(str_view("Ala ma kota").rfind("kota") == 7);
        TEST(str_view("Ala ma kota").rfind("psy") == SIZE_MAX);
        TEST(str_view("Ala ma kota").rfind("", 2) == 2);
        TEST(str_view("Ala ma kota").rfind("a", 4) == 2);
        TEST(str_view("Ala ma kota").rfind('A') == 0);
        TEST(str_view("Ala ma kota").rfind('a') == 10);
        TEST(str_view("Ala ma kota").rfind('Z') == SIZE_MAX);
        TEST(str_view("Ala ma kota").rfind('a', 4) == 2);
        TEST(str_view("Ala Ala Ala").rfind("Ala") == 8);
        TEST(str_view("Ala Ala Ala").rfind("Ala", 7) == 4);
    }
}

static void TestMultithreading()
{
    const char* original = "ABCDEF";
    str_view substr = str_view(original, 4);

    constexpr size_t THREAD_COUNT = 32;
    std::thread threads[THREAD_COUNT];
    const char* ptrs[THREAD_COUNT];
    for(size_t i = 0; i < THREAD_COUNT; ++i)
    {
        threads[i] = std::thread([i, &substr, &ptrs]() {
            TEST(substr.length() == 4);
            const char* cstr = substr.c_str();
            TEST(strcmp(cstr, "ABCD") == 0);
            ptrs[i] = cstr;
        });
    }

    for(size_t i = 0; i < THREAD_COUNT; ++i)
    {
        threads[i].join();
    }
    for(size_t i = 0; i < THREAD_COUNT; ++i)
    {
        // Make sure the same pointer was returned on all threads.
        TEST(ptrs[i] == substr.c_str());
    }
}

int main()
{
    TestBasicConstruction();
    TestAdvancedConstruction();
    TestCopying();
    TestOperators();
    TestZeroCharacter();
    TestOtherMethods();
    TestMultithreading();
}
