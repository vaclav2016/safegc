#include "safegc.h"

void test_ok() {
	char *str = GC_MALLOC(126);
	GC_FREE(str);
	GC_DUMP();
}

void test_bad() {
	char *str = GC_MALLOC(126);
	GC_DUMP();
}

int main() {
	GC_INIT(1024 * 64);
	test_ok();
	test_bad();
	GC_DONE();
	return 0;

}
