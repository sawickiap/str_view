#define STR_VIEW_CPP17 1
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

    string returned2;
    str.to_string(returned2, 1, 3);
    TEST(returned2 == "BCD");
    str.to_string(returned2, 3);
    TEST(returned2 == "DEF");

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

static void TestRemovePrefixSuffix()
{
    // Fixed length
    {
        const char* orig = "ABCDEF--";

        str_view v1 = str_view(orig, 6);
        v1.remove_prefix(0);
        TEST(v1 == "ABCDEF");
        TEST(strcmp(v1.c_str(), "ABCDEF") == 0);
        v1.remove_prefix(2);
        TEST(v1 == "CDEF");
        TEST(strcmp(v1.c_str(), "CDEF") == 0);
        v1.remove_prefix(4);
        TEST(v1.empty());
        TEST(strcmp(v1.c_str(), "") == 0);

        str_view v2 = str_view(orig, 6);
        v2.remove_suffix(0);
        TEST(v2 == "ABCDEF");
        TEST(strcmp(v2.c_str(), "ABCDEF") == 0);
        v2.remove_suffix(2);
        TEST(v2 == "ABCD");
        TEST(strcmp(v2.c_str(), "ABCD") == 0);
        v2.remove_suffix(4);
        TEST(v2.empty());
        TEST(strcmp(v2.c_str(), "") == 0);
    }

    // Null-terminated
    {
        const char* orig = "ABCDEF";

        str_view v1 = str_view(orig);
        v1.remove_prefix(0);
        TEST(v1 == "ABCDEF");
        TEST(v1.c_str() == orig);
        v1.remove_prefix(2);
        TEST(v1 == "CDEF");
        TEST(v1.c_str() == orig + 2);
        v1.remove_prefix(4);
        TEST(v1.empty());
        TEST(strcmp(v1.c_str(), "") == 0);

        str_view v2 = str_view(orig);
        v2.remove_suffix(0);
        TEST(v2 == "ABCDEF");
        TEST(v2.c_str() == orig);
        v2.remove_suffix(2);
        TEST(v2 == "ABCD");
        TEST(strcmp(v2.c_str(), "ABCD") == 0);
        v2.remove_suffix(4);
        TEST(v2.empty());
        TEST(strcmp(v2.c_str(), "") == 0);
    }

    // std::string
    {
        std::string orig = "ABCDEF";

        str_view v1 = str_view(orig);
        v1.remove_prefix(0);
        TEST(v1 == "ABCDEF");
        TEST(v1.c_str() == orig.c_str());
        v1.remove_prefix(2);
        TEST(v1 == "CDEF");
        TEST(v1.c_str() == orig.c_str() + 2);
        v1.remove_prefix(4);
        TEST(v1.empty());
        TEST(strcmp(v1.c_str(), "") == 0);

        str_view v2 = str_view(orig);
        v2.remove_suffix(0);
        TEST(v2 == "ABCDEF");
        TEST(v2.c_str() == orig.c_str());
        v2.remove_suffix(2);
        TEST(v2 == "ABCD");
        TEST(strcmp(v2.c_str(), "ABCD") == 0);
        v2.remove_suffix(4);
        TEST(v2.empty());
        TEST(strcmp(v2.c_str(), "") == 0);
    }
}

#if STR_VIEW_CPP17

static void TestCpp17()
{
    std::string orig = "ABCDEF";
    std::string_view stlView1 = std::string_view(orig);
    str_view v1 = str_view(stlView1);
    TEST(v1 == "ABCDEF");
    std::string_view stlView2;
    v1.to_string_view(stlView2);
    TEST(stlView2 == "ABCDEF");

    v1 = str_view(stlView1, 1, 4);
    TEST(v1 == "BCDE");
    v1.to_string_view(stlView2, 1, 2);
    TEST(stlView2 == "CD");
}

