#include "core/os/memory.h"

static inline void free_safe(void *ptr) {
	if (unlikely(ptr != nullptr)) {
		memfree(ptr);
	}
}
#define TML_IMPLEMENTATION
#define TML_NO_STDIO
#define TML_MALLOC memalloc
#define TML_REALLOC memrealloc
#define TML_FREE free_safe
#include "tiny/tml.h"
