Improvised Malloc Design

# Implement a memory allocator for the heap of a user-level process
# Malloc - functions similar to traditional malloc, but free can free memory from any point in the allocated memory

Implementations of Mem_Alloc (int size) and Mem_Free (void *ptr) are identical, except the pointer passed to Mem_Free does not have to have been previously returned by Mem_Alloc;
instead, ptr can point to any valid range of memory returned by Mem_Alloc.

Needed to have a more sophisticated data structure than the traditional malloc to track the
regions of memory allocated by Mem_Alloc.
Specifically, this data structure will allow you to efficiently map any address to the
corresponding memory object or to determine that there is no corresponding object.

http://pages.cs.wisc.edu/~dusseau/Classes/CS537-F07/Projects/P3/index.html
