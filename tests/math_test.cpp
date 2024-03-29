/*
 * Copyright (C) 2013 The Android Open Source Project
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#define _GNU_SOURCE 1
#include <math.h>

// This include (and the associated definition of __test_capture_signbit)
// must be placed before any files that include <cmath> (gtest.h in this case).
//
// <math.h> is required to define generic macros signbit, isfinite and
// several other such functions.
//
// <cmath> is required to undef declarations of these macros in the global
// namespace and make equivalent functions available in namespace std. Our
// stlport implementation does this only for signbit, isfinite, isinf and
// isnan.
//
// NOTE: We don't write our test using std::signbit because we want to be
// sure that we're testing the bionic version of signbit. The C++ libraries
// are free to reimplement signbit or delegate to compiler builtins if they
// please.

namespace {
template<typename T> inline int test_capture_signbit(const T in) {
  return signbit(in);
}
template<typename T> inline int test_capture_isfinite(const T in) {
  return isfinite(in);
}
template<typename T> inline int test_capture_isnan(const T in) {
  return isnan(in);
}
template<typename T> inline int test_capture_isinf(const T in) {
  return isinf(in);
}
}

#include "math_data_test.h"

#include <gtest/gtest.h>

#include <fenv.h>
#include <float.h>
#include <limits.h>
#include <stdint.h>

#include <private/ScopeGuard.h>

float float_subnormal() {
  union {
    float f;
    uint32_t i;
  } u;
  u.i = 0x007fffff;
  return u.f;
}

double double_subnormal() {
  union {
    double d;
    uint64_t i;
  } u;
  u.i = 0x000fffffffffffffLL;
  return u.d;
}

long double ldouble_subnormal() {
  union {
    long double e;
    unsigned char c[sizeof(long double)];
  } u;

  // Subnormals must have a zero exponent and non zero significand.
  // On all supported representation the 17 bit (counting from either sides)
  // is part of the significand so it should be enough to set that.
  // It also applies for the case sizeof(double) = sizeof(long double)
  for (unsigned int i = 0; i < sizeof(long double); i++) {
    u.c[i] = 0x00;
  }
  u.c[sizeof(long double) - 3] = 0x80;
  u.c[2] = 0x80;

  return u.e;
}

TEST(math, fpclassify) {
  ASSERT_EQ(FP_INFINITE, fpclassify(INFINITY));
  ASSERT_EQ(FP_INFINITE, fpclassify(HUGE_VALF));
  ASSERT_EQ(FP_INFINITE, fpclassify(HUGE_VAL));
#ifndef SKIP_LONG_DOUBLE_TESTS
  ASSERT_EQ(FP_INFINITE, fpclassify(HUGE_VALL));
#endif

  ASSERT_EQ(FP_NAN, fpclassify(nanf("")));
  ASSERT_EQ(FP_NAN, fpclassify(nan("")));
#ifndef SKIP_LONG_DOUBLE_TESTS
  ASSERT_EQ(FP_NAN, fpclassify(nanl("")));
#endif

  ASSERT_EQ(FP_NORMAL, fpclassify(1.0f));
  ASSERT_EQ(FP_NORMAL, fpclassify(1.0));
#ifndef SKIP_LONG_DOUBLE_TESTS
  ASSERT_EQ(FP_NORMAL, fpclassify(1.0L));
#endif

  ASSERT_EQ(FP_SUBNORMAL, fpclassify(float_subnormal()));
  ASSERT_EQ(FP_SUBNORMAL, fpclassify(double_subnormal()));
  ASSERT_EQ(FP_SUBNORMAL, fpclassify(ldouble_subnormal()));

  ASSERT_EQ(FP_ZERO, fpclassify(0.0f));
  ASSERT_EQ(FP_ZERO, fpclassify(0.0));
#ifndef SKIP_LONG_DOUBLE_TESTS
  ASSERT_EQ(FP_ZERO, fpclassify(0.0L));
#endif
}

TEST(math, isfinite) {
  ASSERT_TRUE(test_capture_isfinite(123.0f));
  ASSERT_TRUE(test_capture_isfinite(123.0));
#ifndef SKIP_LONG_DOUBLE_TESTS
  ASSERT_TRUE(test_capture_isfinite(123.0L));
#endif
  ASSERT_FALSE(test_capture_isfinite(HUGE_VALF));
  ASSERT_FALSE(test_capture_isfinite(HUGE_VAL));
#ifndef SKIP_LONG_DOUBLE_TESTS
  ASSERT_FALSE(test_capture_isfinite(HUGE_VALL));
#endif
}

TEST(math, isinf) {
  ASSERT_FALSE(test_capture_isinf(123.0f));
  ASSERT_FALSE(test_capture_isinf(123.0));
#ifndef SKIP_LONG_DOUBLE_TESTS
  ASSERT_FALSE(test_capture_isinf(123.0L));
#endif
  ASSERT_TRUE(test_capture_isinf(HUGE_VALF));
  ASSERT_TRUE(test_capture_isinf(HUGE_VAL));
#ifndef SKIP_LONG_DOUBLE_TESTS
  ASSERT_TRUE(test_capture_isinf(HUGE_VALL));
#endif
}

TEST(math, isnan) {
  ASSERT_FALSE(test_capture_isnan(123.0f));
  ASSERT_FALSE(test_capture_isnan(123.0));
#ifndef SKIP_LONG_DOUBLE_TESTS
  ASSERT_FALSE(test_capture_isnan(123.0L));
#endif
  ASSERT_TRUE(test_capture_isnan(nanf("")));
  ASSERT_TRUE(test_capture_isnan(nan("")));
#ifndef SKIP_LONG_DOUBLE_TESTS
  ASSERT_TRUE(test_capture_isnan(nanl("")));
#endif
}

TEST(math, isnormal) {
  ASSERT_TRUE(isnormal(123.0f));
  ASSERT_TRUE(isnormal(123.0));
#ifndef SKIP_LONG_DOUBLE_TESTS
  ASSERT_TRUE(isnormal(123.0L));
#endif
  ASSERT_FALSE(isnormal(float_subnormal()));
  ASSERT_FALSE(isnormal(double_subnormal()));
  ASSERT_FALSE(isnormal(ldouble_subnormal()));
}

// TODO: isgreater, isgreaterequals, isless, islessequal, islessgreater, isunordered
TEST(math, signbit) {
  ASSERT_EQ(0, test_capture_signbit(0.0f));
  ASSERT_EQ(0, test_capture_signbit(0.0));
#ifndef SKIP_LONG_DOUBLE_TESTS
  ASSERT_EQ(0, test_capture_signbit(0.0L));
#endif

  ASSERT_EQ(0, test_capture_signbit(1.0f));
  ASSERT_EQ(0, test_capture_signbit(1.0));
#ifndef SKIP_LONG_DOUBLE_TESTS
  ASSERT_EQ(0, test_capture_signbit(1.0L));
#endif

  ASSERT_NE(0, test_capture_signbit(-1.0f));
  ASSERT_NE(0, test_capture_signbit(-1.0));
#ifndef SKIP_LONG_DOUBLE_TESTS
  ASSERT_NE(0, test_capture_signbit(-1.0L));
#endif
}

TEST(math, __fpclassifyd) {
#if defined(__GLIBC__)
#define __fpclassifyd __fpclassify
#endif
  ASSERT_EQ(FP_INFINITE, __fpclassifyd(HUGE_VAL));
  ASSERT_EQ(FP_NAN, __fpclassifyd(nan("")));
  ASSERT_EQ(FP_NORMAL, __fpclassifyd(1.0));
  ASSERT_EQ(FP_SUBNORMAL, __fpclassifyd(double_subnormal()));
  ASSERT_EQ(FP_ZERO, __fpclassifyd(0.0));
}

TEST(math, __fpclassifyf) {
  ASSERT_EQ(FP_INFINITE, __fpclassifyf(HUGE_VALF));
  ASSERT_EQ(FP_NAN, __fpclassifyf(nanf("")));
  ASSERT_EQ(FP_NORMAL, __fpclassifyf(1.0f));
  ASSERT_EQ(FP_SUBNORMAL, __fpclassifyf(float_subnormal()));
  ASSERT_EQ(FP_ZERO, __fpclassifyf(0.0f));
}

TEST(math, __fpclassifyl) {
#ifndef SKIP_LONG_DOUBLE_TESTS
  EXPECT_EQ(FP_INFINITE, __fpclassifyl(HUGE_VALL));
  EXPECT_EQ(FP_NAN, __fpclassifyl(nanl("")));
  EXPECT_EQ(FP_NORMAL, __fpclassifyl(1.0L));
#endif
  EXPECT_EQ(FP_SUBNORMAL, __fpclassifyl(ldouble_subnormal()));
#ifndef SKIP_LONG_DOUBLE_TESTS
  EXPECT_EQ(FP_ZERO, __fpclassifyl(0.0L));
#endif
}

TEST(math, finitef) {
  ASSERT_TRUE(finitef(123.0f));
  ASSERT_FALSE(finitef(HUGE_VALF));
}

TEST(math, __isfinite) {
#if defined(__GLIBC__)
#define __isfinite __finite
#endif
  ASSERT_TRUE(__isfinite(123.0));
  ASSERT_FALSE(__isfinite(HUGE_VAL));
}

TEST(math, __isfinitef) {
#if defined(__GLIBC__)
#define __isfinitef __finitef
#endif
  ASSERT_TRUE(__isfinitef(123.0f));
  ASSERT_FALSE(__isfinitef(HUGE_VALF));
}

TEST(math, __isfinitel) {
#if defined(__GLIBC__)
#define __isfinitel __finitel
#endif
#ifndef SKIP_LONG_DOUBLE_TESTS
  ASSERT_TRUE(__isfinitel(123.0L));
  ASSERT_FALSE(__isfinitel(HUGE_VALL));
#endif
}

TEST(math, finite) {
  ASSERT_TRUE(finite(123.0));
  ASSERT_FALSE(finite(HUGE_VAL));
}

TEST(math, isinf_function) {
  // The isinf macro deals with all three types; the isinf function is for doubles.
  ASSERT_FALSE((isinf)(123.0));
  ASSERT_TRUE((isinf)(HUGE_VAL));
}

TEST(math, __isinff) {
  ASSERT_FALSE(__isinff(123.0f));
  ASSERT_TRUE(__isinff(HUGE_VALF));
}

TEST(math, __isinfl) {
#ifndef SKIP_LONG_DOUBLE_TESTS
  ASSERT_FALSE(__isinfl(123.0L));
  ASSERT_TRUE(__isinfl(HUGE_VALL));
#endif
}

TEST(math, isnan_function) {
  // The isnan macro deals with all three types; the isnan function is for doubles.
  ASSERT_FALSE((isnan)(123.0));
  ASSERT_TRUE((isnan)(nan("")));
}

TEST(math, __isnanf) {
  ASSERT_FALSE(__isnanf(123.0f));
  ASSERT_TRUE(__isnanf(nanf("")));
}

TEST(math, __isnanl) {
#ifndef SKIP_LONG_DOUBLE_TESTS
  ASSERT_FALSE(__isnanl(123.0L));
  ASSERT_TRUE(__isnanl(nanl("")));
#endif
}

TEST(math, isnanf) {
  ASSERT_FALSE(isnanf(123.0f));
  ASSERT_TRUE(isnanf(nanf("")));
}

TEST(math, __isnormal) {
#if defined(__BIONIC__)
  ASSERT_TRUE(__isnormal(123.0));
  ASSERT_FALSE(__isnormal(double_subnormal()));
#else // __BIONIC__
  GTEST_LOG_(INFO) << "glibc doesn't have __isnormal.\n";
#endif // __BIONIC__
}

TEST(math, __isnormalf) {
#if defined(__BIONIC__)
  ASSERT_TRUE(__isnormalf(123.0f));
  ASSERT_FALSE(__isnormalf(float_subnormal()));
#else // __BIONIC__
  GTEST_LOG_(INFO) << "glibc doesn't have __isnormalf.\n";
#endif // __BIONIC__
}

TEST(math, __isnormall) {
#if defined(__BIONIC__) && !defined(SKIP_LONG_DOUBLE_TESTS)
  ASSERT_TRUE(__isnormall(123.0L));
  ASSERT_FALSE(__isnormall(ldouble_subnormal()));
#else // __BIONIC__
  GTEST_LOG_(INFO) << "glibc doesn't have __isnormall.\n";
#endif // __BIONIC__
}

TEST(math, __signbit) {
  ASSERT_EQ(0, __signbit(0.0));
  ASSERT_EQ(0, __signbit(1.0));
  ASSERT_NE(0, __signbit(-1.0));
}

TEST(math, __signbitf) {
  ASSERT_EQ(0, __signbitf(0.0f));
  ASSERT_EQ(0, __signbitf(1.0f));
  ASSERT_NE(0, __signbitf(-1.0f));
}

TEST(math, __signbitl) {
#ifndef SKIP_LONG_DOUBLE_TESTS
  ASSERT_EQ(0L, __signbitl(0.0L));
  ASSERT_EQ(0L, __signbitl(1.0L));
  ASSERT_NE(0L, __signbitl(-1.0L));
#endif
}

TEST(math, acos) {
  ASSERT_DOUBLE_EQ(M_PI/2.0, acos(0.0));
}

TEST(math, acosf) {
  ASSERT_FLOAT_EQ(static_cast<float>(M_PI)/2.0f, acosf(0.0f));
}

TEST(math, acosl) {
#ifndef SKIP_LONG_DOUBLE_TESTS
  ASSERT_DOUBLE_EQ(M_PI/2.0L, acosl(0.0L));
#endif
}

TEST(math, asin) {
  ASSERT_DOUBLE_EQ(0.0, asin(0.0));
}

TEST(math, asinf) {
  ASSERT_FLOAT_EQ(0.0f, asinf(0.0f));
}

TEST(math, asinl) {
#ifndef SKIP_LONG_DOUBLE_TESTS
  ASSERT_DOUBLE_EQ(0.0L, asinl(0.0L));
#endif
}

TEST(math, atan) {
  ASSERT_DOUBLE_EQ(0.0, atan(0.0));
}

TEST(math, atanf) {
  ASSERT_FLOAT_EQ(0.0f, atanf(0.0f));
}

TEST(math, atanl) {
#ifndef SKIP_LONG_DOUBLE_TESTS
  ASSERT_DOUBLE_EQ(0.0L, atanl(0.0L));
#endif
}

TEST(math, atan2) {
  ASSERT_DOUBLE_EQ(0.0, atan2(0.0, 0.0));
}

TEST(math, atan2f) {
  ASSERT_FLOAT_EQ(0.0f, atan2f(0.0f, 0.0f));
}

TEST(math, atan2l) {
#ifndef SKIP_LONG_DOUBLE_TESTS
  ASSERT_DOUBLE_EQ(0.0L, atan2l(0.0L, 0.0L));
#endif
}

TEST(math, cos) {
  ASSERT_DOUBLE_EQ(1.0, cos(0.0));
}

TEST(math, cosf) {
  ASSERT_FLOAT_EQ(1.0f, cosf(0.0f));
}

TEST(math, cosl) {
#ifndef SKIP_LONG_DOUBLE_TESTS
  ASSERT_DOUBLE_EQ(1.0L, cosl(0.0L));
#endif
}

TEST(math, sin) {
  ASSERT_DOUBLE_EQ(0.0, sin(0.0));
}

TEST(math, sinf) {
  ASSERT_FLOAT_EQ(0.0f, sinf(0.0f));
}

TEST(math, sinl) {
#ifndef SKIP_LONG_DOUBLE_TESTS
  ASSERT_DOUBLE_EQ(0.0L, sinl(0.0L));
#endif
}

TEST(math, tan) {
  ASSERT_DOUBLE_EQ(0.0, tan(0.0));
}

TEST(math, tanf) {
  ASSERT_FLOAT_EQ(0.0f, tanf(0.0f));
}

TEST(math, tanl) {
#ifndef SKIP_LONG_DOUBLE_TESTS
  ASSERT_DOUBLE_EQ(0.0L, tanl(0.0L));
#endif
}

TEST(math, acosh) {
  ASSERT_DOUBLE_EQ(0.0, acosh(1.0));
}

TEST(math, acoshf) {
  ASSERT_FLOAT_EQ(0.0f, acoshf(1.0f));
}

TEST(math, acoshl) {
#ifndef SKIP_LONG_DOUBLE_TESTS
  ASSERT_DOUBLE_EQ(0.0L, acoshl(1.0L));
#endif
}

TEST(math, asinh) {
  ASSERT_DOUBLE_EQ(0.0, asinh(0.0));
}

TEST(math, asinhf) {
  ASSERT_FLOAT_EQ(0.0f, asinhf(0.0f));
}

TEST(math, asinhl) {
#ifndef SKIP_LONG_DOUBLE_TESTS
  ASSERT_DOUBLE_EQ(0.0L, asinhl(0.0L));
#endif
}

TEST(math, atanh) {
  ASSERT_DOUBLE_EQ(0.0, atanh(0.0));
}

TEST(math, atanhf) {
  ASSERT_FLOAT_EQ(0.0f, atanhf(0.0f));
}

TEST(math, atanhl) {
#ifndef SKIP_LONG_DOUBLE_TESTS
  ASSERT_DOUBLE_EQ(0.0L, atanhl(0.0L));
#endif
}

TEST(math, cosh) {
  ASSERT_DOUBLE_EQ(1.0, cosh(0.0));
}

TEST(math, coshf) {
  ASSERT_FLOAT_EQ(1.0f, coshf(0.0f));
}

TEST(math, coshl) {
#ifndef SKIP_LONG_DOUBLE_TESTS
  ASSERT_DOUBLE_EQ(1.0L, coshl(0.0L));
#endif
}

TEST(math, sinh) {
  ASSERT_DOUBLE_EQ(0.0, sinh(0.0));
}

TEST(math, sinhf) {
  ASSERT_FLOAT_EQ(0.0f, sinhf(0.0f));
}

TEST(math, sinhl) {
#ifndef SKIP_LONG_DOUBLE_TESTS
  ASSERT_DOUBLE_EQ(0.0L, sinhl(0.0L));
#endif
}

TEST(math, tanh) {
  ASSERT_DOUBLE_EQ(0.0, tanh(0.0));
}

TEST(math, tanhf) {
  ASSERT_FLOAT_EQ(0.0f, tanhf(0.0f));
}

TEST(math, tanhl) {
#ifndef SKIP_LONG_DOUBLE_TESTS
  ASSERT_DOUBLE_EQ(0.0L, tanhl(0.0L));
#endif
}

TEST(math, log) {
  ASSERT_DOUBLE_EQ(1.0, log(M_E));
}

TEST(math, logf) {
  ASSERT_FLOAT_EQ(1.0f, logf(static_cast<float>(M_E)));
}

TEST(math, logl) {
#ifndef SKIP_LONG_DOUBLE_TESTS
  ASSERT_DOUBLE_EQ(1.0L, logl(M_E));
#endif
}

TEST(math, log2) {
  ASSERT_DOUBLE_EQ(12.0, log2(4096.0));
}

TEST(math, log2f) {
  ASSERT_FLOAT_EQ(12.0f, log2f(4096.0f));
}

TEST(math, log2l) {
#ifndef SKIP_LONG_DOUBLE_TESTS
  ASSERT_DOUBLE_EQ(12.0L, log2l(4096.0L));
#endif
}

TEST(math, log10) {
  ASSERT_DOUBLE_EQ(3.0, log10(1000.0));
}

TEST(math, log10f) {
  ASSERT_FLOAT_EQ(3.0f, log10f(1000.0f));
}

TEST(math, log10l) {
#ifndef SKIP_LONG_DOUBLE_TESTS
  ASSERT_DOUBLE_EQ(3.0L, log10l(1000.0L));
#endif
}

TEST(math, cbrt) {
  ASSERT_DOUBLE_EQ(3.0, cbrt(27.0));
}

TEST(math, cbrtf) {
  ASSERT_FLOAT_EQ(3.0f, cbrtf(27.0f));
}

TEST(math, cbrtl) {
#ifndef SKIP_LONG_DOUBLE_TESTS
  ASSERT_DOUBLE_EQ(3.0L, cbrtl(27.0L));
#endif
}

TEST(math, sqrt) {
  ASSERT_DOUBLE_EQ(2.0, sqrt(4.0));
}

TEST(math, sqrtf) {
  ASSERT_FLOAT_EQ(2.0f, sqrtf(4.0f));
}

TEST(math, sqrtl) {
#ifndef SKIP_LONG_DOUBLE_TESTS
  ASSERT_DOUBLE_EQ(2.0L, sqrtl(4.0L));
#endif
}

TEST(math, exp) {
  ASSERT_DOUBLE_EQ(1.0, exp(0.0));
  ASSERT_DOUBLE_EQ(M_E, exp(1.0));
}

TEST(math, expf) {
  ASSERT_FLOAT_EQ(1.0f, expf(0.0f));
  ASSERT_FLOAT_EQ(static_cast<float>(M_E), expf(1.0f));
}

TEST(math, expl) {
#ifndef SKIP_LONG_DOUBLE_TESTS
  ASSERT_DOUBLE_EQ(1.0L, expl(0.0L));
  ASSERT_DOUBLE_EQ(M_E, expl(1.0L));
#endif
}

TEST(math, exp2) {
  ASSERT_DOUBLE_EQ(8.0, exp2(3.0));
}

TEST(math, exp2f) {
  ASSERT_FLOAT_EQ(8.0f, exp2f(3.0f));
}

TEST(math, exp2l) {
#ifndef SKIP_LONG_DOUBLE_TESTS
  ASSERT_DOUBLE_EQ(8.0L, exp2l(3.0L));
#endif
}

TEST(math, expm1) {
  ASSERT_DOUBLE_EQ(M_E - 1.0, expm1(1.0));
}

TEST(math, expm1f) {
  ASSERT_FLOAT_EQ(static_cast<float>(M_E) - 1.0f, expm1f(1.0f));
}

TEST(math, expm1l) {
#ifndef SKIP_LONG_DOUBLE_TESTS
  ASSERT_DOUBLE_EQ(M_E - 1.0L, expm1l(1.0L));
#endif
}

TEST(math, pow) {
  ASSERT_TRUE(isnan(pow(nan(""), 3.0)));
  ASSERT_DOUBLE_EQ(1.0, (pow(1.0, nan(""))));
  ASSERT_TRUE(isnan(pow(2.0, nan(""))));
  ASSERT_DOUBLE_EQ(8.0, pow(2.0, 3.0));
}

TEST(math, powf) {
  ASSERT_TRUE(isnanf(powf(nanf(""), 3.0f)));
  ASSERT_FLOAT_EQ(1.0f, (powf(1.0f, nanf(""))));
  ASSERT_TRUE(isnanf(powf(2.0f, nanf(""))));
  ASSERT_FLOAT_EQ(8.0f, powf(2.0f, 3.0f));
}

TEST(math, powl) {
#ifndef SKIP_LONG_DOUBLE_TESTS
  ASSERT_TRUE(__isnanl(powl(nanl(""), 3.0L)));
  ASSERT_DOUBLE_EQ(1.0L, (powl(1.0L, nanl(""))));
  ASSERT_TRUE(__isnanl(powl(2.0L, nanl(""))));
  ASSERT_DOUBLE_EQ(8.0L, powl(2.0L, 3.0L));
#endif
}

TEST(math, ceil) {
  ASSERT_DOUBLE_EQ(1.0, ceil(0.9));
}

TEST(math, ceilf) {
  ASSERT_FLOAT_EQ(1.0f, ceilf(0.9f));
}

TEST(math, ceill) {
#ifndef SKIP_LONG_DOUBLE_TESTS
  ASSERT_DOUBLE_EQ(1.0L, ceill(0.9L));
#endif
}

TEST(math, floor) {
  ASSERT_DOUBLE_EQ(1.0, floor(1.1));
}

TEST(math, floorf) {
  ASSERT_FLOAT_EQ(1.0f, floorf(1.1f));
}

TEST(math, floorl) {
#ifndef SKIP_LONG_DOUBLE_TESTS
  ASSERT_DOUBLE_EQ(1.0L, floorl(1.1L));
#endif
}

TEST(math, fabs) {
  ASSERT_DOUBLE_EQ(1.0, fabs(-1.0));
}

TEST(math, fabsf) {
  ASSERT_FLOAT_EQ(1.0f, fabsf(-1.0f));
}

TEST(math, fabsl) {
#ifndef SKIP_LONG_DOUBLE_TESTS
  ASSERT_DOUBLE_EQ(1.0L, fabsl(-1.0L));
#endif
}

TEST(math, ldexp) {
  ASSERT_DOUBLE_EQ(16.0, ldexp(2.0, 3.0));
}

TEST(math, ldexpf) {
  ASSERT_FLOAT_EQ(16.0f, ldexpf(2.0f, 3.0f));
}

TEST(math, ldexpl) {
#ifndef SKIP_LONG_DOUBLE_TESTS
  ASSERT_DOUBLE_EQ(16.0L, ldexpl(2.0L, 3.0));
#endif
}

TEST(math, fmod) {
  ASSERT_DOUBLE_EQ(2.0, fmod(12.0, 10.0));
}

TEST(math, fmodf) {
  ASSERT_FLOAT_EQ(2.0f, fmodf(12.0f, 10.0f));
}

TEST(math, fmodl) {
#ifndef SKIP_LONG_DOUBLE_TESTS
  ASSERT_DOUBLE_EQ(2.0L, fmodl(12.0L, 10.0L));
#endif
}

TEST(math, remainder) {
  ASSERT_DOUBLE_EQ(2.0, remainder(12.0, 10.0));
}

TEST(math, remainderf) {
  ASSERT_FLOAT_EQ(2.0f, remainderf(12.0f, 10.0f));
}

TEST(math, remainderl) {
#ifndef SKIP_LONG_DOUBLE_TESTS
  ASSERT_DOUBLE_EQ(2.0L, remainderl(12.0L, 10.0L));
#endif
}

TEST(math, drem) {
  ASSERT_DOUBLE_EQ(2.0, drem(12.0, 10.0));
}

TEST(math, dremf) {
  ASSERT_FLOAT_EQ(2.0f, dremf(12.0f, 10.0f));
}

TEST(math, fmax) {
  ASSERT_DOUBLE_EQ(12.0, fmax(12.0, 10.0));
  ASSERT_DOUBLE_EQ(12.0, fmax(12.0, nan("")));
  ASSERT_DOUBLE_EQ(12.0, fmax(nan(""), 12.0));
}

TEST(math, fmaxf) {
  ASSERT_FLOAT_EQ(12.0f, fmaxf(12.0f, 10.0f));
  ASSERT_FLOAT_EQ(12.0f, fmaxf(12.0f, nanf("")));
  ASSERT_FLOAT_EQ(12.0f, fmaxf(nanf(""), 12.0f));
}

TEST(math, fmaxl) {
#ifndef SKIP_LONG_DOUBLE_TESTS
  ASSERT_DOUBLE_EQ(12.0L, fmaxl(12.0L, 10.0L));
  ASSERT_DOUBLE_EQ(12.0L, fmaxl(12.0L, nanl("")));
  ASSERT_DOUBLE_EQ(12.0L, fmaxl(nanl(""), 12.0L));
#endif
}

TEST(math, fmin) {
  ASSERT_DOUBLE_EQ(10.0, fmin(12.0, 10.0));
  ASSERT_DOUBLE_EQ(12.0, fmin(12.0, nan("")));
  ASSERT_DOUBLE_EQ(12.0, fmin(nan(""), 12.0));
}

TEST(math, fminf) {
  ASSERT_FLOAT_EQ(10.0f, fminf(12.0f, 10.0f));
  ASSERT_FLOAT_EQ(12.0f, fminf(12.0f, nanf("")));
  ASSERT_FLOAT_EQ(12.0f, fminf(nanf(""), 12.0f));
}

TEST(math, fminl) {
#ifndef SKIP_LONG_DOUBLE_TESTS
  ASSERT_DOUBLE_EQ(10.0L, fminl(12.0L, 10.0L));
  ASSERT_DOUBLE_EQ(12.0L, fminl(12.0L, nanl("")));
  ASSERT_DOUBLE_EQ(12.0L, fminl(nanl(""), 12.0L));
#endif
}

TEST(math, fma) {
  ASSERT_DOUBLE_EQ(10.0, fma(2.0, 3.0, 4.0));
}

TEST(math, fmaf) {
  ASSERT_FLOAT_EQ(10.0f, fmaf(2.0f, 3.0f, 4.0f));
}

TEST(math, fmal) {
#ifndef SKIP_LONG_DOUBLE_TESTS
  ASSERT_DOUBLE_EQ(10.0L, fmal(2.0L, 3.0L, 4.0L));
#endif
}

TEST(math, hypot) {
  ASSERT_DOUBLE_EQ(5.0, hypot(3.0, 4.0));
}

TEST(math, hypotf) {
  ASSERT_FLOAT_EQ(5.0f, hypotf(3.0f, 4.0f));
}

TEST(math, hypotl) {
#ifndef SKIP_LONG_DOUBLE_TESTS
  ASSERT_DOUBLE_EQ(5.0L, hypotl(3.0L, 4.0L));
#endif
}

TEST(math, erf) {
  ASSERT_DOUBLE_EQ(0.84270079294971489, erf(1.0));
}

TEST(math, erff) {
  ASSERT_FLOAT_EQ(0.84270078f, erff(1.0f));
}

TEST(math, erfl) {
#ifndef SKIP_LONG_DOUBLE_TESTS
  ASSERT_DOUBLE_EQ(0.84270079294971489L, erfl(1.0L));
#endif
}

TEST(math, erfc) {
  ASSERT_DOUBLE_EQ(0.15729920705028513, erfc(1.0));
}

TEST(math, erfcf) {
  ASSERT_FLOAT_EQ(0.15729921f, erfcf(1.0f));
}

TEST(math, erfcl) {
#ifndef SKIP_LONG_DOUBLE_TESTS
  ASSERT_DOUBLE_EQ(0.15729920705028513l, erfcl(1.0L));
#endif
}

TEST(math, lrint) {
  auto guard = make_scope_guard([]() {
    fesetenv(FE_DFL_ENV);
  });

  fesetround(FE_UPWARD); // lrint/lrintf/lrintl obey the rounding mode.
  ASSERT_EQ(1235, lrint(1234.01));
  ASSERT_EQ(1235, lrintf(1234.01f));
#ifndef SKIP_LONG_DOUBLE_TESTS
  ASSERT_EQ(1235, lrintl(1234.01L));
#endif
  fesetround(FE_TOWARDZERO); // lrint/lrintf/lrintl obey the rounding mode.
  ASSERT_EQ(1234, lrint(1234.01));
  ASSERT_EQ(1234, lrintf(1234.01f));
#ifndef SKIP_LONG_DOUBLE_TESTS
  ASSERT_EQ(1234, lrintl(1234.01L));
#endif

  fesetround(FE_UPWARD); // llrint/llrintf/llrintl obey the rounding mode.
  ASSERT_EQ(1235L, llrint(1234.01));
  ASSERT_EQ(1235L, llrintf(1234.01f));
#ifndef SKIP_LONG_DOUBLE_TESTS
  ASSERT_EQ(1235L, llrintl(1234.01L));
#endif
  fesetround(FE_TOWARDZERO); // llrint/llrintf/llrintl obey the rounding mode.
  ASSERT_EQ(1234L, llrint(1234.01));
  ASSERT_EQ(1234L, llrintf(1234.01f));
#ifndef SKIP_LONG_DOUBLE_TESTS
  ASSERT_EQ(1234L, llrintl(1234.01L));
#endif
}

TEST(math, rint) {
  auto guard = make_scope_guard([]() {
    fesetenv(FE_DFL_ENV);
  });

  fesetround(FE_UPWARD); // rint/rintf/rintl obey the rounding mode.
  feclearexcept(FE_ALL_EXCEPT); // rint/rintf/rintl do set the FE_INEXACT flag.
  ASSERT_EQ(1234.0, rint(1234.0));
  ASSERT_TRUE((fetestexcept(FE_ALL_EXCEPT) & FE_INEXACT) == 0);
  ASSERT_EQ(1235.0, rint(1234.01));
  ASSERT_TRUE((fetestexcept(FE_ALL_EXCEPT) & FE_INEXACT) != 0);

  feclearexcept(FE_ALL_EXCEPT); // rint/rintf/rintl do set the FE_INEXACT flag.
  ASSERT_EQ(1234.0f, rintf(1234.0f));
  ASSERT_TRUE((fetestexcept(FE_ALL_EXCEPT) & FE_INEXACT) == 0);
  ASSERT_EQ(1235.0f, rintf(1234.01f));
  ASSERT_TRUE((fetestexcept(FE_ALL_EXCEPT) & FE_INEXACT) != 0);

#ifndef SKIP_LONG_DOUBLE_TESTS
  feclearexcept(FE_ALL_EXCEPT); // rint/rintf/rintl do set the FE_INEXACT flag.
  ASSERT_EQ(1234.0, rintl(1234.0L));
  ASSERT_TRUE((fetestexcept(FE_ALL_EXCEPT) & FE_INEXACT) == 0);
  ASSERT_EQ(1235.0, rintl(1234.01L));
  ASSERT_TRUE((fetestexcept(FE_ALL_EXCEPT) & FE_INEXACT) != 0);
#endif

  fesetround(FE_TOWARDZERO); // rint/rintf obey the rounding mode.
  ASSERT_EQ(1234.0, rint(1234.01));
  ASSERT_EQ(1234.0f, rintf(1234.01f));
#ifndef SKIP_LONG_DOUBLE_TESTS
  ASSERT_EQ(1234.0, rintl(1234.01L));
#endif
}

TEST(math, nearbyint) {
  auto guard = make_scope_guard([]() {
    fesetenv(FE_DFL_ENV);
  });
  fesetround(FE_UPWARD); // nearbyint/nearbyintf/nearbyintl obey the rounding mode.
  feclearexcept(FE_ALL_EXCEPT); // nearbyint/nearbyintf/nearbyintl don't set the FE_INEXACT flag.
  ASSERT_EQ(1234.0, nearbyint(1234.0));
  ASSERT_TRUE((fetestexcept(FE_ALL_EXCEPT) & FE_INEXACT) == 0);
  ASSERT_EQ(1235.0, nearbyint(1234.01));
  ASSERT_TRUE((fetestexcept(FE_ALL_EXCEPT) & FE_INEXACT) == 0);

  feclearexcept(FE_ALL_EXCEPT);
  ASSERT_EQ(1234.0f, nearbyintf(1234.0f));
  ASSERT_TRUE((fetestexcept(FE_ALL_EXCEPT) & FE_INEXACT) == 0);
  ASSERT_EQ(1235.0f, nearbyintf(1234.01f));
  ASSERT_TRUE((fetestexcept(FE_ALL_EXCEPT) & FE_INEXACT) == 0);

#ifndef SKIP_LONG_DOUBLE_TESTS
  feclearexcept(FE_ALL_EXCEPT); // nearbyint/nearbyintf/nearbyintl don't set the FE_INEXACT flag.
  ASSERT_EQ(1234.0, nearbyintl(1234.0L));
  ASSERT_TRUE((fetestexcept(FE_ALL_EXCEPT) & FE_INEXACT) == 0);
  ASSERT_EQ(1235.0, nearbyintl(1234.01L));
  ASSERT_TRUE((fetestexcept(FE_ALL_EXCEPT) & FE_INEXACT) == 0);
#endif

  fesetround(FE_TOWARDZERO); // nearbyint/nearbyintf/nearbyintl obey the rounding mode.
  ASSERT_EQ(1234.0, nearbyint(1234.01));
  ASSERT_EQ(1234.0f, nearbyintf(1234.01f));
#ifndef SKIP_LONG_DOUBLE_TESTS
  ASSERT_EQ(1234.0, nearbyintl(1234.01L));
#endif
}

TEST(math, lround) {
  auto guard = make_scope_guard([]() {
    fesetenv(FE_DFL_ENV);
  });
  fesetround(FE_UPWARD); // lround ignores the rounding mode.
  ASSERT_EQ(1234, lround(1234.01));
  ASSERT_EQ(1234, lroundf(1234.01f));
#ifndef SKIP_LONG_DOUBLE_TESTS
  ASSERT_EQ(1234, lroundl(1234.01L));
#endif
}

TEST(math, llround) {
  auto guard = make_scope_guard([]() {
    fesetenv(FE_DFL_ENV);
  });
  fesetround(FE_UPWARD); // llround ignores the rounding mode.
  ASSERT_EQ(1234L, llround(1234.01));
  ASSERT_EQ(1234L, llroundf(1234.01f));
#ifndef SKIP_LONG_DOUBLE_TESTS
  ASSERT_EQ(1234L, llroundl(1234.01L));
#endif
}

TEST(math, ilogb) {
  ASSERT_EQ(FP_ILOGB0, ilogb(0.0));
  ASSERT_EQ(FP_ILOGBNAN, ilogb(nan("")));
  ASSERT_EQ(INT_MAX, ilogb(HUGE_VAL));
  ASSERT_EQ(0, ilogb(1.0));
  ASSERT_EQ(3, ilogb(10.0));
}

TEST(math, ilogbf) {
  ASSERT_EQ(FP_ILOGB0, ilogbf(0.0f));
  ASSERT_EQ(FP_ILOGBNAN, ilogbf(nanf("")));
  ASSERT_EQ(INT_MAX, ilogbf(HUGE_VALF));
  ASSERT_EQ(0, ilogbf(1.0f));
  ASSERT_EQ(3, ilogbf(10.0f));
}

TEST(math, ilogbl) {
#ifndef SKIP_LONG_DOUBLE_TESTS
  ASSERT_EQ(FP_ILOGB0, ilogbl(0.0L));
  ASSERT_EQ(FP_ILOGBNAN, ilogbl(nanl("")));
  ASSERT_EQ(INT_MAX, ilogbl(HUGE_VALL));
  ASSERT_EQ(0L, ilogbl(1.0L));
  ASSERT_EQ(3L, ilogbl(10.0L));
#endif
}

TEST(math, logb) {
  ASSERT_EQ(-HUGE_VAL, logb(0.0));
  ASSERT_TRUE(isnan(logb(nan(""))));
  ASSERT_TRUE(isinf(logb(HUGE_VAL)));
  ASSERT_EQ(0.0, logb(1.0));
  ASSERT_EQ(3.0, logb(10.0));
}

TEST(math, logbf) {
  ASSERT_EQ(-HUGE_VALF, logbf(0.0f));
  ASSERT_TRUE(isnanf(logbf(nanf(""))));
  ASSERT_TRUE(__isinff(logbf(HUGE_VALF)));
  ASSERT_EQ(0.0f, logbf(1.0f));
  ASSERT_EQ(3.0f, logbf(10.0f));
}

TEST(math, logbl) {
#ifndef SKIP_LONG_DOUBLE_TESTS
  ASSERT_EQ(-HUGE_VAL, logbl(0.0L));
  ASSERT_TRUE(isnan(logbl(nanl(""))));
  ASSERT_TRUE(isinf(logbl(HUGE_VALL)));
  ASSERT_EQ(0.0L, logbl(1.0L));
  ASSERT_EQ(3.0L, logbl(10.0L));
#endif
}

TEST(math, log1p) {
  ASSERT_EQ(-HUGE_VAL, log1p(-1.0));
  ASSERT_TRUE(isnan(log1p(nan(""))));
  ASSERT_TRUE(isinf(log1p(HUGE_VAL)));
  ASSERT_DOUBLE_EQ(1.0, log1p(M_E - 1.0));
}

TEST(math, log1pf) {
  ASSERT_EQ(-HUGE_VALF, log1pf(-1.0f));
  ASSERT_TRUE(isnanf(log1pf(nanf(""))));
  ASSERT_TRUE(__isinff(log1pf(HUGE_VALF)));
  ASSERT_FLOAT_EQ(1.0f, log1pf(static_cast<float>(M_E) - 1.0f));
}

TEST(math, log1pl) {
#ifndef SKIP_LONG_DOUBLE_TESTS
  ASSERT_EQ(-HUGE_VALL, log1pl(-1.0L));
  ASSERT_TRUE(isnan(log1pl(nanl(""))));
  ASSERT_TRUE(isinf(log1pl(HUGE_VALL)));
  ASSERT_DOUBLE_EQ(1.0L, log1pl(M_E - 1.0L));
#endif
}

TEST(math, fdim) {
  ASSERT_DOUBLE_EQ(0.0, fdim(1.0, 1.0));
  ASSERT_DOUBLE_EQ(1.0, fdim(2.0, 1.0));
  ASSERT_DOUBLE_EQ(0.0, fdim(1.0, 2.0));
}

TEST(math, fdimf) {
  ASSERT_FLOAT_EQ(0.0f, fdimf(1.0f, 1.0f));
  ASSERT_FLOAT_EQ(1.0f, fdimf(2.0f, 1.0f));
  ASSERT_FLOAT_EQ(0.0f, fdimf(1.0f, 2.0f));
}

TEST(math, fdiml) {
#ifndef SKIP_LONG_DOUBLE_TESTS
  ASSERT_DOUBLE_EQ(0.0L, fdiml(1.0L, 1.0L));
  ASSERT_DOUBLE_EQ(1.0L, fdiml(2.0L, 1.0L));
  ASSERT_DOUBLE_EQ(0.0L, fdiml(1.0L, 2.0L));
#endif
}

TEST(math, round) {
  auto guard = make_scope_guard([]() {
    fesetenv(FE_DFL_ENV);
  });
  fesetround(FE_TOWARDZERO); // round ignores the rounding mode and always rounds away from zero.
  ASSERT_DOUBLE_EQ(1.0, round(0.5));
  ASSERT_DOUBLE_EQ(-1.0, round(-0.5));
  ASSERT_DOUBLE_EQ(0.0, round(0.0));
  ASSERT_DOUBLE_EQ(-0.0, round(-0.0));
  ASSERT_TRUE(isnan(round(nan(""))));
  ASSERT_DOUBLE_EQ(HUGE_VAL, round(HUGE_VAL));
}

TEST(math, roundf) {
  auto guard = make_scope_guard([]() {
    fesetenv(FE_DFL_ENV);
  });
  fesetround(FE_TOWARDZERO); // roundf ignores the rounding mode and always rounds away from zero.
  ASSERT_FLOAT_EQ(1.0f, roundf(0.5f));
  ASSERT_FLOAT_EQ(-1.0f, roundf(-0.5f));
  ASSERT_FLOAT_EQ(0.0f, roundf(0.0f));
  ASSERT_FLOAT_EQ(-0.0f, roundf(-0.0f));
  ASSERT_TRUE(isnanf(roundf(nanf(""))));
  ASSERT_FLOAT_EQ(HUGE_VALF, roundf(HUGE_VALF));
}

TEST(math, roundl) {
  auto guard = make_scope_guard([]() {
    fesetenv(FE_DFL_ENV);
  });
  fesetround(FE_TOWARDZERO); // roundl ignores the rounding mode and always rounds away from zero.
#ifndef SKIP_LONG_DOUBLE_TESTS
  ASSERT_DOUBLE_EQ(1.0L, roundl(0.5L));
  ASSERT_DOUBLE_EQ(-1.0L, roundl(-0.5L));
  ASSERT_DOUBLE_EQ(0.0L, roundl(0.0L));
  ASSERT_DOUBLE_EQ(-0.0L, roundl(-0.0L));
  ASSERT_TRUE(isnan(roundl(nanl(""))));
  ASSERT_DOUBLE_EQ(HUGE_VALL, roundl(HUGE_VALL));
#endif
}

TEST(math, trunc) {
  auto guard = make_scope_guard([]() {
    fesetenv(FE_DFL_ENV);
  });
  fesetround(FE_UPWARD); // trunc ignores the rounding mode and always rounds toward zero.
  ASSERT_DOUBLE_EQ(1.0, trunc(1.5));
  ASSERT_DOUBLE_EQ(-1.0, trunc(-1.5));
  ASSERT_DOUBLE_EQ(0.0, trunc(0.0));
  ASSERT_DOUBLE_EQ(-0.0, trunc(-0.0));
  ASSERT_TRUE(isnan(trunc(nan(""))));
  ASSERT_DOUBLE_EQ(HUGE_VAL, trunc(HUGE_VAL));
}

TEST(math, truncf) {
  auto guard = make_scope_guard([]() {
    fesetenv(FE_DFL_ENV);
  });
  fesetround(FE_UPWARD); // truncf ignores the rounding mode and always rounds toward zero.
  ASSERT_FLOAT_EQ(1.0f, truncf(1.5f));
  ASSERT_FLOAT_EQ(-1.0f, truncf(-1.5f));
  ASSERT_FLOAT_EQ(0.0f, truncf(0.0f));
  ASSERT_FLOAT_EQ(-0.0f, truncf(-0.0f));
  ASSERT_TRUE(isnan(truncf(nanf(""))));
  ASSERT_FLOAT_EQ(HUGE_VALF, truncf(HUGE_VALF));
}

TEST(math, truncl) {
  auto guard = make_scope_guard([]() {
    fesetenv(FE_DFL_ENV);
  });
  fesetround(FE_UPWARD); // truncl ignores the rounding mode and always rounds toward zero.
#ifndef SKIP_LONG_DOUBLE_TESTS
  ASSERT_DOUBLE_EQ(1.0L, truncl(1.5L));
  ASSERT_DOUBLE_EQ(-1.0L, truncl(-1.5L));
  ASSERT_DOUBLE_EQ(0.0L, truncl(0.0L));
  ASSERT_DOUBLE_EQ(-0.0L, truncl(-0.0L));
  ASSERT_TRUE(isnan(truncl(nan(""))));
  ASSERT_DOUBLE_EQ(HUGE_VALL, truncl(HUGE_VALL));
#endif
}

TEST(math, nextafter) {
  ASSERT_DOUBLE_EQ(0.0, nextafter(0.0, 0.0));
  ASSERT_DOUBLE_EQ(4.9406564584124654e-324, nextafter(0.0, 1.0));
  ASSERT_DOUBLE_EQ(-4.9406564584124654e-324, nextafter(0.0, -1.0));
}

TEST(math, nextafterf) {
  ASSERT_FLOAT_EQ(0.0f, nextafterf(0.0f, 0.0f));
  ASSERT_FLOAT_EQ(1.4012985e-45f, nextafterf(0.0f, 1.0f));
  ASSERT_FLOAT_EQ(-1.4012985e-45f, nextafterf(0.0f, -1.0f));
}

TEST(math, nextafterl) {
  ASSERT_DOUBLE_EQ(0.0L, nextafterl(0.0L, 0.0L));
  // Use a runtime value to accomodate the case when
  // sizeof(double) == sizeof(long double)
  long double smallest_positive = ldexpl(1.0L, LDBL_MIN_EXP - LDBL_MANT_DIG);
  ASSERT_DOUBLE_EQ(smallest_positive, nextafterl(0.0L, 1.0L));
  ASSERT_DOUBLE_EQ(-smallest_positive, nextafterl(0.0L, -1.0L));
}

TEST(math, nexttoward) {
  ASSERT_DOUBLE_EQ(0.0, nexttoward(0.0, 0.0L));
  ASSERT_DOUBLE_EQ(4.9406564584124654e-324, nexttoward(0.0, 1.0L));
  ASSERT_DOUBLE_EQ(-4.9406564584124654e-324, nexttoward(0.0, -1.0L));
}

TEST(math, nexttowardf) {
  ASSERT_FLOAT_EQ(0.0f, nexttowardf(0.0f, 0.0L));
  ASSERT_FLOAT_EQ(1.4012985e-45f, nexttowardf(0.0f, 1.0L));
  ASSERT_FLOAT_EQ(-1.4012985e-45f, nexttowardf(0.0f, -1.0L));
}

TEST(math, nexttowardl) {
  ASSERT_DOUBLE_EQ(0.0L, nexttowardl(0.0L, 0.0L));
  // Use a runtime value to accomodate the case when
  // sizeof(double) == sizeof(long double)
  long double smallest_positive = ldexpl(1.0L, LDBL_MIN_EXP - LDBL_MANT_DIG);
  ASSERT_DOUBLE_EQ(smallest_positive, nexttowardl(0.0L, 1.0L));
  ASSERT_DOUBLE_EQ(-smallest_positive, nexttowardl(0.0L, -1.0L));
}

TEST(math, copysign) {
  ASSERT_DOUBLE_EQ(0.0, copysign(0.0, 1.0));
  ASSERT_DOUBLE_EQ(-0.0, copysign(0.0, -1.0));
  ASSERT_DOUBLE_EQ(2.0, copysign(2.0, 1.0));
  ASSERT_DOUBLE_EQ(-2.0, copysign(2.0, -1.0));
}

TEST(math, copysignf) {
  ASSERT_FLOAT_EQ(0.0f, copysignf(0.0f, 1.0f));
  ASSERT_FLOAT_EQ(-0.0f, copysignf(0.0f, -1.0f));
  ASSERT_FLOAT_EQ(2.0f, copysignf(2.0f, 1.0f));
  ASSERT_FLOAT_EQ(-2.0f, copysignf(2.0f, -1.0f));
}

TEST(math, copysignl) {
#ifndef SKIP_LONG_DOUBLE_TESTS
  ASSERT_DOUBLE_EQ(0.0L, copysignl(0.0L, 1.0L));
  ASSERT_DOUBLE_EQ(-0.0L, copysignl(0.0L, -1.0L));
  ASSERT_DOUBLE_EQ(2.0L, copysignl(2.0L, 1.0L));
  ASSERT_DOUBLE_EQ(-2.0L, copysignl(2.0L, -1.0L));
#endif
}

TEST(math, significand) {
  ASSERT_DOUBLE_EQ(0.0, significand(0.0));
  ASSERT_DOUBLE_EQ(1.2, significand(1.2));
  ASSERT_DOUBLE_EQ(1.53125, significand(12.25));
}

TEST(math, significandf) {
  ASSERT_FLOAT_EQ(0.0f, significandf(0.0f));
  ASSERT_FLOAT_EQ(1.2f, significandf(1.2f));
  ASSERT_FLOAT_EQ(1.53125f, significandf(12.25f));
}

TEST(math, significandl) {
#ifndef SKIP_LONG_DOUBLE_TESTS
  ASSERT_DOUBLE_EQ(0.0L, significandl(0.0L));
  ASSERT_DOUBLE_EQ(1.2L, significandl(1.2L));
  ASSERT_DOUBLE_EQ(1.53125L, significandl(12.25L));
#endif
}

TEST(math, scalb) {
  ASSERT_DOUBLE_EQ(12.0, scalb(3.0, 2.0));
}

TEST(math, scalbf) {
  ASSERT_FLOAT_EQ(12.0f, scalbf(3.0f, 2.0f));
}

TEST(math, scalbln) {
  ASSERT_DOUBLE_EQ(12.0, scalbln(3.0, 2L));
}

TEST(math, scalblnf) {
  ASSERT_FLOAT_EQ(12.0f, scalblnf(3.0f, 2L));
}

TEST(math, scalblnl) {
#ifndef SKIP_LONG_DOUBLE_TESTS
  ASSERT_DOUBLE_EQ(12.0L, scalblnl(3.0L, 2L));
#endif
}

TEST(math, scalbn) {
  ASSERT_DOUBLE_EQ(12.0, scalbn(3.0, 2));
}

TEST(math, scalbnf) {
  ASSERT_FLOAT_EQ(12.0f, scalbnf(3.0f, 2));
}

TEST(math, scalbnl) {
#ifndef SKIP_LONG_DOUBLE_TESTS
  ASSERT_DOUBLE_EQ(12.0L, scalbnl(3.0L, 2));
#endif
}

TEST(math, gamma) {
  ASSERT_DOUBLE_EQ(log(24.0), gamma(5.0));
}

TEST(math, gammaf) {
  ASSERT_FLOAT_EQ(logf(24.0f), gammaf(5.0f));
}

TEST(math, gamma_r) {
#if defined(__BIONIC__)
  int sign;
  ASSERT_DOUBLE_EQ(log(24.0), gamma_r(5.0, &sign));
  ASSERT_EQ(1, sign);
#else // __BIONIC__
  GTEST_LOG_(INFO) << "glibc doesn't have gamma_r.\n";
#endif // __BIONIC__
}

TEST(math, gammaf_r) {
#if defined(__BIONIC__)
  int sign;
  ASSERT_FLOAT_EQ(logf(24.0f), gammaf_r(5.0f, &sign));
  ASSERT_EQ(1, sign);
#else // __BIONIC__
  GTEST_LOG_(INFO) << "glibc doesn't have gammaf_r.\n";
#endif // __BIONIC__
}

TEST(math, lgamma) {
  ASSERT_DOUBLE_EQ(log(24.0), lgamma(5.0));
}

TEST(math, lgammaf) {
  ASSERT_FLOAT_EQ(logf(24.0f), lgammaf(5.0f));
}

TEST(math, lgammal) {
#ifndef SKIP_LONG_DOUBLE_TESTS
  ASSERT_DOUBLE_EQ(logl(24.0L), lgammal(5.0L));
#endif
}

TEST(math, lgamma_r) {
  int sign;
  ASSERT_DOUBLE_EQ(log(24.0), lgamma_r(5.0, &sign));
  ASSERT_EQ(1, sign);
}

TEST(math, lgamma_r_17471883) {
  int sign;

  sign = 0;
  ASSERT_DOUBLE_EQ(HUGE_VAL, lgamma_r(0.0, &sign));
  ASSERT_EQ(1, sign);
  sign = 0;
  ASSERT_DOUBLE_EQ(HUGE_VAL, lgamma_r(-0.0, &sign));
  ASSERT_EQ(-1, sign);
}

TEST(math, lgammaf_r) {
  int sign;
  ASSERT_FLOAT_EQ(logf(24.0f), lgammaf_r(5.0f, &sign));
  ASSERT_EQ(1, sign);
}

TEST(math, lgammaf_r_17471883) {
  int sign;

  sign = 0;
  ASSERT_FLOAT_EQ(HUGE_VALF, lgammaf_r(0.0f, &sign));
  ASSERT_EQ(1, sign);
  sign = 0;
  ASSERT_FLOAT_EQ(HUGE_VALF, lgammaf_r(-0.0f, &sign));
  ASSERT_EQ(-1, sign);
}

TEST(math, lgammal_r) {
#ifndef SKIP_LONG_DOUBLE_TESTS
  int sign;
  ASSERT_DOUBLE_EQ(log(24.0L), lgamma_r(5.0L, &sign));
  ASSERT_EQ(1, sign);
#endif
}

TEST(math, lgammal_r_17471883) {
#ifndef SKIP_LONG_DOUBLE_TESTS
  int sign;

  sign = 0;
  ASSERT_DOUBLE_EQ(HUGE_VAL, lgammal_r(0.0L, &sign));
  ASSERT_EQ(1, sign);
  sign = 0;
  ASSERT_DOUBLE_EQ(HUGE_VAL, lgammal_r(-0.0L, &sign));
  ASSERT_EQ(-1, sign);
#endif
}

TEST(math, tgamma) {
  ASSERT_DOUBLE_EQ(24.0, tgamma(5.0));
}

TEST(math, tgammaf) {
  ASSERT_FLOAT_EQ(24.0f, tgammaf(5.0f));
}

TEST(math, tgammal) {
#ifndef SKIP_LONG_DOUBLE_TESTS
  ASSERT_DOUBLE_EQ(24.0L, tgammal(5.0L));
#endif
}

TEST(math, j0) {
  ASSERT_DOUBLE_EQ(1.0, j0(0.0));
  ASSERT_DOUBLE_EQ(0.76519768655796661, j0(1.0));
}

TEST(math, j0f) {
  ASSERT_FLOAT_EQ(1.0f, j0f(0.0f));
  ASSERT_FLOAT_EQ(0.76519769f, j0f(1.0f));
}

TEST(math, j1) {
  ASSERT_DOUBLE_EQ(0.0, j1(0.0));
  ASSERT_DOUBLE_EQ(0.44005058574493355, j1(1.0));
}

TEST(math, j1f) {
  ASSERT_FLOAT_EQ(0.0f, j1f(0.0f));
  ASSERT_FLOAT_EQ(0.44005057f, j1f(1.0f));
}

TEST(math, jn) {
  ASSERT_DOUBLE_EQ(0.0, jn(4, 0.0));
  ASSERT_DOUBLE_EQ(0.0024766389641099553, jn(4, 1.0));
}

TEST(math, jnf) {
  ASSERT_FLOAT_EQ(0.0f, jnf(4, 0.0f));
  ASSERT_FLOAT_EQ(0.0024766389f, jnf(4, 1.0f));
}

TEST(math, y0) {
  ASSERT_DOUBLE_EQ(-HUGE_VAL, y0(0.0));
  ASSERT_DOUBLE_EQ(0.08825696421567697, y0(1.0));
}

TEST(math, y0f) {
  ASSERT_FLOAT_EQ(-HUGE_VALF, y0f(0.0f));
  ASSERT_FLOAT_EQ(0.088256963f, y0f(1.0f));
}

TEST(math, y1) {
  ASSERT_DOUBLE_EQ(-HUGE_VAL, y1(0.0));
  ASSERT_DOUBLE_EQ(-0.78121282130028868, y1(1.0));
}

TEST(math, y1f) {
  ASSERT_FLOAT_EQ(-HUGE_VALF, y1f(0.0f));
  ASSERT_FLOAT_EQ(-0.78121281f, y1f(1.0f));
}

TEST(math, yn) {
  ASSERT_DOUBLE_EQ(-HUGE_VAL, yn(4, 0.0));
  ASSERT_DOUBLE_EQ(-33.278423028972114, yn(4, 1.0));
}

TEST(math, ynf) {
  ASSERT_FLOAT_EQ(-HUGE_VALF, ynf(4, 0.0f));
  ASSERT_FLOAT_EQ(-33.278423f, ynf(4, 1.0f));
}

TEST(math, frexp) {
  int exp;
  double dr = frexp(1024.0, &exp);
  ASSERT_DOUBLE_EQ(1024.0, scalbn(dr, exp));
}

TEST(math, frexpf) {
  int exp;
  float fr = frexpf(1024.0f, &exp);
  ASSERT_FLOAT_EQ(1024.0f, scalbnf(fr, exp));
}

TEST(math, frexpl) {
#ifndef SKIP_LONG_DOUBLE_TESTS
  int exp;
  long double ldr = frexpl(1024.0L, &exp);
  ASSERT_DOUBLE_EQ(1024.0L, scalbnl(ldr, exp));
#endif
}

TEST(math, modf) {
  double di;
  double df = modf(123.75, &di);
  ASSERT_DOUBLE_EQ(123.0, di);
  ASSERT_DOUBLE_EQ(0.75, df);
}

TEST(math, modff) {
  float fi;
  float ff = modff(123.75f, &fi);
  ASSERT_FLOAT_EQ(123.0f, fi);
  ASSERT_FLOAT_EQ(0.75f, ff);
}

TEST(math, modfl) {
#ifndef SKIP_LONG_DOUBLE_TESTS
  long double ldi;
  long double ldf = modfl(123.75L, &ldi);
  ASSERT_DOUBLE_EQ(123.0L, ldi);
  ASSERT_DOUBLE_EQ(0.75L, ldf);
#endif
}

TEST(math, remquo) {
  int q;
  double d = remquo(13.0, 4.0, &q);
  ASSERT_EQ(3, q);
  ASSERT_DOUBLE_EQ(1.0, d);
}

TEST(math, remquof) {
  int q;
  float f = remquof(13.0f, 4.0f, &q);
  ASSERT_EQ(3, q);
  ASSERT_FLOAT_EQ(1.0, f);
}

TEST(math, remquol) {
#ifndef SKIP_LONG_DOUBLE_TESTS
  int q;
  long double ld = remquol(13.0L, 4.0L, &q);
  ASSERT_DOUBLE_EQ(3L, q);
  ASSERT_DOUBLE_EQ(1.0L, ld);
#endif
}

// https://code.google.com/p/android/issues/detail?id=6697
TEST(math, frexpf_public_bug_6697) {
  int exp;
  float fr = frexpf(14.1f, &exp);
  ASSERT_FLOAT_EQ(14.1f, scalbnf(fr, exp));
}

TEST(math, exp2_STRICT_ALIGN_OpenBSD_bug) {
  // OpenBSD/x86's libm had a bug here, but it was already fixed in FreeBSD:
  // http://svnweb.FreeBSD.org/base/head/lib/msun/src/math_private.h?revision=240827&view=markup
  ASSERT_DOUBLE_EQ(5.0, exp2(log2(5)));
  ASSERT_FLOAT_EQ(5.0f, exp2f(log2f(5)));
#ifndef SKIP_LONG_DOUBLE_TESTS
  ASSERT_DOUBLE_EQ(5.0L, exp2l(log2l(5)));
#endif
}

TEST(math, nextafterl_OpenBSD_bug) {
  // OpenBSD/x86's libm had a bug here.
  ASSERT_TRUE(nextafter(1.0, 0.0) - 1.0 < 0.0);
  ASSERT_TRUE(nextafterf(1.0f, 0.0f) - 1.0f < 0.0f);
  ASSERT_TRUE(nextafterl(1.0L, 0.0L) - 1.0L < 0.0L);
}

#include "math_data/acos_intel_data.h"
TEST(math, acos_intel) {
  DoMathDataTest<1>(g_acos_intel_data, acos);
}

#include "math_data/acosf_intel_data.h"
TEST(math, acosf_intel) {
  DoMathDataTest<1>(g_acosf_intel_data, acosf);
}

#include "math_data/acosh_intel_data.h"
TEST(math, acosh_intel) {
  DoMathDataTest<2>(g_acosh_intel_data, acosh);
}

#include "math_data/acoshf_intel_data.h"
TEST(math, acoshf_intel) {
  DoMathDataTest<2>(g_acoshf_intel_data, acoshf);
}

#include "math_data/asin_intel_data.h"
TEST(math, asin_intel) {
  DoMathDataTest<1>(g_asin_intel_data, asin);
}

#include "math_data/asinf_intel_data.h"
TEST(math, asinf_intel) {
  DoMathDataTest<1>(g_asinf_intel_data, asinf);
}

#include "math_data/asinh_intel_data.h"
TEST(math, asinh_intel) {
  DoMathDataTest<2>(g_asinh_intel_data, asinh);
}

#include "math_data/asinhf_intel_data.h"
TEST(math, asinhf_intel) {
  DoMathDataTest<2>(g_asinhf_intel_data, asinhf);
}

#include "math_data/atan2_intel_data.h"
TEST(math, atan2_intel) {
  DoMathDataTest<2>(g_atan2_intel_data, atan2);
}

#include "math_data/atan2f_intel_data.h"
TEST(math, atan2f_intel) {
  DoMathDataTest<2>(g_atan2f_intel_data, atan2f);
}

#include "math_data/atan_intel_data.h"
TEST(math, atan_intel) {
  DoMathDataTest<1>(g_atan_intel_data, atan);
}

#include "math_data/atanf_intel_data.h"
TEST(math, atanf_intel) {
  DoMathDataTest<1>(g_atanf_intel_data, atanf);
}

#include "math_data/atanh_intel_data.h"
TEST(math, atanh_intel) {
  DoMathDataTest<2>(g_atanh_intel_data, atanh);
}

#include "math_data/atanhf_intel_data.h"
TEST(math, atanhf_intel) {
  DoMathDataTest<2>(g_atanhf_intel_data, atanhf);
}

#include "math_data/cbrt_intel_data.h"
TEST(math, cbrt_intel) {
  DoMathDataTest<1>(g_cbrt_intel_data, cbrt);
}

#include "math_data/cbrtf_intel_data.h"
TEST(math, cbrtf_intel) {
  DoMathDataTest<1>(g_cbrtf_intel_data, cbrtf);
}

#include "math_data/ceil_intel_data.h"
TEST(math, ceil_intel) {
  DoMathDataTest<1>(g_ceil_intel_data, ceil);
}

#include "math_data/ceilf_intel_data.h"
TEST(math, ceilf_intel) {
  DoMathDataTest<1>(g_ceilf_intel_data, ceilf);
}

#include "math_data/copysign_intel_data.h"
TEST(math, copysign_intel) {
  DoMathDataTest<1>(g_copysign_intel_data, copysign);
}

#include "math_data/copysignf_intel_data.h"
TEST(math, copysignf_intel) {
  DoMathDataTest<1>(g_copysignf_intel_data, copysignf);
}

#include "math_data/cos_intel_data.h"
TEST(math, cos_intel) {
  DoMathDataTest<1>(g_cos_intel_data, cos);
}

#include "math_data/cosf_intel_data.h"
TEST(math, cosf_intel) {
  DoMathDataTest<1>(g_cosf_intel_data, cosf);
}

#include "math_data/cosh_intel_data.h"
TEST(math, cosh_intel) {
  DoMathDataTest<2>(g_cosh_intel_data, cosh);
}

#include "math_data/coshf_intel_data.h"
TEST(math, coshf_intel) {
  DoMathDataTest<2>(g_coshf_intel_data, coshf);
}

#include "math_data/exp_intel_data.h"
TEST(math, exp_intel) {
  DoMathDataTest<1>(g_exp_intel_data, exp);
}

#include "math_data/expf_intel_data.h"
TEST(math, expf_intel) {
  DoMathDataTest<1>(g_expf_intel_data, expf);
}

#include "math_data/exp2_intel_data.h"
TEST(math, exp2_intel) {
  DoMathDataTest<1>(g_exp2_intel_data, exp2);
}

#include "math_data/exp2f_intel_data.h"
TEST(math, exp2f_intel) {
  DoMathDataTest<1>(g_exp2f_intel_data, exp2f);
}

#include "math_data/expm1_intel_data.h"
TEST(math, expm1_intel) {
  DoMathDataTest<1>(g_expm1_intel_data, expm1);
}

#include "math_data/expm1f_intel_data.h"
TEST(math, expm1f_intel) {
  DoMathDataTest<1>(g_expm1f_intel_data, expm1f);
}

#include "math_data/fabs_intel_data.h"
TEST(math, fabs_intel) {
  DoMathDataTest<1>(g_fabs_intel_data, fabs);
}

#include "math_data/fabsf_intel_data.h"
TEST(math, fabsf_intel) {
  DoMathDataTest<1>(g_fabsf_intel_data, fabsf);
}

#include "math_data/fdim_intel_data.h"
TEST(math, fdim_intel) {
  DoMathDataTest<1>(g_fdim_intel_data, fdim);
}

#include "math_data/fdimf_intel_data.h"
TEST(math, fdimf_intel) {
  DoMathDataTest<1>(g_fdimf_intel_data, fdimf);
}

#include "math_data/floor_intel_data.h"
TEST(math, floor_intel) {
  DoMathDataTest<1>(g_floor_intel_data, floor);
}

#include "math_data/floorf_intel_data.h"
TEST(math, floorf_intel) {
  DoMathDataTest<1>(g_floorf_intel_data, floorf);
}

#include "math_data/fma_intel_data.h"
TEST(math, fma_intel) {
  DoMathDataTest<1>(g_fma_intel_data, fma);
}

#include "math_data/fmaf_intel_data.h"
TEST(math, fmaf_intel) {
  DoMathDataTest<1>(g_fmaf_intel_data, fmaf);
}

#include "math_data/fmax_intel_data.h"
TEST(math, fmax_intel) {
  DoMathDataTest<1>(g_fmax_intel_data, fmax);
}

#include "math_data/fmaxf_intel_data.h"
TEST(math, fmaxf_intel) {
  DoMathDataTest<1>(g_fmaxf_intel_data, fmaxf);
}

#include "math_data/fmin_intel_data.h"
TEST(math, fmin_intel) {
  DoMathDataTest<1>(g_fmin_intel_data, fmin);
}

#include "math_data/fminf_intel_data.h"
TEST(math, fminf_intel) {
  DoMathDataTest<1>(g_fminf_intel_data, fminf);
}

#include "math_data/fmod_intel_data.h"
TEST(math, fmod_intel) {
  DoMathDataTest<1>(g_fmod_intel_data, fmod);
}

#include "math_data/fmodf_intel_data.h"
TEST(math, fmodf_intel) {
  DoMathDataTest<1>(g_fmodf_intel_data, fmodf);
}

#include "math_data/frexp_intel_data.h"
TEST(math, frexp_intel) {
  DoMathDataTest<1>(g_frexp_intel_data, frexp);
}

#include "math_data/frexpf_intel_data.h"
TEST(math, frexpf_intel) {
  DoMathDataTest<1>(g_frexpf_intel_data, frexpf);
}

#include "math_data/hypot_intel_data.h"
TEST(math, hypot_intel) {
  DoMathDataTest<1>(g_hypot_intel_data, hypot);
}

#include "math_data/hypotf_intel_data.h"
TEST(math, hypotf_intel) {
  DoMathDataTest<1>(g_hypotf_intel_data, hypotf);
}

#include "math_data/ilogb_intel_data.h"
TEST(math, ilogb_intel) {
  DoMathDataTest<1>(g_ilogb_intel_data, ilogb);
}

#include "math_data/ilogbf_intel_data.h"
TEST(math, ilogbf_intel) {
  DoMathDataTest<1>(g_ilogbf_intel_data, ilogbf);
}

#include "math_data/ldexp_intel_data.h"
TEST(math, ldexp_intel) {
  DoMathDataTest<1>(g_ldexp_intel_data, ldexp);
}

#include "math_data/ldexpf_intel_data.h"
TEST(math, ldexpf_intel) {
  DoMathDataTest<1>(g_ldexpf_intel_data, ldexpf);
}

#include "math_data/log_intel_data.h"
TEST(math, log_intel) {
  DoMathDataTest<1>(g_log_intel_data, log);
}

#include "math_data/logf_intel_data.h"
TEST(math, logf_intel) {
  DoMathDataTest<1>(g_logf_intel_data, logf);
}

#include "math_data/log10_intel_data.h"
TEST(math, log10_intel) {
  DoMathDataTest<1>(g_log10_intel_data, log10);
}

#include "math_data/log10f_intel_data.h"
TEST(math, log10f_intel) {
  DoMathDataTest<1>(g_log10f_intel_data, log10f);
}

#include "math_data/log1p_intel_data.h"
TEST(math, log1p_intel) {
  DoMathDataTest<1>(g_log1p_intel_data, log1p);
}

#include "math_data/log1pf_intel_data.h"
TEST(math, log1pf_intel) {
  DoMathDataTest<1>(g_log1pf_intel_data, log1pf);
}

#include "math_data/log2_intel_data.h"
TEST(math, log2_intel) {
  DoMathDataTest<1>(g_log2_intel_data, log2);
}

#include "math_data/log2f_intel_data.h"
TEST(math, log2f_intel) {
  DoMathDataTest<1>(g_log2f_intel_data, log2f);
}

#include "math_data/logb_intel_data.h"
TEST(math, logb_intel) {
  DoMathDataTest<1>(g_logb_intel_data, logb);
}

#include "math_data/logbf_intel_data.h"
TEST(math, logbf_intel) {
  DoMathDataTest<1>(g_logbf_intel_data, logbf);
}

#include "math_data/modf_intel_data.h"
TEST(math, modf_intel) {
  DoMathDataTest<1>(g_modf_intel_data, modf);
}

#include "math_data/modff_intel_data.h"
TEST(math, modff_intel) {
  DoMathDataTest<1>(g_modff_intel_data, modff);
}

#include "math_data/nearbyint_intel_data.h"
TEST(math, nearbyint_intel) {
  DoMathDataTest<1>(g_nearbyint_intel_data, nearbyint);
}

#include "math_data/nearbyintf_intel_data.h"
TEST(math, nearbyintf_intel) {
  DoMathDataTest<1>(g_nearbyintf_intel_data, nearbyintf);
}

#include "math_data/nextafter_intel_data.h"
TEST(math, nextafter_intel) {
  DoMathDataTest<1>(g_nextafter_intel_data, nextafter);
}

#include "math_data/nextafterf_intel_data.h"
TEST(math, nextafterf_intel) {
  DoMathDataTest<1>(g_nextafterf_intel_data, nextafterf);
}

#include "math_data/pow_intel_data.h"
TEST(math, pow_intel) {
  DoMathDataTest<1>(g_pow_intel_data, pow);
}

#include "math_data/powf_intel_data.h"
TEST(math, powf_intel) {
  DoMathDataTest<1>(g_powf_intel_data, powf);
}

#include "math_data/remainder_intel_data.h"
TEST(math, remainder_intel) {
  DoMathDataTest<1>(g_remainder_intel_data, remainder);
}

#include "math_data/remainderf_intel_data.h"
TEST(math, remainderf_intel) {
  DoMathDataTest<1>(g_remainderf_intel_data, remainderf);
}

#include "math_data/remquo_intel_data.h"
TEST(math, remquo_intel) {
  DoMathDataTest<1>(g_remquo_intel_data, remquo);
}

#include "math_data/remquof_intel_data.h"
TEST(math, remquof_intel) {
  DoMathDataTest<1>(g_remquof_intel_data, remquof);
}

#include "math_data/rint_intel_data.h"
TEST(math, rint_intel) {
  DoMathDataTest<1>(g_rint_intel_data, rint);
}

#include "math_data/rintf_intel_data.h"
TEST(math, rintf_intel) {
  DoMathDataTest<1>(g_rintf_intel_data, rintf);
}

#include "math_data/round_intel_data.h"
TEST(math, round_intel) {
  DoMathDataTest<1>(g_round_intel_data, round);
}

#include "math_data/roundf_intel_data.h"
TEST(math, roundf_intel) {
  DoMathDataTest<1>(g_roundf_intel_data, roundf);
}

#include "math_data/scalb_intel_data.h"
TEST(math, scalb_intel) {
  DoMathDataTest<1>(g_scalb_intel_data, scalb);
}

#include "math_data/scalbf_intel_data.h"
TEST(math, scalbf_intel) {
  DoMathDataTest<1>(g_scalbf_intel_data, scalbf);
}

#include "math_data/scalbn_intel_data.h"
TEST(math, scalbn_intel) {
  DoMathDataTest<1>(g_scalbn_intel_data, scalbn);
}

#include "math_data/scalbnf_intel_data.h"
TEST(math, scalbnf_intel) {
  DoMathDataTest<1>(g_scalbnf_intel_data, scalbnf);
}

#include "math_data/significand_intel_data.h"
TEST(math, significand_intel) {
  DoMathDataTest<1>(g_significand_intel_data, significand);
}

#include "math_data/significandf_intel_data.h"
TEST(math, significandf_intel) {
  DoMathDataTest<1>(g_significandf_intel_data, significandf);
}

#include "math_data/sin_intel_data.h"
TEST(math, sin_intel) {
  DoMathDataTest<1>(g_sin_intel_data, sin);
}

#include "math_data/sinf_intel_data.h"
TEST(math, sinf_intel) {
  DoMathDataTest<1>(g_sinf_intel_data, sinf);
}

#include "math_data/sinh_intel_data.h"
TEST(math, sinh_intel) {
  DoMathDataTest<2>(g_sinh_intel_data, sinh);
}

#include "math_data/sinhf_intel_data.h"
TEST(math, sinhf_intel) {
  DoMathDataTest<2>(g_sinhf_intel_data, sinhf);
}

#include "math_data/sincos_intel_data.h"
TEST(math, sincos_intel) {
  DoMathDataTest<1>(g_sincos_intel_data, sincos);
}

#include "math_data/sincosf_intel_data.h"
TEST(math, sincosf_intel) {
  DoMathDataTest<1>(g_sincosf_intel_data, sincosf);
}

#include "math_data/sqrt_intel_data.h"
TEST(math, sqrt_intel) {
  DoMathDataTest<1>(g_sqrt_intel_data, sqrt);
}

#include "math_data/sqrtf_intel_data.h"
TEST(math, sqrtf_intel) {
  DoMathDataTest<1>(g_sqrtf_intel_data, sqrtf);
}

#include "math_data/tan_intel_data.h"
TEST(math, tan_intel) {
  DoMathDataTest<1>(g_tan_intel_data, tan);
}

#include "math_data/tanf_intel_data.h"
TEST(math, tanf_intel) {
  DoMathDataTest<1>(g_tanf_intel_data, tanf);
}

#include "math_data/tanh_intel_data.h"
TEST(math, tanh_intel) {
  DoMathDataTest<2>(g_tanh_intel_data, tanh);
}

#include "math_data/tanhf_intel_data.h"
TEST(math, tanhf_intel) {
  DoMathDataTest<2>(g_tanhf_intel_data, tanhf);
}

#include "math_data/trunc_intel_data.h"
TEST(math, trunc_intel) {
  DoMathDataTest<1>(g_trunc_intel_data, trunc);
}

#include "math_data/truncf_intel_data.h"
TEST(math, truncf_intel) {
  DoMathDataTest<1>(g_truncf_intel_data, truncf);
}
