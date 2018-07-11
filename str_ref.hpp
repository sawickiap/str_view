#pragma once

#include <string>
#include <atomic>
#include <algorithm> // for min, max
#include <memory> // for memcmp

#include <cassert>
#include <cstring>
#include <cstdint>

inline size_t tstrlen(const char* sz) { return strlen(sz); }
inline size_t tstrlen(const wchar_t* sz) { return wcslen(sz); }
inline void tstrcpy(char* dst, size_t dstCapacity, const char* src) { strcpy_s(dst, dstCapacity, src); }
inline void tstrcpy(wchar_t* dst, size_t dstCapacity, const wchar_t* src) { wcscpy_s(dst, dstCapacity, src); }

template<typename CharT>
class str_ref_template
{
public:
    typedef CharT CharT;
    typedef std::basic_string<CharT, std::char_traits<CharT>, std::allocator<CharT>> StringT;

    // Initializes to empty string.
    inline str_ref_template();
    
    // Initializes from a null-terminated string.
    // Null is acceptable. It means empty string.
    inline str_ref_template(const CharT* sz);
    // Initializes from not null-terminated string.
    // Null is acceptable if length is 0.
    inline str_ref_template(const CharT* str, size_t length);
    // Initializes from string with given length, with explicit statement that it is null-terminated.
    // Null is acceptable if length is 0.
    struct StillNullTerminated { };
    inline str_ref_template(const CharT* str, size_t length, StillNullTerminated);
    
    // Initializes from an STL string.
    // length can exceed actual str.length() - it then spans to the end of str.
    inline str_ref_template(const StringT& str, size_t offset = 0, size_t length = SIZE_MAX);

    // Copy constructor.
    inline str_ref_template(const str_ref_template<CharT>& src, size_t offset = 0, size_t length = SIZE_MAX);
    // Move constructor.
    inline str_ref_template(str_ref_template<CharT>&& src);
    
    inline ~str_ref_template();

    // Copy assignment operator.
    inline str_ref_template<CharT>& operator=(const str_ref_template<CharT>& src);
    // Move assignment operator.
    inline str_ref_template<CharT>& operator=(str_ref_template<CharT>&& src);

    inline bool empty() const { return m_Length == 0; }
    inline size_t length() const { return m_Length; }
    inline size_t size() const { return m_Length; }
    inline const CharT* data() const { return m_Begin; }
    inline const CharT* begin() const { return m_Begin; }
    inline const CharT* front() const { return m_Begin; }
    inline const CharT* end() const { return m_Begin + m_Length; }
    inline const CharT* back() const { return m_Begin + m_Length; }
    inline CharT operator[](size_t index) const { return m_Begin[index]; }
    inline CharT at(size_t index) const { return m_Begin[index]; }

    // Returns null-terminated string with contents of this object.
    // Possibly an internal copy.
    inline const CharT* c_str() const;

    // Returns substring of this string.
    str_ref_template<CharT> substr(size_t offset = 0, size_t length = SIZE_MAX);

    inline void to_string(StringT& dst) { dst.assign(begin(), end()); }

    /* TODO:
    swap
    operator==, !=, < > etc.
    starts_with, ends_with
    comarison case-insensitive
    Rename to str_view.
    Read more about std::basic_string_view
    */

    inline bool operator==(const str_ref_template<CharT>& rhs) const;
    inline bool operator!=(const str_ref_template<CharT>& rhs) const { return !operator==(rhs); }

private:
    size_t m_Length;
    const CharT* m_Begin;
    // Bit 0 set means string pointed by m_Begin + m_Length is null-terminated by itself.
    // Else: any others bits set mean pointer to array with null-terminated copy.
    mutable std::atomic<uintptr_t> m_NullTerminatedPtr;
};

typedef str_ref_template<char> str_ref;
typedef str_ref_template<wchar_t> wstr_ref;

template<typename CharT>
inline str_ref_template<CharT>::str_ref_template() :
	m_Length(0),
	m_Begin(nullptr),
	m_NullTerminatedPtr(0)
{
}

template<typename CharT>
inline str_ref_template<CharT>::str_ref_template(const CharT* sz) :
	m_Length(sz ? tstrlen(sz) : 0),
	m_Begin(sz),
	m_NullTerminatedPtr(sz ? 1 : 0)
{
}

template<typename CharT>
inline str_ref_template<CharT>::str_ref_template(const CharT* str, size_t length) :
	m_Length(length),
	m_Begin(length ? str : nullptr),
	m_NullTerminatedPtr(0)
{
}

template<typename CharT>
inline str_ref_template<CharT>::str_ref_template(const CharT* str, size_t length, StillNullTerminated) :
	m_Length(length),
	m_Begin(nullptr),
	m_NullTerminatedPtr(0)
{
    if(length)
    {
        m_Begin = str;
        m_NullTerminatedPtr = 1;
    }
    assert(m_Begin[m_Length] == (CharT)0); // Make sure it's really null terminated.
}

