#pragma once

#include <vector>

struct SlotHandle
{
    uint32_t id;
    uint16_t gen;
};

template <typename T>
class SlotVector
{
public:

    T& Get(SlotHandle const& h)
    {
        ASSERT(m_Generations[h.id] == h.gen);
        return m_Storage[h.id];
    }

    void Grow(size_t size)
    {
        if(m_Generations.size() < size)
        {
            m_Generations.resize(size);
            m_Storage.resize(size);
        }
    }

    SlotHandle Push(T obj)
    {
        size_t id = m_Storage.size();

        SlotHandle h{};
        auto it = std::find_if(m_Storage.begin(), m_Storage.end(), [](T const& rhs)
            { return rhs == nullptr; });
        if (it != m_Storage.end())
        {
            size_t d = std::distance(m_Storage.begin(), it);

            *it = obj;
            h.gen = m_Generations[d];
            h.id = (uint32_t)d;
        }
        else
        {
            FAILMSG("Capacity exceeded! Ensure SlotVector is big enough to avoid mem allocs");
            m_Generations.push_back(0);
            m_Storage.push_back(obj);

            h.id = (uint32_t)id;
            h.gen = 0;
        }
        return h;
    }

    bool Erase(SlotHandle h)
    {
        if (m_Generations[h.id] == h.gen)
        {
            m_Storage[h.id] = nullptr;

            ++m_Generations[h.id];
            return true;
        }
        ASSERT("Incorrect generation used for this handle.");
        return false;
    }

private:
    std::vector<uint16_t> m_Generations;
    std::vector<T> m_Storage;
};

