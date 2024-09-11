#include "core/os/memory.h"

static inline void free_safe(void *ptr) {
	if (unlikely(ptr != nullptr)) {
		memfree(ptr);
	}
}
#define TSF_IMPLEMENTATION
#define TSF_NO_STDIO
#define TSF_MALLOC memalloc
#define TSF_REALLOC memrealloc
#define TSF_FREE free_safe
#include "stb/stb_vorbis.c"
#include "tiny/tsf.h"
