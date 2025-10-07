
#include <assert.h>
#include "../include/my_malloc.h"
#include <stdio.h>
#include <sys/mman.h>

// A pointer to the head of the free list.
node_t *head = NULL;

// The heap function returns the head pointer to the free list. If the heap
// has not been allocated yet (head is NULL) it will use mmap to allocate
// a page of memory from the OS and initialize the first free node.
node_t *heap() {
  if (head == NULL) {
    // This allocates the heap and initializes the head node.
    head = (node_t *)mmap(NULL, HEAP_SIZE, PROT_READ | PROT_WRITE,
                          MAP_ANON | MAP_PRIVATE, -1, 0);
    head->size = HEAP_SIZE - sizeof(node_t);
    head->next = NULL;
  }

  return head;
}

// Reallocates the heap.
void reset_heap() {
  if (head != NULL) {
    munmap(head, HEAP_SIZE);
    head = NULL;
    heap();
  }
}

// Returns a pointer to the head of the free list.
node_t *free_list() { return head; }

// Calculates the amount of free memory available in the heap.
size_t available_memory() {
  size_t n = 0;
  node_t *p = heap();
  while (p != NULL) {
    n += p->size;
    p = p->next;
  }
  return n;
}

// Returns the number of nodes on the free list.
int number_of_free_nodes() {
  int count = 0;
  node_t *p = heap();
  while (p != NULL) {
    count++;
    p = p->next;
  }
  return count;
}

// Prints the free list. Useful for debugging purposes.
void print_free_list() {
  node_t *p = heap();
  while (p != NULL) {
    printf("Free(%zd)", p->size);
    p = p->next;
    if (p != NULL) {
      printf("->");
    }
  }
  printf("\n");
}

// Finds a node on the free list that has enough available memory to
// allocate to a calling program. This function uses the "first-fit"
// algorithm to locate a free node.
//
// PARAMETERS:
// size - the number of bytes requested to allocate
//
// RETURNS:
// found - the node found on the free list with enough memory to allocate
// previous - the previous node to the found node
//
void find_free(size_t size, node_t **found, node_t **previous) {
  // piazza post??
  heap(); 
  // initialize nodes
  node_t *current_node = head; 
  node_t *previous_node = NULL;
  // initialize vars
  size_t space = sizeof(header_t) + size; 
  // iterate through list 
  while (current_node != NULL) {
    // check for available size; 
    if (current_node->size >= space) {
      // update pointers
      *found = current_node;
      *previous = previous_node; 
      return; 
    } else {
      // move onto next node
      previous_node = current_node;
      current_node = current_node->next; 
    }
  }
}

// Splits a found free node to accommodate an allocation request.
//
// The job of this function is to take a given free_node found from
// `find_free` and split it according to the number of bytes to allocate.
// In doing so, it will adjust the size and next pointer of the `free_block`
// as well as the `previous` node to properly adjust the free list.
//
// PARAMETERS:
// size - the number of bytes requested to allocate
// previous - the previous node to the free block
// free_block - the node on the free list to allocate from
//
// RETURNS:
// allocated - an allocated block to be returned to the calling program
//
void split(size_t size, node_t **previous, node_t **free_block,
           header_t **allocated) {
  assert(*free_block != NULL);
  // pointer to original free block
  node_t *pointer = *free_block; 
  // adjust size
  size_t actual_size = size + sizeof(header_t);
  char *allocated_space = ((char *)pointer + actual_size);
  // check if theres enough size
  if ((*free_block)->size >= *allocated_space) {
      // initialize new nodes & update size
      *free_block = (node_t *)(allocated_space);
      (*free_block)->size = pointer->size - actual_size;
    } else {
    // not enough space
    *free_block = NULL;
  }
  // allocating first node or not
  *previous == NULL ? head = *free_block : (*previous)->next = *free_block; 
  // cast 
  *allocated = (header_t *)pointer; 
  // update fields
  (*allocated)->size = size; 
  (*allocated)->magic = MAGIC; 
}

// Returns a pointer to a region of memory having at least the request `size`
// bytes.
//
// PARAMETERS:
// size - the number of bytes requested to allocate
//
// RETURNS:
// A void pointer to the region of allocated memory
//
void *my_malloc(size_t size) {
  // initialize nodes & vars 
  node_t *found = NULL;
  node_t *previous = NULL;
  header_t *allocated = NULL;
  // find first free space
  find_free(size, &found, &previous); 
  // if no free space then return
  if (found == NULL) { return NULL; }
  // split memory is free space
  split(size, &previous, &found, &allocated);
  // return pointer to space after header
  return (void *)(allocated + 1); 
}

// Merges adjacent nodes on the free list to reduce external fragmentation.
//
// This function will only coalesce nodes starting with `free_block`. It will
// not handle coalescing of previous nodes (we don't have previous pointers!).
//
// PARAMETERS:
// free_block - the starting node on the free list to coalesce
//
void coalesce(node_t *free_block) {
  // initialize nodes & vars
  node_t *current_node = free_block; 
  size_t block_size = current_node->size + sizeof(node_t);
  // iterate through list
  while (current_node->next != NULL) {
    // check if current val is the same as the next value's next
    if (((char *)(current_node) + block_size == (char*)current_node->next)) {
      current_node->size += current_node->next->size + sizeof(node_t); 
      current_node->next = current_node->next->next; 
      // exit after first 
      break; 
    } else {
      // move onto next node
      current_node = current_node->next; 
    }
  }
}

// Frees a given region of memory back to the free list.
//
// PARAMETERS:
// allocated - a pointer to a region of memory previously allocated by my_malloc
//
void my_free(void *allocated) {
  // cast to header type and adjust to be actual start of header
  header_t *node_to_free = (header_t *)((char *)allocated - sizeof(header_t)); 
  // assert magic value
  assert(node_to_free->magic == MAGIC); 
  // cast to node type
  node_t *free_memory = (node_t *)node_to_free;
  // set size 
  // free_memory->size = sizeof(header_t) + node_to_free->size; 
  free_memory->size = node_to_free->size; 
  // make start of heap & update pointer
  free_memory->next = head;
  head = free_memory; 
  // coalesce at the end
  coalesce(head); 
}