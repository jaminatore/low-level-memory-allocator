# Custom Memory Allocator 

This project implements a simplified custom memory allocator in C/C++, similar to `malloc()` and `free()` in the standard library. It manages a manually allocated heap using `mmap()` and a linked list free list structure.

The goal is to demonstrate how dynamic memory management can be built from scratch, including allocation, deallocation, and coalescing of free blocks.

---

## Files

| File | Description |
|------|--------------|
| `my_malloc.h` | Header file defining structs (`header_t`, `node_t`), constants, and function prototypes. |
| `my_malloc.cpp` | Core implementation of the memory allocator (`my_malloc`, `my_free`, `find_free`, `split`, `coalesce`, etc.). |
| `main.cpp` | Test driver that allocates, frees, and prints heap status for verification. |

---

## Overview

### Key Concepts

1. **Heap Simulation**
   - A single page of memory (4 KB) is allocated using `mmap()`.
   - The heap is tracked by a global pointer `head`, which points to the start of the free list.

2. **Free List**
   - A singly linked list (`node_t`) representing all available memory blocks.
   - Each node stores its size and a pointer to the next free node.

3. **Allocation (`my_malloc`)**
   - Uses a **first-fit** algorithm to find a block large enough for the request.
   - Splits that block into an allocated region (with a header) and a remaining free region.
   - Returns a pointer just past the `header_t`.

4. **Deallocation (`my_free`)**
   - Revalidates the pointer by checking a “magic number” (`0xDEADBEEF`).
   - Inserts the freed block back into the free list.
   - Calls `coalesce()` to merge adjacent free regions.

5. **Coalescing**
   - When two adjacent free blocks are contiguous in memory, they are merged to reduce fragmentation.

6. **Utilities**
   - `available_memory()` sums the total free memory in the heap.
   - `print_free_list()` prints the current free list layout.
   - `reset_heap()` resets the allocator by unmapping and reinitializing the heap.

---

## Directory Structure

```
low-level-memory-allocator/
├── include/
│ └── my_malloc.h
├── src/
│ ├── main.cpp
│ └── my_malloc.cpp
├── docs/
│ └── protocol-notes.md
├── README.md
└── .gitignore
```
