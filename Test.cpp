#define LOCK_FREE_STACK_DEBUG

#include "LockFreeIndexStack.h"

using namespace LockFree;

#include <thread>
#include <iostream>
#include <cassert>
#include <algorithm>

const size_t stack_size = 8;
const size_t n_threads = 10;

// syncronize start of all threads
std::atomic<bool> go(false);

void foo(LockFreeIndexStack *stack)
{
    while (!go)
        std::this_thread::yield();
    for (size_t i = 0; i < 100; ++i) {
        LockFreeIndexStack::index_t index = stack->pop();
        std::this_thread::yield();
        stack->push(index);
        std::this_thread::yield();
    }
}

int main()
{
    LockFreeIndexStack stack((LockFreeIndexStack::index_t) stack_size);

    std::vector<std::unique_ptr<std::thread> > threads(n_threads);
    for (auto& t : threads)
        t.reset(new std::thread(foo, &stack));
    go = true;
    for (const auto& t : threads)
        t->join();

    std::cout << "n ops " << stack.counter() << "\n";
    std::cout << "n pop fail " << stack.pop_fail() << "\n";
    std::cout << "n pop spin " << stack.pop_spin() << "\n";
    std::cout << "n push spin " << stack.push_spin() << "\n";

    std::cout << "final stack state: ";
    LockFreeIndexStack::index_t next;
    std::vector<LockFreeIndexStack::index_t> s;
    while(true) {
        next = stack.pop_try();
        if (LockFreeIndexStack::is_valid(next)) {
            std::cout << next << ", ";
            s.push_back(next);
        }
        else
            break;
    }
    std::cout << "\n";
    assert(s.size() == stack_size);
    std::sort(s.begin(), s.end());
    for (size_t i = 0; i < stack_size; ++i)
        assert(s[i] == i);

    std::cout << "test success\n";

    return 0;
}

