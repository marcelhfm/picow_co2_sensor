#include "unity.h"
#include "unity_internals.h"

void setUp(void) {}

void tearDown(void) {}

void test_update_display_task_init(void) {
  int a = 0;

  TEST_ASSERT_EQUAL(a, 0);
}

int main(void) {
  UNITY_BEGIN();

  RUN_TEST(test_update_display_task_init);

  return UNITY_END();
}
