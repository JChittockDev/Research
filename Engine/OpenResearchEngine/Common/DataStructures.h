#pragma once
#include <algorithm>
#include <d3d12.h>
#include <unordered_map>
#include <vector>
#include <memory>
#include <string>
#include <stdexcept>
#include <wrl/client.h>
#include <DirectXMath.h>

namespace DataStructures
{
    // TrackedMap stores values by string key and tracks the insertion order of
    // each key via two auxiliary maps. All three maps are kept in sync on every
    // mutation. Insertion order is 0-based and monotonically increasing; gaps
    // are left when entries are removed.
    template<typename T>
    class TrackedMap
    {
    public:
        // Insert or update a value.
        // If the key is new it is assigned the next insertion index.
        // If the key already exists its insertion index is preserved.
        void Insert(const std::string& key, const T& value)
        {
            auto [orderIt, inserted] = m_order.try_emplace(key, m_nextOrder);
            if (inserted)
            {
                m_indexToKey.try_emplace(m_nextOrder, key);
                ++m_nextOrder;
            }
            m_data.insert_or_assign(key, value);
        }

        void Insert(const std::string& key, T&& value)
        {
            auto [orderIt, inserted] = m_order.try_emplace(key, m_nextOrder);
            if (inserted)
            {
                m_indexToKey.try_emplace(m_nextOrder, key);
                ++m_nextOrder;
            }
            m_data.insert_or_assign(key, std::move(value));
        }

        // Returns true if the key exists.
        bool Contains(const std::string& key) const
        {
            return m_data.find(key) != m_data.end();
        }

        // Returns a reference to the value; throws std::out_of_range if absent.
        T& Get(const std::string& key)
        {
            auto it = m_data.find(key);
            if (it == m_data.end())
                throw std::out_of_range("TrackedMap: key not found: " + key);
            return it->second;
        }

        const T& Get(const std::string& key) const
        {
            auto it = m_data.find(key);
            if (it == m_data.end())
                throw std::out_of_range("TrackedMap: key not found: " + key);
            return it->second;
        }

        // Returns a pointer to the value, or nullptr if the key is absent.
        T* TryGet(const std::string& key)
        {
            auto it = m_data.find(key);
            return it != m_data.end() ? &it->second : nullptr;
        }

        const T* TryGet(const std::string& key) const
        {
            auto it = m_data.find(key);
            return it != m_data.end() ? &it->second : nullptr;
        }

        // Returns a reference to the value at the given insertion index.
        // Throws std::out_of_range if no entry with that index exists.
        T& GetByIndex(size_t index)
        {
            auto it = m_indexToKey.find(index);
            if (it == m_indexToKey.end())
                throw std::out_of_range("TrackedMap: insertion index not found: " + std::to_string(index));
            return m_data.at(it->second);
        }

        const T& GetByIndex(size_t index) const
        {
            auto it = m_indexToKey.find(index);
            if (it == m_indexToKey.end())
                throw std::out_of_range("TrackedMap: insertion index not found: " + std::to_string(index));
            return m_data.at(it->second);
        }

        // Returns a pointer to the value at the given insertion index,
        // or nullptr if no entry with that index exists (e.g. it was removed).
        T* TryGetByIndex(size_t index)
        {
            auto it = m_indexToKey.find(index);
            return it != m_indexToKey.end() ? &m_data.at(it->second) : nullptr;
        }

        const T* TryGetByIndex(size_t index) const
        {
            auto it = m_indexToKey.find(index);
            return it != m_indexToKey.end() ? &m_data.at(it->second) : nullptr;
        }

        // Returns the 0-based insertion index of the key; throws if absent.
        size_t GetInsertionOrder(const std::string& key) const
        {
            auto it = m_order.find(key);
            if (it == m_order.end())
                throw std::out_of_range("TrackedMap: key not found: " + key);
            return it->second;
        }

        // Removes the key from all three maps. Returns true if the key existed.
        bool Remove(const std::string& key)
        {
            auto orderIt = m_order.find(key);
            if (orderIt == m_order.end())
                return false;
            m_indexToKey.erase(orderIt->second);
            m_order.erase(orderIt);
            m_data.erase(key);
            return true;
        }

        // Removes all entries and resets the insertion counter.
        void Clear()
        {
            m_data.clear();
            m_order.clear();
            m_indexToKey.clear();
            m_nextOrder = 0;
        }

        size_t Size()  const { return m_data.size(); }
        bool   Empty() const { return m_data.empty(); }

        // Returns all keys sorted by insertion index (ascending).
        // Gaps left by removed keys do not affect the sort.
        std::vector<std::string> GetOrderedKeys() const
        {
            std::vector<std::pair<size_t, std::string>> ordered(
                m_indexToKey.begin(), m_indexToKey.end());
            std::sort(ordered.begin(), ordered.end());
            std::vector<std::string> keys;
            keys.reserve(ordered.size());
            for (auto& [idx, key] : ordered)
                keys.push_back(std::move(key));
            return keys;
        }

    private:
        std::unordered_map<std::string, T>      m_data;       // key   → value
        std::unordered_map<std::string, size_t> m_order;      // key   → insertion index
        std::unordered_map<size_t, std::string> m_indexToKey; // index → key
        size_t                                  m_nextOrder = 0;
    };
}
