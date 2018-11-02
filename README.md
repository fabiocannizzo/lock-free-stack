`LockFreeIndexStack` is a simple lock free stack class which manages a sequence of unsigned 16 bit integers.

This can be used as the building block of a stack of genric objects.

The stack class has a fixed size, which must be stated when the class in initially constructed. Once created, the stack is full of 16 bit unsigned integers in the range 0..size-1.

```c++
LockFreeIndexStack stack(4);  // creates a stack containinig: 0, 1, 2, 3
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
LockFreeIndexStack::index_t index = stack.pop_try(); // Get an index from the stack if available.
if (LockFreeIndexStack::is_valid(index) {
    // ... do something with the index
    s.push(index); // return index to the stack.
}
else {
    std::cout << "No index available at the moment\n";
}
```

There are no error checks on `push`: if you push to the stack a value that was not previosuly extracted from the stack, or if you push twice the same value to the stack, you will corrupt the stack.

```c++
LockFreeIndexStack stack(4);  // creates a stack containinig: 0, 1, 2, 3
s.push(5); // This will corrupt the stack, because 5 is not in the valid range
```

```c++
LockFreeIndexStack stack(4);  // creates a stack containinig: 0, 1, 2, 3
s.push(2); // This will corrupt the stack, because 2 was not previously extracted from the stack
```

```c++
LockFreeIndexStack stack(4);  // creates a stack containinig: 0, 1, 2, 3
LockFreeIndexStack::index_t index = stack.pop(); // Get an index from the stack. Spin locks until one is available.
stack.push(index); // return index to the stack.
stack.push(index); // This will corrupt the stack, becuase the index has been returned twice
```


