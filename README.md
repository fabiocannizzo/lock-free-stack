`LockFreeIndexStack` is a simple lock free stack class which manages a sequence of 32 bits unsigned integers.

This can be used as the building block of a stack of generic objects.

It is based on C++11, but could be easily adapted to work with C++98 using tbb, boost, or even directly the API for compare and exchange available in Windows and Linux.

The stack class has a fixed size, which must be stated when the class is initially constructed. Once created, the stack is full of 32 bits unsigned integers in the range 0..size-1.

```c++
LockFreeIndexStack stack(4);  // creates a stack containing: 0, 1, 2, 3
```

Main methods, `push`, `pop` and `pop_try`, are lock free, but not wait free. They can be used to extract and push back values to the stack.
```c++
LockFreeIndexStack stack(4);  // creates a stack containinig: 0, 1, 2, 3
LockFreeIndexStack::index_t index = stack.pop(); // Get an index from the stack. Spin locks until one is available.
// ... do something with the index
stack.push(index); // return index to the stack.
```

The function `is_valid` must be used to check that the return value of `pop_try` is valid.
```c++
LockFreeIndexStack stack(4);  // creates a stack containinig: 0, 1, 2, 3
LockFreeIndexStack::index_t index = stack.pop_try(); // get an index from the stack if available, otherwise returns an ivalid index.
if (LockFreeIndexStack::is_valid(index) {
    // ... do something with 'index'
    s.push(index); // return 'index' to the stack.
}
else {
    std::cout << "No index available at the moment\n";
}
```

There are no error checks on `push`: if you push to the stack a value that was not previosuly extracted from the stack, or if you push twice the same value to the stack, you will corrupt the stack.

```c++
LockFreeIndexStack stack(4);  // creates a stack containinig: 0, 1, 2, 3
s.push(5); // this will corrupts the stack, because 5 is not in the valid range
```

```c++
LockFreeIndexStack stack(4);  // creates a stack containinig: 0, 1, 2, 3
s.push(2); // corrupts the stack, because 2 was not previously extracted from the stack
```

```c++
LockFreeIndexStack stack(4);  // creates a stack containinig: 0, 1, 2, 3
LockFreeIndexStack::index_t index = stack.pop(); // Get an index from the stack. Spin locks until one is available.
stack.push(index); // return 'index' to the stack.
stack.push(index); // corrupts the stack, becuase 'index' has been already returned
```

There is a very remote possibility that the class fails and the stack gest corrupted.
The `pop` and `pop_try` methods attempt an atomic compare and exchage of the top of the stack. They also use a 32 bits counter which is incremented at every operation to avoid the ABA problem, but there is one particular case where the ABA problem can still occurr. This is explained in the code below.
```c++
index_t pop_try()
{
    Bundle curtop(m_top);
    index_t candidate = curtop.m_value.m_index;
    if (candidate != s_null) {
        index_t next = m_next[candidate];
        Bundle newtop(next, curtop.m_value.m_count + 1);
        // In the very remote eventuality that, while this thread is here,
        // all the below circumstances occur simultaneously:
        // - other threads execute exactly a multiple of 2^32 pop or push operations,
        //   so that 'm_count' assumes again the original value;
        // - the value read as 'candidate' 2^32 transactions ago is again top of the stack;
        // - the value 'm_next[candidate]' is no longer what it was 2^32 transactions ago
        // then the stack will get corrupted
        if (m_top.compare_exchange_weak(curtop.m_bundle, newtop.m_bundle)) {
            return candidate;
        }
    }
    return s_null;
}
```