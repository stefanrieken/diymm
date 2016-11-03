DIY Memory Management
=====================

Show how a simple singly linked list can be used to allocate data on the heap.

Unfortunately, since this version is set to run on an operating system, the
heap itself has to be allocated at once using a single 'malloc'.

Status: works but invites refactoring. Allocations done in 'main' are visualized
with ASCII art: #= a list entry; \*= in use; \-= free.

There is always a root node.
