#pragma once

template <typename T>
class Span
{
public:

    Span(std::initializer_list<T> data)
      : m_Data(data.begin())
      , m_Length(data.size())
    {
    
    }

    Span(std::vector<T>& v)
        : m_Data(v.data())
        , m_Length(v.size())
    {
    
    }
    Span(std::vector<T> const& v)
      : m_Data(v.data())
      , m_Length(v.size())
    {
    
    }

    Span(T* data, size_t length)
      : m_Data(data)
      , m_Length(length)
    {
    }

    T* begin()
    {
        return m_Data;
    }

    T* end()
    {
        return m_Data + m_Length;
    }

    T const* begin() const
    {
        return m_Data;
    }

    T const* end() const
    {
        return m_Data + m_Length;
    }

    T* data() 
    {
        return const_cast<T*>(m_Data);
    }

    T const* data() const
    {
        return m_Data;
    }

    T& operator[](size_t idx)
    {
        ASSERT(idx < m_Length);
        // C++ Why?? Initializer list is sadgers :(
        return const_cast<T&>(m_Data[idx]);
    }

    template< std::enable_if<std::is_const<T>::value>>
    T const& operator[](size_t idx) const
    {
        ASSERT(idx < m_Length);
        return m_Data[idx];
    }

    size_t size() const { return m_Length; }


private:
    const T* m_Data;
    size_t m_Length;
};

