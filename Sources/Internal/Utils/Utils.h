#ifndef __DAVAENGINE_UTILS_H__
#define __DAVAENGINE_UTILS_H__

/**
	\defgroup utils Utilities
 */

#include "Base/BaseTypes.h"
#include "FileSystem/FilePath.h"
#include "Render/RenderBase.h"
#include <sstream>

#ifdef __DAVAENGINE_WIN_UAP__
#include <ppltasks.h>
#endif

namespace DAVA
{
int read_handler(void* ext, unsigned char* buffer, size_t size, size_t* length);

WideString WcharToWString(const wchar_t* s);

bool IsDrawThread();

inline WideString StringToWString(const String& s);
inline String WStringToString(const WideString& s);

WideString GetDeviceName();

void DisableSleepTimer();
void EnableSleepTimer();

void Split(const String& inputString, const String& delims, Vector<String>& tokens, bool skipDuplicated = false, bool addEmptyTokens = false);
void Merge(const Vector<String>& tokens, const char delim, String& outString);
void ReplaceBundleName(const String& newBundlePath);

template <class T>
T ParseStringTo(const String& str);

template <class T>
bool ParseFromString(const String& str, T& res);

template <class T>
void Swap(T& v1, T& v2);

/**
 \brief Function to compare strings case-insensitive
 \param[in] ext1 - first string 
 \param[in] ext2 - second string 
 \param[out] result of comparision 
 */
int32 CompareCaseInsensitive(const String& str1, const String& str2);

//implementation

inline WideString StringToWString(const String& s)
{
    WideString temp(s.length(), L' ');
    std::copy(s.begin(), s.end(), temp.begin());
    return temp;
}

inline void StringReplace(String& repString, const String& needle, const String& s)
{
    String::size_type lastpos = 0, thispos;
    while ((thispos = repString.find(needle, lastpos)) != String::npos)
    {
        repString.replace(thispos, needle.length(), s);
        lastpos = thispos + s.length();
    }
}

inline String WStringToString(const WideString& s)
{
    size_t len = s.length();
    String temp(len, ' ');
    //std::copy(s.begin(), s.end(), temp.begin());
    for (size_t i = 0; i < len; ++i)
        temp[i] = static_cast<char>(s[i]);
    return temp;
}

#if defined(__DAVAENGINE_WIN_UAP__)
inline Platform::String ^ StringToRTString(const String& s)
{
    return ref new Platform::String(StringToWString(s).c_str());
}

inline String RTStringToString(Platform::String ^ s)
{
    return WStringToString(s->Data());
}
#endif

template <class T>
bool FindAndRemoveExchangingWithLast(Vector<T>& array, const T& object)
{
    size_t size = array.size();
    for (size_t k = 0; k < size; ++k)
    {
        if (array[k] == object)
        {
            array[k] = array[size - 1];
            array.pop_back();
            return true;
        }
    }
    return false;
}

template <class T>
void RemoveExchangingWithLast(Vector<T>& array, size_t index)
{
    array[index] = array[array.size() - 1];
    array.pop_back();
}

template <class T>
T ParseStringTo(const String& str)
{
    T result;
    std::stringstream stream(str);
    stream >> result;
    return result;
}

template <class T>
bool ParseFromString(const String& str, T& result)
{
    std::stringstream stream(str);
    stream >> result;
    return (stream.eof() == true && stream.fail() == false);
}

template <class T>
void Swap(T& v1, T& v2)
{
    T temp = v1;
    v1 = v2;
    v2 = temp;
}

template <class T, std::size_t size>
class CircularArray
{
public:
    T& Next()
    {
        T& ret = elements[currentIndex];

        if ((++currentIndex) == elements.size())
            currentIndex = 0;

        return ret;
    }

    std::array<T, size> elements;

protected:
    std::size_t currentIndex = 0;
};

template <class T, uint32 _Size>
class RingArray
{
public:
    static_assert(((_Size - 1) & _Size) == 0 && _Size != 0, "Size of RingArray should be pow of two");

    RingArray() = default;
    RingArray(const RingArray& a)
    {
        memcpy(elements.data(), a.elements.data(), elements.size() * sizeof(T));
        mask = a.mask;
        head = a.head.load();
    }

    class iterator;
    class reverse_iterator;

    inline T& next()
    {
        return elements[(head++) & mask];
    }
    inline iterator begin()
    {
        return iterator(elements.data(), head & mask, mask);
    }
    inline iterator end()
    {
        return iterator(elements.data(), (head & mask) | (mask + 1), mask);
    }
    inline reverse_iterator rbegin()
    {
        return reverse_iterator(elements.data(), (head - 1) & mask, mask);
    }
    inline reverse_iterator rend()
    {
        return reverse_iterator(elements.data(), ((head - 1) & mask) | (mask + 1), mask);
    }
    inline size_t size()
    {
        return _Size;
    }

