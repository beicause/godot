#include "core/os/memory.h"

static void free_safe(void *ptr) {
	if (ptr != nullptr) {
		memfree(ptr);
	}
}
#define TSF_IMPLEMENTATION
#define TSF_NO_STDIO
#define TSF_MALLOC memalloc
#define TSF_REALLOC memrealloc
#define TSF_FREE free_safe
#include "tiny/tsf.h"
// for .sf3
#include "stb/stb_vorbis.h"
