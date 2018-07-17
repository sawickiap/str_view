# str_view

Null-termination-aware string-view class for C++.

# Introduction

str_view is a small library for C++.
It offers a convenient and optimized class that represents view into a character string.
It has a form of a single header file: `str_view.hpp`, which you can just add to your project.
All the members are defined as inline, so no compilation of additional CPP files or linking with additional libraries is required.

str_view depends only on standard C and C++ library.
It has been developed and tested under Windows using Microsoft Visual Studio Communiity 2017 Version 15.7.1, but it should work in other compilers and platforms as well. If you find any compatibility issues, please let me know.
It works in both 32-bit and 64-bit code.

The class is defined as `str_view_template`, because it's a template that can be parametrized with character types. Two typedefs are provided:

- `str_view` - compatible with `char` and `std::string` (single-byte or UTF-8 strings).
- `wstr_view` - compatible with `wchar_t` and `std::wstring` (2-byte UTF-16 strings, also called Unicode in WinAPI).

All the examples below use `str_view`, but everything what is described here applies also to `wstr_view` and Unicode strings.

An object of `str_view` class represents a view into an externally owned character string, or piece of thereof. It stores pointer to its beginning and its length. The original string must remain alive and unchanged as long as the view object is alive and points to it. Otherwise behavior is undefined.

String pointed by `str_view` is immutable. It means it is possible to only read properties (like length) and characters of the pointer string, but not to alter it in any way through the view, either its length or individual characters. Defining `str_view` object as `const` only means that the view itself is immutable and cannot be changed to point to a different string.

It is best used for passing parameters into library functions that can take strings of unknown origin. Let's consider an example library function:

```cpp
void Foo(const str_view& v);
```

# Creating string view

There are multiple ways in which a string view object can be constructed, as there are multiple overloaded constructors. None of them are marked as `explicit`, so implicit conversion is allowed.

## Basic construction

Default constructor initializes valid, but empty string (one with length of 0).

```cpp
Foo(str_view()); // Passed ""
```

View can be created from a null-terminated string. It can be either string literal...

```cpp
Foo("Ala ma kota"); // Passed "Ala ma kota"
```

...or dynamically created C string.

```cpp
char sz[32];
sprintf(sz, "Number is %i", 7);
Foo(sz); // Passed "Number is 7"
```

View can be created from STL string.

```cpp
std::string str = "Ala ma kota";
Foo(str); // Passed "Ala ma kota"
```

The class also supports copy constructor, move constructor, assignment operator, move assignment operator, `swap` method and global `swap` function. It may be efficient to pass and return such objects by value, not necessarily `const&`.

## Advanced construction

Passing `null` as source pointer is also valid. It initializes view to an empty string.

```cpp
Foo(str_view(nullptr)); // Passed ""
```

View can also be created from an array of characters, by specifying pointer to first character and length (number of characters). Such string doesn't need to be null-terminated.

```cpp
char array[4] = { 'A', 'B', 'C', 'D' };
Foo(str_view(array,
    4)); // length
// Passed "ABCD"
```

View can be created to point to only a piece of original string.

```cpp
const char* sz = "Ala ma kota";
Foo(str_view(sz + 4,
    2)); // length
// Passed "ma"
```

It can also point to a piece of STL string. Still no copy is performed. The view object still refers to the original string.

```cpp
std::string str = "Ala ma kota";
Foo(str_view(str,
    4, // offset
    2)); // length
// Passed "ma"
```

The class offers powerful `substr` method that returns new view, which may point to a piece of source view.

```cpp
str_view orig = "Ala ma kota";
Foo(orig.substr(
    4)); // offset
// Passed "ma kota" - substring from offset 0 to the end.
Foo(orig.substr(
    0, // offset
    3)); // length
// Passed "Ala" - substring limited to 3 characters.
Foo(orig.substr(
    4, // offset
    2)); // length
// Passed "ma"
```

# Using string view

`str_view` class offers a convenient set of methods and operators similar to `std::string` and `std::string_view` from C++17, but it's not fully compatible with any of them.

Call `length()` to retrieve length of string view (number of charecters). Alternative name is `size()`, but it's not recommended because its name may be misleading - it may suggest size in bytes not in characters.

`empty()` method returns `true` when the string is empty (has length of 0).

`data()` method returns a pointer to the underlying character array. Characters are also laid out sequentially in memory, so the pointer may be used as normal C array.

`begin()` and `end()` methods return pointers to the first character and to the character following the last character of the view, respectively. Together they form a range that may be used e.g. with STL algorithms that expect random-access or any other type of iterators.

Individual characters can be read using overloaded `operator[]`. Alternative syntax is `at()` method.

First character can also be fetched using `front()` method, and last character is returned by `back()` method.

Pointed string can be copied to a specified destination array of characters using method `copy_to`, or to an STL string using method `to_string()`.

String views can be compared lexicographically using all comparison operators, like `==`, `!=`, `<`, `<=` etc. There is also more powerful method `compare()` which returns negative integer, zero, or positive integer, depending on the result of the comparison. Comparison can be made case-insensitive.

```cpp
str_view v1 = str_view("aaa");
str_view v2 = str_view("BBB");
int r = v1.compare(v2,
    false); // case_sensitive
// r is -1 because v1 goes before v2 when compared in case-insensitive way.
```

String view can also be searched and checked using methods: `starts_with()` and `ends_with()` (also supports case-insensitive comparison), `find()`, `rfind()`, `find_first_of()`, `find_last_of()`, `find_first_not_of()`, `find_last_not_of()`.

Last but not least, strings passed in a C++ program often need to end up as null-terminated C strings to be passed to some external libraries, so the class offers `c_str()` method similar to `std::string` that returns pointer to such null-terminated string. It may be either pointer to the original string if it's null terminated, or an internal copy. The copy is valid as long as `str_view` object is alive and it's not modified to point to a different string. It is automatically destroyed.

```cpp
str_view v = str_view("Ala ma kota");
str_view sub_v = v.substr(4, 2);
printf("sub_v is: %s", sub_v.c_str()); // Prints "sub_v is: ma"
```