    class iterator
    {
    public:
        iterator() = default;
        ~iterator() = default;

        iterator(const reverse_iterator& it)
        {
            arrayData = it.arrayData;
            exmask = it.exmask;
            index = it.index ^ ((exmask >> 1) + 1);
        }

        inline iterator operator+(uint32 n)
        {
            iterator it(*this);
            it.index = (index + n) & exmask;
            return it;
        }
        inline iterator operator-(uint32 n)
        {
            iterator it(*this);
            it.index = (index - n) & exmask;
            return it;
        }
        inline iterator& operator++()
        {
            index = (index + 1) & exmask;
            return *this;
        }
        inline iterator operator++(int)
        {
            iterator prev = *this;
            ++(*this);
            return prev;
        }
        inline iterator& operator--()
        {
            index = (index - 1) & exmask;
            return *this;
        }
        inline iterator operator--(int)
        {
            iterator prev = *this;
            --(*this);
            return prev;
        }
        inline bool operator==(const iterator& it)
        {
            return index == it.index;
        }
        inline bool operator!=(const iterator& it)
        {
            return index != it.index;
        }
        inline T& operator*()
        {
            return arrayData[index & (exmask >> 1)];
        }
        inline T* operator->()
        {
            return &arrayData[index & (exmask >> 1)];
        }
        inline operator reverse_iterator() const
        {
            return reverse_iterator(arrayData, index ^ ((exmask >> 1) + 1), exmask >> 1);
        }

    protected:
        iterator(T* data, uint32 _index, uint32 _mask)
            : arrayData(data)
            , index(_index)
        {
            exmask = (_mask << 1) + 1;
        }

        T* arrayData = nullptr;
        uint32 index = 0;
        uint32 exmask = 0;

        friend class RingArray;
        friend class reverse_iterator;
    };

    class reverse_iterator
    {
    public:
        reverse_iterator() = default;
        ~reverse_iterator() = default;

        reverse_iterator(const iterator& it)
        {
            arrayData = it.arrayData;
            exmask = it.exmask;
            index = it.index ^ ((exmask >> 1) + 1);
        }

        inline reverse_iterator operator+(uint32 n)
        {
            reverse_iterator it(*this);
            it.index = (index - n) & exmask;
            return it;
        }
        inline reverse_iterator operator-(uint32 n)
        {
            reverse_iterator it(*this);
            it.index = (index + n) & exmask;
            return it;
        }
        inline reverse_iterator& operator++()
        {
            index = (index - 1) & exmask;
            return *this;
        }
        inline reverse_iterator operator++(int)
        {
            reverse_iterator prev = *this;
            ++(*this);
            return prev;
        }
        inline reverse_iterator& operator--()
        {
            index = (index + 1) & exmask;
            return *this;
        }
        inline reverse_iterator operator--(int)
        {
            reverse_iterator prev = *this;
            --(*this);
            return prev;
        }
        inline bool operator==(const reverse_iterator& it)
        {
            return index == it.index;
        }
        inline bool operator!=(const reverse_iterator& it)
        {
            return index != it.index;
        }
        inline T& operator*()
        {
            return arrayData[index & (exmask >> 1)];
        }
        inline T* operator->()
        {
            return &arrayData[index & (exmask >> 1)];
        }
        inline operator iterator() const
        {
            return iterator(arrayData, index ^ ((exmask >> 1) + 1), exmask >> 1);
        }

    protected:
        reverse_iterator(T* data, uint32 _index, uint32 _mask)
            : arrayData(data)
            , index(_index)
        {
            exmask = (_mask << 1) + 1;
        }

        T* arrayData = nullptr;
        uint32 index = 0;
        uint32 exmask = 0;

        friend class RingArray;
        friend class iterator;
    };

protected:
    std::array<T, _Size> elements;
    uint32 mask = _Size - 1;
    std::atomic<uint32> head = { 0 };
};

#if defined(__DAVAENGINE_IPHONE__) || defined(__DAVAENGINE_ANDROID__)

uint64 EglGetCurrentContext();
	
#endif

// Open the URL in external browser.
void OpenURL(const String& url);

String GenerateGUID();

#ifdef __DAVAENGINE_WIN_UAP__
template <typename T>
T WaitAsync(Windows::Foundation::IAsyncOperation<T> ^ async_operation)
{
    return concurrency::create_task(async_operation).get();
}
#endif
#ifdef __DAVAENGINE_WIN32__
Vector<String> GetCommandLineArgs();
#endif // __DAVAENGINE_WIN32__
};

#endif // __DAVAENGINE_UTILS_H__
