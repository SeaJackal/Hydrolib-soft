#include "test_hydrolib_rq_env.hpp"

INSTANTIATE_TEST_CASE_P(
    Test,
    TestHydrolibRingQueue,
    ::testing::Range<uint16_t>(0, 16));
