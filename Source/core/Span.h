#pragma once

template <typename T>
class Span
{
public:
    Span(std::initializer_list<T> data);

    Span(std::vector<T>& v);

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
        return m_Data;
    }

    T const* data() const
    {
        return m_Data;
    }


private:
    T* m_Data;
    size_t m_Length;
};

template <typename T>
Span<T>::Span(std::initializer_list<T> data)
    : m_Data(data.begin())
    , m_Length(data.size())
{
}

template <typename T>
Span<T>::Span(std::vector<T>& v)
  : m_Data(v.data())
  , m_Length(v.size())
{
}