#endif // #if STR_VIEW_CPP17

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

    // find_first_of
    {
        TEST(str_view("Ala ma kota").find_first_of(str_view(nullptr)) == SIZE_MAX);
        TEST(str_view("Ala ma kota").find_first_of(str_view("Ala")) == 0);
        TEST(str_view("Ala ma kota").find_first_of(str_view("maA")) == 0);
        TEST(str_view("Ala ma kota").find_first_of(str_view("m")) == 4);
        TEST(str_view("Ala ma kota").find_first_of(str_view("zm")) == 4);
        TEST(str_view("Ala ma kota").find_first_of(str_view("ZzXx")) == SIZE_MAX);
        TEST(str_view("Ala ma kota").find_first_of(str_view("a")) == 2);
        TEST(str_view("").find_first_of("ABab") == SIZE_MAX);
        TEST(str_view("Ala ma kota").find_first_of(str_view("ZzXxa"), 3) == 5);
    }

    // find_last_of
    {
        TEST(str_view("Ala ma kota").find_last_of(str_view(nullptr)) == SIZE_MAX);
        TEST(str_view("Ala ma kota").find_last_of(str_view("A")) == 0);
        TEST(str_view("Ala ma kota").find_last_of(str_view("maA")) == 10); // !
        TEST(str_view("Ala ma kota").find_last_of(str_view("m")) == 4);
        TEST(str_view("Ala ma kota").find_last_of(str_view("zm")) == 4);
        TEST(str_view("Ala ma kota").find_last_of(str_view("ZzXx")) == SIZE_MAX);
        TEST(str_view("Ala ma kota").find_last_of(str_view("a")) == 10); // !
    }

    // find_first_not_of
    {
        TEST(str_view("Ala ma kota").find_first_not_of(str_view("Ala mkot")) == SIZE_MAX);
        TEST(str_view("Ala ma kota").find_first_not_of(str_view("Z")) == 0);
        TEST(str_view("Ala ma kota").find_first_not_of(str_view("Ala")) == 3);
        TEST(str_view("Ala ma kota").find_first_not_of(str_view("ma "), 3) == 7);
        TEST(str_view("Ala ma kota").find_first_not_of(str_view(nullptr)) == SIZE_MAX);
        TEST(str_view(nullptr).find_first_not_of(str_view("Ala")) == SIZE_MAX);
    }

    // find_last_not_of
    {
        TEST(str_view("Ala ma kota").find_last_not_of(str_view("Ala mkot")) == SIZE_MAX);
        TEST(str_view("Ala ma kota").find_last_not_of(str_view("Z")) == 10); // !
        TEST(str_view("Ala ma kota").find_last_not_of(str_view("Ala")) == 9); // !
        TEST(str_view("Ala ma kota").find_last_not_of(str_view("ma "), 9) == 9);
        TEST(str_view("Ala ma kota").find_last_not_of(str_view("")) == SIZE_MAX);
        TEST(str_view("").find_last_not_of(str_view("Ala")) == SIZE_MAX);
    }
}

static void TestUnicode()
{
    wstr_view fromNull = wstr_view(nullptr);
    TEST(fromNull.empty());
    TEST(fromNull.length() == 0);
    TEST(fromNull.size() == 0);
    TEST(fromNull.begin() == fromNull.end());
    TEST(fromNull == wstr_view(L""));
    TEST(wcscmp(fromNull.c_str(), L"") == 0);

    wstr_view fromSz = wstr_view(L"Ala ma kota");
    TEST(!fromSz.empty());
    TEST(fromSz.size() == 11);
    TEST(fromSz.length() == 11);
    TEST(*fromSz.begin() == L'A');
    TEST(fromSz.back() == L'a');
    TEST(fromSz == wstr_view(L"Ala ma kota"));
    TEST(wcscmp(fromSz.c_str(), L"Ala ma kota") == 0);

    wstring stl = L"Ala ma kota";
    wstr_view fromStl = wstr_view(stl);
    TEST(!fromStl.empty());
    TEST(fromStl.size() == 11);
    TEST(fromStl.length() == 11);
    TEST(*fromStl.begin() == L'A');
    TEST(fromStl.back() == L'a');
    TEST(fromStl == wstr_view(L"Ala ma kota"));
    TEST(wcscmp(fromStl.c_str(), L"Ala ma kota") == 0);
}

static void TestNatvis()
{
    string s = "Mateusz ma psy";

    str_view fromNull = str_view(nullptr);
    str_view fromSz = str_view("Ala ma kota");
    str_view fromStl = str_view(s);

    str_view fromSzSub = str_view(fromSz, 4, 2);
    str_view fromStlSub = str_view(fromStl, 4, 2);

    wstr_view unicode = wstr_view(L"Ala ma kota Unicode");

    // Place breakpoint here and check in Visual Studio debugger whether natvis is working.
    int DEBUG = 1;
}

void Foo1(const str_view& v)
{
    printf("String is: %s\n", v.c_str());
}

