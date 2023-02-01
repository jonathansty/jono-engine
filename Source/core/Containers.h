#pragma once

#include <vector>

template <typename T>
class SlotVector
{
public:
    struct Handle
    {
        uint32_t id;
        uint16_t gen;
    };

    T& Get(Handle const& h)
    {
        ASSERT(m_Generations[h.id] == h.gen);
        return m_Storage[h.id];
    }

    Handle Push(T obj)
    {
        size_t id = m_Storage.size();

        Handle h{};
        auto it = std::find_if(m_Storage.begin(), m_Storage.end(), [](T const& rhs)
            { return rhs == nullptr; });
        if (it != m_Storage.end())
        {
            size_t d = std::distance(m_Storage.begin(), it);

            *it = obj;
            ++m_Generations[d];

            h.gen = m_Generations[d];
            h.id = (uint32_t)d;
        }
        else
        {
            m_Generations.push_back(0);
            m_Storage.push_back(obj);

            h.id = (uint32_t)id;
            h.gen = 0;
        }
        return h;
    }

private:
    std::vector<uint16_t> m_Generations;
    std::vector<T> m_Storage;
};
