#include "unittest.h"

unittest_init();
unittest(
  unittest_test("Some failing test", {
	  unittest_assert(0 == 1,"WTF 0 should be equal to 1... I thought I was good at maths...");
	}, UNITTEST_NONE)
  unittest_test("Some passing test", {
    unittest_assert(0 == 0, "0 should be equal to 0, NO SHIT ???")
  }, UNITTEST_NONE)
  unittest_test("Not yet written test", {
    
  }, UNITTEST_TODO)
)
