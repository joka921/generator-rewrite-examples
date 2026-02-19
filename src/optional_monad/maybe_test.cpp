// Unit tests for optional monad coroutine implementation
#include <gtest/gtest.h>

#include "maybe_example.h"

// ============================================================================
// ChainedCalculationTest - Tests for the chained_calculation coroutine
// ============================================================================

TEST(ChainedCalculationTest, SuccessfulChain) {
  auto result = chained_calculation(10, 2, 16);
  ASSERT_TRUE(result.has_value());
  EXPECT_EQ(*result, 9);  // (10 / 2) + sqrt(16) = 5 + 4 = 9
}

TEST(ChainedCalculationTest, ShortCircuitOnDivideByZero) {
  auto result = chained_calculation(10, 0, 16);
  EXPECT_FALSE(result.has_value());
}

TEST(ChainedCalculationTest, ShortCircuitOnNegativeSqrt) {
  auto result = chained_calculation(10, 2, -4);
  EXPECT_FALSE(result.has_value());
}

// ============================================================================
// WithExceptionsTest - Tests for exception handling in coroutines
// ============================================================================

TEST(WithExceptionsTest, ThrowsOnNegativeInput) {
  auto result = with_exceptions(-1);
  EXPECT_FALSE(result.has_value());
}

TEST(WithExceptionsTest, SuccessfulCalculation) {
  auto result = with_exceptions(5);
  ASSERT_TRUE(result.has_value());
  EXPECT_EQ(*result, 10);  // 5 * 2 = 10
}

TEST(WithExceptionsTest, ZeroInput) {
  auto result = with_exceptions(0);
  ASSERT_TRUE(result.has_value());
  EXPECT_EQ(*result, 0);  // 0 * 2 = 0
}

// ============================================================================
// HelperFunctionsTest - Tests for safe_divide and safe_sqrt
// ============================================================================

TEST(HelperFunctionsTest, SafeDivideSuccess) {
  auto result = safe_divide(10, 2);
  ASSERT_TRUE(result.has_value());
  EXPECT_EQ(*result, 5);
}

TEST(HelperFunctionsTest, SafeDivideDivideByZero) {
  auto result = safe_divide(10, 0);
  EXPECT_FALSE(result.has_value());
}

TEST(HelperFunctionsTest, SafeDivideNegativeNumbers) {
  auto result = safe_divide(-10, 2);
  ASSERT_TRUE(result.has_value());
  EXPECT_EQ(*result, -5);
}

TEST(HelperFunctionsTest, SafeSqrtSuccess) {
  auto result = safe_sqrt(16);
  ASSERT_TRUE(result.has_value());
  EXPECT_EQ(*result, 4);
}

TEST(HelperFunctionsTest, SafeSqrtZero) {
  auto result = safe_sqrt(0);
  ASSERT_TRUE(result.has_value());
  EXPECT_EQ(*result, 0);
}

TEST(HelperFunctionsTest, SafeSqrtNegativeInput) {
  auto result = safe_sqrt(-4);
  EXPECT_FALSE(result.has_value());
}

TEST(HelperFunctionsTest, SafeSqrtNonPerfectSquare) {
  auto result = safe_sqrt(5);
  ASSERT_TRUE(result.has_value());
  EXPECT_EQ(*result, 2);  // Integer result of sqrt(5) truncated
}

TEST(HelperFunctionsTest, SafeSqrtLargeNumber) {
  auto result = safe_sqrt(100);
  ASSERT_TRUE(result.has_value());
  EXPECT_EQ(*result, 10);
}

// ============================================================================
// ManualImplementationTest - Verify equivalence between coroutine and manual
// ============================================================================

TEST(ManualImplementationTest, ChainedCalculationSuccessEquivalence) {
  auto coro_result = chained_calculation(10, 2, 16);
  auto manual_result = chained_calculation_no_coro(10, 2, 16);

  ASSERT_EQ(coro_result.has_value(), manual_result.has_value());
  if (coro_result) {
    EXPECT_EQ(*coro_result, *manual_result);
  }
}

TEST(ManualImplementationTest, ChainedCalculationDivideByZeroEquivalence) {
  auto coro_result = chained_calculation(10, 0, 16);
  auto manual_result = chained_calculation_no_coro(10, 0, 16);

  EXPECT_EQ(coro_result.has_value(), manual_result.has_value());
  EXPECT_FALSE(coro_result.has_value());
}

TEST(ManualImplementationTest, ChainedCalculationNegativeSqrtEquivalence) {
  auto coro_result = chained_calculation(10, 2, -4);
  auto manual_result = chained_calculation_no_coro(10, 2, -4);

  EXPECT_EQ(coro_result.has_value(), manual_result.has_value());
  EXPECT_FALSE(coro_result.has_value());
}

TEST(ManualImplementationTest, WithExceptionsSuccessEquivalence) {
  auto coro_result = with_exceptions(5);
  auto manual_result = with_exceptions_no_coro(5);

  ASSERT_EQ(coro_result.has_value(), manual_result.has_value());
  if (coro_result) {
    EXPECT_EQ(*coro_result, *manual_result);
  }
}

TEST(ManualImplementationTest, WithExceptionsFailureEquivalence) {
  auto coro_result = with_exceptions(-1);
  auto manual_result = with_exceptions_no_coro(-1);

  EXPECT_EQ(coro_result.has_value(), manual_result.has_value());
  EXPECT_FALSE(coro_result.has_value());
}