#include "test/jemalloc_test.h"

static rtree_node_elm_t *
node_alloc(size_t nelms)
{

	return (calloc(nelms, sizeof(rtree_node_elm_t)));
}

static void
node_dalloc(rtree_node_elm_t *node)
{

	free(node);
}

TEST_BEGIN(test_rtree_get_empty)
{
	unsigned i;

	for (i = 1; i <= (sizeof(uintptr_t) << 3); i++) {
		rtree_t rtree;
		assert_false(rtree_new(&rtree, i, node_alloc, node_dalloc),
		    "Unexpected rtree_new() failure");
		assert_ptr_eq(rtree_get(&rtree, 0), NULL,
		    "rtree_get() should return NULL for empty tree");
		rtree_delete(&rtree);
	}
}
TEST_END

TEST_BEGIN(test_rtree_extrema)
{
	unsigned i;
	extent_node_t node_a, node_b;

	for (i = 1; i <= (sizeof(uintptr_t) << 3); i++) {
		rtree_t rtree;
		assert_false(rtree_new(&rtree, i, node_alloc, node_dalloc),
		    "Unexpected rtree_new() failure");

		rtree_set(&rtree, 0, &node_a);
		assert_ptr_eq(rtree_get(&rtree, 0), &node_a,
		    "rtree_get() should return previously set value");

		rtree_set(&rtree, ~((uintptr_t)0), &node_b);
		assert_ptr_eq(rtree_get(&rtree, ~((uintptr_t)0)), &node_b,
		    "rtree_get() should return previously set value");

		rtree_delete(&rtree);
	}
}
TEST_END

TEST_BEGIN(test_rtree_bits)
{
	unsigned i, j, k;

	for (i = 1; i < (sizeof(uintptr_t) << 3); i++) {
		uintptr_t keys[] = {0, 1,
		    (((uintptr_t)1) << (sizeof(uintptr_t)*8-i)) - 1};
		extent_node_t node;
		rtree_t rtree;

		assert_false(rtree_new(&rtree, i, node_alloc, node_dalloc),
		    "Unexpected rtree_new() failure");

		for (j = 0; j < sizeof(keys)/sizeof(uintptr_t); j++) {
			rtree_set(&rtree, keys[j], &node);
			for (k = 0; k < sizeof(keys)/sizeof(uintptr_t); k++) {
				assert_ptr_eq(rtree_get(&rtree, keys[k]), &node,
				    "rtree_get() should return previously set "
				    "value and ignore insignificant key bits; "
				    "i=%u, j=%u, k=%u, set key=%#"PRIxPTR", "
				    "get key=%#"PRIxPTR, i, j, k, keys[j],
				    keys[k]);
			}
			assert_ptr_eq(rtree_get(&rtree,
			    (((uintptr_t)1) << (sizeof(uintptr_t)*8-i))), NULL,
			    "Only leftmost rtree leaf should be set; "
			    "i=%u, j=%u", i, j);
			rtree_set(&rtree, keys[j], NULL);
		}

		rtree_delete(&rtree);
	}
}
TEST_END

TEST_BEGIN(test_rtree_random)
{
	unsigned i;
	sfmt_t *sfmt;
#define	NSET 16
#define	SEED 42

	sfmt = init_gen_rand(SEED);
	for (i = 1; i <= (sizeof(uintptr_t) << 3); i++) {
		uintptr_t keys[NSET];
		extent_node_t node;
		unsigned j;
		rtree_t rtree;

		assert_false(rtree_new(&rtree, i, node_alloc, node_dalloc),
		    "Unexpected rtree_new() failure");

		for (j = 0; j < NSET; j++) {
			keys[j] = (uintptr_t)gen_rand64(sfmt);
			rtree_set(&rtree, keys[j], &node);
			assert_ptr_eq(rtree_get(&rtree, keys[j]), &node,
			    "rtree_get() should return previously set value");
		}
		for (j = 0; j < NSET; j++) {
			assert_ptr_eq(rtree_get(&rtree, keys[j]), &node,
			    "rtree_get() should return previously set value");
		}

		for (j = 0; j < NSET; j++) {
			rtree_set(&rtree, keys[j], NULL);
			assert_ptr_eq(rtree_get(&rtree, keys[j]), NULL,
			    "rtree_get() should return previously set value");
		}
		for (j = 0; j < NSET; j++) {
			assert_ptr_eq(rtree_get(&rtree, keys[j]), NULL,
			    "rtree_get() should return previously set value");
		}

		rtree_delete(&rtree);
	}
	fini_gen_rand(sfmt);
#undef NSET
#undef SEED
}
TEST_END

int
main(void)
{

	return (test(
	    test_rtree_get_empty,
	    test_rtree_extrema,
	    test_rtree_bits,
	    test_rtree_random));
}
