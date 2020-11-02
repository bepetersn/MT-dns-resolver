# MT-dns-resolver

### mt-cirque.c, mt-cirque.h

Queue implementation that is multi-threaded (not circular, that was old version).
Use for buffering logs in a thread, as shared buffer, and as a file array.

### multi-lookup.c, multi-lookup.h

main thread, create shared resources, etc.

### requester.c, requester.h

requester thread function

### resolver.c, resolver.h

resolver thread function

### util.c, util.h

Added one file here.
