#include "thread_dlmalloc.h"

int main() {
	mspace space = create_mspace(1000);
	void* ptr = mspace_malloc(space, 100);
	return 0;
}

