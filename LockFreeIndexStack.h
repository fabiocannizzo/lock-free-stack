#pragma once

#include <cstdint>
#include <vector>
#include <atomic>

namespace LockFree
{
    class LockFreeIndexStack
    {
    public:
        typedef uint32_t index_t;
        typedef uint64_t bundle_t;

    private:
        static const index_t s_null = ~index_t(0);

        union Bundle
        {
            Bundle(index_t index, index_t count)
            {
                m_value.m_index = index;
                m_value.m_count = count;
            }

            Bundle(bundle_t bundle)
            {
                m_bundle = bundle;
            }

            struct {
                index_t m_index;
                index_t m_count;  // this is to probabilistically avoid the ABA problem (Treiber)
            } m_value;

            bundle_t m_bundle;
        };

    public:
        LockFreeIndexStack(index_t n)
            : m_top(Bundle(0, 0).m_bundle)
            , m_next(n, s_null)
#ifdef LOCK_FREE_STACK_DEBUG
            , m_pop_spin(0)
            , m_pop_fail(0)
            , m_push_spin(0)
#endif
        {
            // CHKVALID(n > 0 && n < s_null, "Stack size outside of allowed range");

            // init stack entries
            for (index_t i = 1; i < n; ++i)
                m_next[i - 1] = i;
        }

        index_t top() const
        {
            return Bundle(m_top).m_value.m_index;
        }

        index_t counter() const
        {
            return Bundle(m_top).m_value.m_count;
        }

#ifdef LOCK_FREE_STACK_DEBUG
        size_t pop_spin() const
        {
            return m_pop_spin;
        }
        size_t push_spin() const
        {
            return m_push_spin;
        }
        size_t pop_fail() const
        {
            return m_pop_fail;
        }
#endif
        index_t pop_try()
        {
            Bundle curtop(m_top);
            index_t candidate = curtop.m_value.m_index;
            if (candidate != s_null) {
                index_t next = m_next[candidate];
                Bundle newtop(next, curtop.m_value.m_count + 1);
                if (m_top.compare_exchange_weak(curtop.m_bundle, newtop.m_bundle)) {
                    return candidate;
                }
                else {
#ifdef LOCK_FREE_STACK_DEBUG
                    ++m_pop_spin;
#endif
                }
            }
            else {
#ifdef LOCK_FREE_STACK_DEBUG
                ++m_pop_fail;
#endif
            }
            return s_null;
        }

        index_t pop()
        {
            index_t index;
            do {
                index = pop_try();
            } while (index == s_null);
            return index;
        }

        index_t capacity() const { return (index_t)m_next.size(); }

        static bool is_valid(index_t index) { return index != s_null; }

        void push(index_t index)
        {
            while (true) {
                Bundle curtop(m_top);
                index_t current = curtop.m_value.m_index;
                m_next[index] = current;
                Bundle newtop(index, curtop.m_value.m_count + 1);
                if (m_top.compare_exchange_weak(curtop.m_bundle, newtop.m_bundle)) {
                    break;
                }
                else {
#ifdef LOCK_FREE_STACK_DEBUG
                    ++m_push_spin;
#endif
                }
            };
        }

    private:
        std::atomic<bundle_t> m_top;
        std::vector<index_t> m_next;

#ifdef LOCK_FREE_STACK_DEBUG
        std::atomic<size_t> m_pop_spin;
        std::atomic<size_t> m_pop_fail;
        std::atomic<size_t> m_push_spin;
#endif
    };

} // namespace LockFree