template<typename CharT>
inline str_ref_template<CharT>::str_ref_template(const StringT& str, size_t offset, size_t length) :
	m_Length(0),
	m_Begin(nullptr),
	m_NullTerminatedPtr(0)
{
	assert(offset <= str.length());
    m_Length = std::min(length, str.length() - offset);
    if(m_Length)
    {
        if(m_Length == str.length() - offset)
        {
            m_Begin = str.c_str() + offset;
            m_NullTerminatedPtr = 1;
        }
        else
            m_Begin = str.data() + offset;
    }
}

template<typename CharT>
inline str_ref_template<CharT>::str_ref_template(const str_ref_template<CharT>& src, size_t offset, size_t length) :
	m_Length(0),
	m_Begin(nullptr),
	m_NullTerminatedPtr(0)
{
	assert(offset <= src.m_Length);
    m_Length = std::min(length, src.m_Length - offset);
    if(m_Length)
    {
        m_Begin = src.m_Begin + offset;
        if(src.m_NullTerminatedPtr && m_Length == src.m_Length - offset)
            m_NullTerminatedPtr = 1;
    }
}

template<typename CharT>
inline str_ref_template<CharT>::str_ref_template(str_ref_template<CharT>&& src) :
	m_Length(src.m_Length),
	m_Begin(src.m_Begin),
	m_NullTerminatedPtr(src.m_NullTerminatedPtr.exchange(0))
{
	src.m_Begin = nullptr;
	src.m_Length = 0;
}

template<typename CharT>
inline str_ref_template<CharT>::~str_ref_template()
{
    uintptr_t v = m_NullTerminatedPtr;
	if(v > 1)
		delete[] (CharT*)v;
}

template<typename CharT>
inline str_ref_template<CharT>& str_ref_template<CharT>::operator=(const str_ref_template<CharT>& src)
{
	if(&src != this)
    {
        uintptr_t v = m_NullTerminatedPtr;
		if(v > 1)
			delete[] (CharT*)v;
		m_Begin = src.m_Begin;
		m_Length = src.m_Length;
		m_NullTerminatedPtr = src.m_NullTerminatedPtr == 1 ? 1 : 0;
    }
	return *this;
}

template<typename CharT>
inline str_ref_template<CharT>& str_ref_template<CharT>::operator=(str_ref_template<CharT>&& src)
{
	if(&src != this)
    {
        uintptr_t v = m_NullTerminatedPtr;
		if(v > 1)
			delete[] (CharT*)v;
		m_Begin = src.m_Begin;
		m_Length = src.m_Length;
		m_NullTerminatedPtr = src.m_NullTerminatedPtr.exchange(0);
		src.m_Begin = nullptr;
		src.m_Length = 0;
    }
	return *this;
}

template<typename CharT>
inline const CharT* str_ref_template<CharT>::c_str() const
{
    static const CharT nullChar = (CharT)0;
	if(empty())
		return &nullChar;
    uintptr_t v = m_NullTerminatedPtr;
	if(v == 1)
    {
        assert(m_Begin[m_Length] == (CharT)0); // Make sure it's really null terminated.
		return m_Begin;
    }
	if(v == 0)
    {
        CharT* nullTerminatedCopy = new CharT[m_Length + 1];
        assert(((uintptr_t)nullTerminatedCopy & 1) == 0); // Make sure allocated address is even.
		memcpy(nullTerminatedCopy, begin(), m_Length * sizeof(CharT));
		nullTerminatedCopy[m_Length] = (CharT)0;

        uintptr_t expected = 0;
        if(m_NullTerminatedPtr.compare_exchange_strong(expected, (uintptr_t)nullTerminatedCopy))
            return nullTerminatedCopy;
        else
        {
            // Other thread was quicker to set his copy to m_NullTerminatedPtr. Destroy mine, use that one.
            delete[] nullTerminatedCopy;
            return (const CharT*)expected;
        }
    }
	return (const CharT*)v;
}

template<typename CharT>
inline str_ref_template<CharT> str_ref_template<CharT>::substr(size_t offset, size_t length)
{
	assert(offset <= m_Length);
	length = std::min(length, m_Length - offset);
	// Result will be null-terminated.
	if(m_NullTerminatedPtr == 1 && length == m_Length - offset)
		return str_ref_template<CharT>(m_Begin + offset, length, StillNullTerminated());
	// Result will not be null-terminated.
	return str_ref_template<CharT>(m_Begin + offset, length);
}

template<typename CharT>
inline bool str_ref_template<CharT>::operator==(const str_ref_template<CharT>& rhs) const
{
    const size_t len = length();
    return len == rhs.length() &&
        memcmp(data(), rhs.data(), len) == 0;
}
