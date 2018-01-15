# Memory-Allocator
A memory Allocator written in C that can assign, free and show data space
It takes commands from stdin and initializes the arena first. The commands can be:
INITIALIZE
FINALIZE
DUMP = shows the arena
FILL = fills x bytes with a designated character
ALLOC = allocates the number of bytes desired
FREE = frees an allocated area