static void TestDocumentationSamples()
{
    // Basic construction

    Foo1(str_view()); // Passed ""

    Foo1("Ala ma kota"); // Passed "Ala ma kota"

    {
        char sz[32];
        sprintf_s(sz, "Number is %i", 7);
        Foo1(sz); // Passed "Number is 7"
    }

    {
        std::string str = "Ala ma kota";
        Foo1(str); // Passed "Ala ma kota"
    }

    // Advanced construction

    Foo1(str_view(nullptr)); // Passed ""

    {
        char array[4] = { 'A', 'B', 'C', 'D' };
        Foo1(str_view(array,
            4)); // length
        // Passed "ABCD"
    }

    {
        const char* sz = "Ala ma kota";
        Foo1(str_view(sz + 4,
            2)); // length
        // Passed "ma"
    }

    {
        std::string str = "Ala ma kota";
        Foo1(str_view(str,
            4, // offset
            2)); // length
        // Passed "ma"
    }

    {
        str_view orig = "Ala ma kota";
        Foo1(orig.substr(
            4)); // offset
        // Passed "ma kota" - substring from offset 4 to the end.
        Foo1(orig.substr(
            0, // offset
            3)); // length
        // Passed "Ala" - substring limited to 3 characters.
        Foo1(orig.substr(
            4, // offset
            2)); // length
        // Passed "ma"
    }

    // Using string view

    {
        str_view v1 = str_view("aaa");
        str_view v2 = str_view("BBB");
        int r = v1.compare(v2,
            false); // case_sensitive
        // r is -1 because v1 goes before v2 when compared in case-insensitive way.
        printf("r = %i\n", r);
    }

    {
        str_view v = str_view("Ala ma kota");
        // Prints "Ala ma kota"
        for(char ch : v)
            printf("%c", ch);
    }
    printf("\n");

    {
        str_view v = str_view("Ala ma kota");
        str_view sub_v = v.substr(4, 2);
        printf("sub_v is: %s\n", sub_v.c_str()); // Prints "sub_v is: ma"
    }

    // Performance
    // Use debugger to confirm described behavior.

    {
        const char* sz = "Ala ma kota";
        str_view v = str_view(sz);

        // empty() peeks only first character. Length still unknown.
        printf("Empty: %s\n", v.empty() ? "true" : "false"); // Prints "Empty: false"
        // length() calculates length on first call.
        printf("Length: %zu\n", v.length()); // Prints "Length: 11"
        // c_str() trivially returns original pointer.
        printf("String is: %s\n", v.c_str()); // Prints "Ala ma kota"
        TEST(v.c_str() == sz);
    }

    {
        std::string s = "Ala ma kota";
        str_view v = str_view(s);

        // c_str() returns pointer returned from original s.c_str().
        printf("String is: %s\n", v.c_str());
        TEST(v.c_str() == s.c_str());
        // Length is explicitly known from s, so empty() trivially checks if it's not 0.
        printf("Empty: %s\n", v.empty() ? "true" : "false");
        // Length is explicitly known from s, so length() trivially returns it.
        printf("Length: %zu\n", v.length());
    }

    {
        const char* sz = "Ala ma kota";
        str_view v = str_view(sz + 4, 2);

        // c_str() creates and returns local, null-terminated copy.
        printf("String is: %s\n", v.c_str()); // Prints "ma"
        TEST(v.c_str() != sz);
        // Length is explicitly known, so empty() trivially checks if it's not 0.
        printf("Empty: %s\n", v.empty() ? "true" : "false"); // Prints "Empty: false"
        // Length is explicitly known, so length() trivially returns it.
        printf("Length: %zu\n", v.length()); // Prints "Length: 2"
    }

    {
        str_view vFull = str_view("Ala ma kota");
        str_view vBegin = vFull.substr(
            0, // offset
            3); // length

        // Substring is not null-terminated. c_str() creates and returns local, null-terminated copy.
        printf("String is: %s\n", vBegin.c_str()); // Prints "Ala"
        TEST(vBegin.c_str() != vFull.c_str());
        // Length is explicitly known, so empty() trivially checks if it's not 0.
        printf("Empty: %s\n", vBegin.empty() ? "true" : "false"); // Prints "Empty: false"
        // Length is explicitly known, so length() trivially returns it.
        printf("Length: %zu\n", vBegin.length()); // Prints "Length: 3"
    }

    {
        str_view vFull = str_view("Ala ma kota");
        str_view vEnd = vFull.substr(
            7); // offset

        // Substring is null-terminated. c_str() returns original pointer, adjusted by offset.
        printf("String is: %s\n", vEnd.c_str()); // Prints "kota"
        TEST(vEnd.c_str() == vFull.c_str() + 7);
        // Length is still unknown. empty() peeks only first character.
        printf("Empty: %s\n", vEnd.empty() ? "true" : "false"); // Prints "Empty: false"
        // length() calculates length on first call.
        printf("Length: %zu\n", vEnd.length()); // Prints "Length: 4"
    }
}

int main()
{
    TestBasicConstruction();
    TestAdvancedConstruction();
    TestCopying();
    TestOperators();
    TestRemovePrefixSuffix();
#if STR_VIEW_CPP17
    TestCpp17();
#endif
    TestZeroCharacter();
    TestOtherMethods();
    TestUnicode();
    TestNatvis();
    TestDocumentationSamples();
}
