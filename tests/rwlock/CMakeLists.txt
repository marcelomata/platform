ADD_EXECUTABLE(rwlock_test rwlock_test.cc)
TARGET_LINK_LIBRARIES(rwlock_test gtest gtest_main platform ${FOLLY_LIBRARIES})
TARGET_INCLUDE_DIRECTORIES(rwlock_test PRIVATE
        ${Platform_SOURCE_DIR}/include
        ${FOLLY_INCLUDE_DIR})
ADD_TEST(rwlock_test rwlock_test)

# The death tests in this suite rely on halting as soon as an error is detected.
# For macOS must explicitly enable deadlock detection.
SET_TESTS_PROPERTIES(rwlock_test PROPERTIES
  ENVIRONMENT "TSAN_OPTIONS=halt_on_error=1 detect_deadlocks=1")
