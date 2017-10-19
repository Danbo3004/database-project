/*************************************************
 * file: test_harness.cc
 * author: keith schwarz (htiek@cs.stanford.edu)
 *
 * file containing several test cases that can be
 * used to verify the correctness of the kdtree
 * implementation.  while i've tried to exercise
 * as many aspects of the kdtree as possible here,
 * there are almost certainly cases i haven't
 * thought of.  you should make sure to do your own
 * testing in addition to ensuring that the test
 * cases here pass.
 */
#include <cstdarg>
#include <iomanip>
#include <iostream>
#include <set>
#include <sstream>
#include <string>
#include <vector>
#include "kdtree.h"

/* these flags control which tests will be run.  initially, only the
 * basic test will be executed.  as you complete more and more parts
 * of the implementation, you will want to turn more and more of these
 * flags on.
 */
#define BasicKDTreeTestEnabled         1  // Step one checks
#define ModerateKDTreeTestEnabled      1
#define HarderKDTreeTestEnabled        1
#define EdgeCaseKDTreeTestEnabled      1
#define MutatingKDTreeTestEnabled      1
#define ThrowingKDTreeTestEnabled      1
#define ConstKDTreeTestEnabled         1

#define NearestNeighborTestEnabled     1  // Step two checks
#define MoreNearestNeighborTestEnabled 1

#define BasicCopyTestEnabled           1  // Step three checks
#define ModerateCopyTestEnabled        1

enum class TestResult { kPass, kFail, kTestDisabled };

struct Test {
  std::string test_name;
  TestResult (*test_function)();
  TestResult test_result;
};

/* A utility function to construct a Point from a range of iterators. */
template <size_t N, typename IteratorType>
Point<N> PointFromRange(IteratorType begin, IteratorType end) {
  Point<N> result;
  std::copy(begin, end, result.begin());
  return result;
}

/* Utility functions to create 1-, 2-, 3-, or 4-Points from values. */
Point<1> MakePoint(double x) {
  Point<1> result;
  result[0] = x;
  return result;
}
Point<2> MakePoint(double x, double y) {
  Point<2> result;
  result[0] = x;
  result[1] = y;
  return result;
}
Point<3> MakePoint(double x, double y, double z) {
  Point<3> result;
  result[0] = x;
  result[1] = y;
  result[2] = z;
  return result;
}
Point<4> MakePoint(double x, double y, double z, double w) {
  Point<4> result;
  result[0] = x;
  result[1] = y;
  result[2] = z;
  result[3] = w;
  return result;
}

/* This function is what the test suite uses to ensure that the KDTree works
 * correctly.  It takes as parameters an expression and description, along
 * with a file and line number, then checks whether the condition is true.
 * If so, it prints that the test passed.  Otherwise, it reports that the
 * test fails and points the caller to the proper file and line.
 */
bool DoCheckCondition(bool expr, const std::string& rationale,
                      const std::string& file, int line) {
  /* It worked!  Congrats. */
  if (expr) {
    std::cout << "PASS: " << rationale << std::endl;
    return true;
  }

  /* Uh oh!  The test failed! */
  std::cout << "FAIL: " << rationale << std::endl;
  std::cout << "  Error at " << file << ", line " << line << std::endl;
  return false;
}

/* Reports that an unexpected error occurred that caused a test to fail. */
void FailTest(const std::exception& e) {
  std::cerr << "TEST FAILED: Unexpected exception: " << e.what() << std::endl;
}

/* This macro takes in an expression and a string, then invokes
 * DoCheckCondition passing in the arguments along with the file
 * and line number on which the macro was called.  This makes it
 * easier to track down the source of bugs if a test case should
 * fail.
 */
#define CheckCondition(expr, rationale) \
  DoCheckCondition(expr, rationale, __FILE__, __LINE__)

/* Utility function to delimit the start and end of test cases. */
void PrintBanner(const std::string& header) {
  std::cout << std::endl << header << std::endl;
  std::cout << std::setw(40) << std::setfill('-') << "" << std::setfill(' ')
            << std::endl;
}

/* Utility function to signal that a test isn't begin run. */
void TestDisabled(const std::string& header) {
  std::cout << "== Test " << header << " NOT RUN ==" << std::endl << std::endl;
}

/* Utility function to signal the end of a test. */
void EndTest() { std::cout << "== end of test ==" << std::endl << std::endl; }

/* Basic test: Can we build a small tree and look up the elements it contains?
 */
TestResult BasicKDTreeTest() try {
#if BasicKDTreeTestEnabled
  bool pass = true;

  PrintBanner("Basic KDTree Test");

  /* Construct the KDTree. */
  KDTree<3, size_t> kd;
  pass &= CheckCondition(true, "KDTree construction completed.");

  /* Check basic properties of the KDTree. */
  pass &= CheckCondition(kd.dimension() == 3, "Dimension is three.");
  pass &= CheckCondition(kd.size() == 0, "New KD tree has no elements.");
  pass &= CheckCondition(kd.empty(), "New KD tree is empty.");

  /* Add some elements. */
  const double dataPoints[3][3] = {{1, 0, 0}, {0, 1, 0}, {0, 0, 1}};
  for (size_t i = 0; i < 3; ++i)
    kd.insert(PointFromRange<3>(dataPoints[i], dataPoints[i] + 3), i);

  /* Check basic properties again. */
  pass &= CheckCondition(kd.size() == 3,
                         "After adding three elements, KDTree has size 3.");
  pass &= CheckCondition(!kd.empty(),
                         "After adding three elements, KDTree is not empty.");

  /* Make sure that the elements we built the tree out of are still there. */
  pass &= CheckCondition(
      kd.contains(PointFromRange<3>(dataPoints[0], dataPoints[0] + 3)),
      "New KD tree has element zero.");
  pass &= CheckCondition(
      kd.contains(PointFromRange<3>(dataPoints[1], dataPoints[1] + 3)),
      "New KD tree has element one.");
  pass &= CheckCondition(
      kd.contains(PointFromRange<3>(dataPoints[2], dataPoints[2] + 3)),
      "New KD tree has element two.");

  /* Make sure that the values of these points are correct. */
  for (size_t i = 0; i < 3; ++i)
    pass &= CheckCondition(
        kd.at(PointFromRange<3>(dataPoints[i], dataPoints[i] + 3)) == i,
        "New KD tree has correct values.");

  EndTest();
  return pass ? TestResult::kPass : TestResult::kFail;
#else
  TestDisabled("BasicKDTreeTest");
  return TestResult::kTestDisabled;
#endif
} catch (const std::exception& e) {
  FailTest(e);
  return TestResult::kFail;
}

/* A trickier test that involves looking up nonexistent elements and working
 * with a
 * larger data set.
 */
TestResult ModerateKDTreeTest() try {
#if ModerateKDTreeTestEnabled
  bool pass = true;

  PrintBanner("Moderate KDTree Test");

  /* Build the data set. */
  const double dataPoints[][4] = {
      {0, 0, 0, 0}, {0, 0, 0, 1}, {0, 0, 1, 0}, {0, 0, 1, 1},
      {0, 1, 0, 0}, {0, 1, 0, 1}, {0, 1, 1, 0}, {0, 1, 1, 1},
      {1, 0, 0, 0}, {1, 0, 0, 1}, {1, 0, 1, 0}, {1, 0, 1, 1},
      {1, 1, 0, 0}, {1, 1, 0, 1}, {1, 1, 1, 0}, {1, 1, 1, 1},
  };

  KDTree<4, size_t> kd;
  for (size_t i = 0; i < 16; ++i)
    kd.insert(PointFromRange<4>(dataPoints[i], dataPoints[i] + 4), i);

  /* Check that basic properties hold. */
  pass &= CheckCondition(kd.dimension() == 4, "Dimension is four.");
  pass &= CheckCondition(kd.size() == 16,
                         "New KD tree has the right number of elements.");
  pass &= CheckCondition(!kd.empty(), "New KD tree is nonempty.");

  /* Make sure that the values of these points are correct. */
  for (size_t i = 0; i < 16; ++i)
    pass &= CheckCondition(
        kd.at(PointFromRange<4>(dataPoints[i], dataPoints[i] + 4)) == i,
        "New KD tree has correct values.");

  /* Try looking up some nonexistent elements and see what happens. */
  pass &= CheckCondition(!kd.contains(MakePoint(1.0, 1.0, 1.0, 0.5)),
                         "Nonexistent elements aren't in the tree.");
  pass &= CheckCondition(!kd.contains(MakePoint(0.0, 0.0, 0.0, -0.5)),
                         "Nonexistent elements aren't in the tree.");

  EndTest();
  return pass ? TestResult::kPass : TestResult::kFail;
#else
  TestDisabled("ModerateKDTreeTest");
  return TestResult::kTestDisabled;
#endif
} catch (const std::exception& e) {
  FailTest(e);
  return TestResult::kFail;
}

/* This test still uses just the basic functionality, but it hammers it a bit
 * more
 * by checking for strange edge cases like duplicated elements.
 */
TestResult HarderKDTreeTest() try {
#if HarderKDTreeTestEnabled
  bool pass = true;

  PrintBanner("Harder KDTree Test");

  /* Build the data set. */
  const double dataPoints[][4] = {
      {0, 0, 0, 0}, {0, 1, 0, 1}, {0, 0, 0, 0},  // Duplicate!
      {0, 1, 0, 1},                              // Duplicate!
      {0, 1, 1, 0}, {1, 0, 1, 0},
  };
  KDTree<4, size_t> kd;
  for (size_t i = 0; i < 6; ++i)
    kd.insert(PointFromRange<4>(dataPoints[i], dataPoints[i] + 4), i);

  /* Check basic properties. */
  pass &= CheckCondition(kd.dimension() == 4, "Dimension is four.");
  pass &= CheckCondition(
      kd.size() == 4,
      "New KD tree has the right number of elements (no duplicates).");
  pass &= CheckCondition(!kd.empty(), "New KD tree is nonempty.");

  /* Make sure that elements are still there, without checking values. */
  for (size_t i = 0; i < 6; ++i)
    pass &= CheckCondition(
        kd.contains(PointFromRange<4>(dataPoints[i], dataPoints[i] + 4)),
        "New KD tree has original elems.");

  /* Check that the elements have the correct keys.  Elements [2, 6) should have
   * the
   * correct keys, but elements 0 and 1 will have the keys of elements 2 and 3
   * because
   * they were overwritten.
   */
  for (size_t i = 2; i < 6; ++i)
    pass &= CheckCondition(
        kd.at(PointFromRange<4>(dataPoints[i], dataPoints[i] + 4)) == i,
        "KD tree has correct labels.");
  for (size_t i = 0; i < 2; ++i)
    pass &= CheckCondition(
        kd.at(PointFromRange<4>(dataPoints[i], dataPoints[i] + 4)) == i + 2,
        "insert overwrites old labels.");

  EndTest();
  return pass ? TestResult::kPass : TestResult::kFail;
#else
  TestDisabled("HarderKDTreeTest");
  return TestResult::kTestDisabled;
#endif
} catch (const std::exception& e) {
  FailTest(e);
  return TestResult::kFail;
}

/* This test builds a KDTree where the data has the same values everywhere
 * except
 * along one coordinate axis.  If you are failing this test case, make sure that
 * your implementation of find() descends into the left subtree only if the
 * current
 * coordinate is _strictly less_ than the partition point's coordinate.
 */
TestResult EdgeCaseKDTreeTest() try {
#if EdgeCaseKDTreeTestEnabled
  bool pass = true;

  PrintBanner("Edge Case KDTree Test");

  /* Build the data set. */
  const double dataPoints[8][3] = {
      {0, 0, 0}, {0, 1, 0}, {0, 2, 0}, {0, 3, 0},
      {0, 4, 0}, {0, 5, 0}, {0, 6, 0}, {0, 7, 0},
  };

  /* Add points. */
  KDTree<3, size_t> kd;
  for (size_t i = 0; i < 8; ++i)
    kd.insert(PointFromRange<3>(dataPoints[i], dataPoints[i] + 3), i);

  /* Basic checks. */
  pass &= CheckCondition(kd.dimension() == 3, "Dimension is three.");
  pass &= CheckCondition(kd.size() == 8,
                         "New KD tree has the right number of elements.");
  pass &= CheckCondition(!kd.empty(), "New KD tree is nonempty.");

  /* Make sure everything can be found. */
  for (size_t i = 0; i < kd.size(); ++i)
    pass &= CheckCondition(
        kd.contains(PointFromRange<3>(dataPoints[i], dataPoints[i] + 3)),
        "Lookup succeeded.");

  EndTest();
  return pass ? TestResult::kPass : TestResult::kFail;
#else
  TestDisabled("EdgeCaseTreeTest");
  return TestResult::kTestDisabled;
#endif
} catch (const std::exception& e) {
  FailTest(e);
  return TestResult::kFail;
}

/* This test actively mutates the elements of the KDTree using
 * operator[].  If you are failing this test, check to make sure
 * that your implementation of operator[] correctly allows for
 * mutation and that it inserts elements if they don't already
 * exist.
 */
TestResult MutatingKDTreeTest() try {
#if MutatingKDTreeTestEnabled
  bool pass = true;

  PrintBanner("Mutating KDTree Test");

  /* Build the data set. */
  const double dataPoints[8][3] = {
      {0, 0, 0}, {0, 0, 1}, {0, 1, 0}, {0, 1, 1},
      {1, 0, 0}, {1, 0, 1}, {1, 1, 0}, {1, 1, 1},
  };

  /* Add points using []. */
  KDTree<3, size_t> kd;
  for (size_t i = 0; i < 8; ++i)
    kd[PointFromRange<3>(dataPoints[i], dataPoints[i] + 3)] = i;

  /* Basic checks. */
  pass &= CheckCondition(kd.dimension() == 3, "Dimension is three.");
  pass &= CheckCondition(kd.size() == 8,
                         "New KD tree has the right number of elements.");
  pass &= CheckCondition(!kd.empty(), "New KD tree is nonempty.");

  /* Make sure everything can be found. */
  for (size_t i = 0; i < kd.size(); ++i)
    pass &= CheckCondition(
        kd.contains(PointFromRange<3>(dataPoints[i], dataPoints[i] + 3)),
        "Lookup succeeded.");

  /* Change every other element to have key 0. */
  for (size_t i = 0; i < 8; i += 2)
    kd[PointFromRange<3>(dataPoints[i], dataPoints[i] + 3)] = 0;

  /* Check that the keys are right. */
  for (size_t i = 1; i < 8; i += 2)
    pass &= CheckCondition(
        kd[PointFromRange<3>(dataPoints[i], dataPoints[i] + 3)] == i,
        "Keys are correct for odd elements.");

  /* Check that the keys are right. */
  for (size_t i = 0; i < 8; i += 2)
    pass &= CheckCondition(
        kd[PointFromRange<3>(dataPoints[i], dataPoints[i] + 3)] == 0,
        "Keys are correct for even elements.");

  EndTest();
  return pass ? TestResult::kPass : TestResult::kFail;
#else
  TestDisabled("MutatingKDTreeTest");
  return TestResult::kTestDisabled;
#endif
} catch (const std::exception& e) {
  FailTest(e);
  return TestResult::kFail;
}

/* This test checks that the at() operators correctly throw
 * exceptions when elements are not found.  This is not an
 * exhaustive test, and you should be sure to confirm on your
 * own that everything works correctly.
 */
TestResult ThrowingKDTreeTest() try {
#if ThrowingKDTreeTestEnabled
  bool pass = true;

  PrintBanner("Throwing KDTree Test");

  {
    /* Create a non-const, empty KDTree and look things up. */
    KDTree<3, size_t> empty;

    bool didThrow = false;
    try {
      empty.at(MakePoint(0, 0, 0));
    } catch (const std::out_of_range&) {
      didThrow = true;
    }

    pass &= CheckCondition(didThrow,
                           "Exception generated during non-const lookup.");
  }

  {
    /* Create a const, empty KDTree and look things up. */
    KDTree<3, size_t> empty;

    bool didThrow = false;
    try {
      empty.at(MakePoint(0, 0, 0));
    } catch (const std::out_of_range&) {
      didThrow = true;
    }

    pass &=
        CheckCondition(didThrow, "Exception generated during const lookup.");
  }

  EndTest();
  return pass ? TestResult::kPass : TestResult::kFail;
#else
  TestDisabled("ThrowingKDTreeTest");
  return TestResult::kTestDisabled;
#endif
} catch (const std::exception& e) {
  FailTest(e);
  return TestResult::kFail;
}

/* A basic test that creates a const KDTree and a non-const KDTree to ensure
 * the class still compiles properly. It also tests the the const version of
 * at is working correctly on the basic KDTree tests.
 */
TestResult ConstKDTreeTest() try {
#if ConstKDTreeTestEnabled
  bool pass = true;

  PrintBanner("Const KDTree Test");

  /* Build the data set. */
  const double dataPoints[][4] = {
      {0, 0, 0, 0}, {0, 0, 0, 1}, {0, 0, 1, 0}, {0, 0, 1, 1}};

  KDTree<4, size_t> kd;
  for (size_t i = 0; i < 4; ++i)
    kd.insert(PointFromRange<4>(dataPoints[i], dataPoints[i] + 4), i);

  /* Check that the code compiles for the non-const version. */
  kd.dimension();
  kd.size();
  kd.empty();
  kd.at(PointFromRange<4>(dataPoints[0], dataPoints[0] + 4)) = 100;

  const KDTree<4, size_t>& const_kd = kd;

  /* Check that the code compiles for the const version. */
  const_kd.dimension();
  const_kd.size();
  const_kd.empty();
  const_kd.at(PointFromRange<4>(dataPoints[0], dataPoints[0] + 4));

  pass &= CheckCondition(true, "Const code compiles.");

  /* Run the basic KD Tree tests using a const KD Tree. */
  pass &= CheckCondition(
      const_kd.contains(PointFromRange<4>(dataPoints[0], dataPoints[0] + 4)),
      "Const KD tree has element zero.");
  pass &= CheckCondition(
      const_kd.contains(PointFromRange<4>(dataPoints[1], dataPoints[1] + 4)),
      "Const KD tree has element one.");
  pass &= CheckCondition(
      const_kd.contains(PointFromRange<4>(dataPoints[2], dataPoints[2] + 4)),
      "Const KD tree has element two.");
  pass &= CheckCondition(
      const_kd.contains(PointFromRange<4>(dataPoints[3], dataPoints[3] + 4)),
      "Const KD tree has element three.");

  /* Make sure that the values of these points are correct. */
  pass &= CheckCondition(
      const_kd.at(PointFromRange<4>(dataPoints[0], dataPoints[0] + 4)) == 100,
      "Const KD tree has correct values.");
  for (size_t i = 1; i < 4; ++i)
    pass &= CheckCondition(
        const_kd.at(PointFromRange<4>(dataPoints[i], dataPoints[i] + 4)) == i,
        "Const KD tree has correct values.");

  EndTest();
  return pass ? TestResult::kPass : TestResult::kFail;
#else
  TestDisabled("ConstKDTreeTest");
  return TestResult::kTestDisabled;
#endif
} catch (const std::exception& e) {
  std::cout << "Note: KD tree lookup failed, but const code compiles."
            << std::endl;
  FailTest(e);
  return TestResult::kFail;
}

/* This is a simple test case that checks whether 1-NN lookups work
 * correctly.  If you're failing this test case, you probably need
 * to check that your implementation of kNNValue works
 * correctly.
 */
TestResult NearestNeighborTest() try {
#if NearestNeighborTestEnabled
  bool pass = true;

  PrintBanner("Nearest Neighbor Test");

  /* Build two data sets - a set of tree points and a set of test points. */
  const double dataPoints[][4] = {
      {0, 0, 0, 0},  // 0
      {0, 0, 0, 1},  // 1
      {0, 0, 1, 0},  // 2
      {0, 0, 1, 1},  // 3
      {0, 1, 0, 0},  // 4
      {0, 1, 0, 1},  // 5
      {0, 1, 1, 0},  // 6
      {0, 1, 1, 1},  // 7
      {1, 0, 0, 0},  // 8
      {1, 0, 0, 1},  // 9
      {1, 0, 1, 0},  // 10
      {1, 0, 1, 1},  // 11
      {1, 1, 0, 0},  // 12
      {1, 1, 0, 1},  // 13
      {1, 1, 1, 0},  // 14
      {1, 1, 1, 1},  // 15
  };

  /* Each test point should be right next to the corresponding point in the
   * tree point set.
   */
  const double testPoints[][4] = {
      {0, 0, 0, 0},           // 0
      {0, 0, 0, 0.7},         // 1
      {0, 0, 0.9, 0},         // 2
      {0, 0, 0.6, 0.6},       // 3
      {0, 0.9, 0, 0},         // 4
      {0, 0.8, 0, 0.7},       // 5
      {0, 0.6, 0.7, -0.1},    // 6
      {-0.4, 0.7, 0.7, 0.7},  // 7
      {1, 0, 0, 0},           // 8
      {1, 0, 0, 1},           // 9
      {1, 0, 1, 0},           // 10
      {1, 0, 1, 1},           // 11
      {1, 1, 0, 0},           // 12
      {1, 1, 0, 1},           // 13
      {1, 1, 1, 0},           // 14
      {1, 1, 1, 1},           // 15
  };

  /* Build up a data set from the first set of points. */
  KDTree<4, size_t> kd;
  for (size_t i = 0; i < 16; ++i)
    kd.insert(PointFromRange<4>(dataPoints[i], dataPoints[i] + 4), i);

  /* Check that calling nearest neighbor on a point in the tree yields that
  point. */
  for (size_t i = 0; i < 16; ++i)
    pass &= CheckCondition(
        kd.kNNValue(PointFromRange<4>(dataPoints[i], dataPoints[i] + 4), 1) ==
            i,
        "Nearest neighbor of element is that element.");

  /* Check that calling nearest neighbor on the test points yields the correct
  tree points. */
  for (size_t i = 0; i < 16; ++i)
    pass &= CheckCondition(
        kd.kNNValue(PointFromRange<4>(testPoints[i], testPoints[i] + 4), 1) ==
            i,
        "Test point yielded correct nearest neighbor.");

  EndTest();
  return pass ? TestResult::kPass : TestResult::kFail;
#else
  TestDisabled("NearestNeighborTest");
  return TestResult::kTestDisabled;
#endif
} catch (const std::exception& e) {
  FailTest(e);
  return TestResult::kFail;
}

/* This test verifies that k-NN works for k > 1.  If you're failing this test
 * case, then you probably
 * need to check that you're computing the majority element correctly.
 */
TestResult MoreNearestNeighborTest() try {
#if MoreNearestNeighborTestEnabled
  bool pass = true;

  PrintBanner("More Nearest Neighbor Tests");

  /* We're going to build a 3x3 lattice that looks like this:
   *
   * a b a
   * b a b
   * b b b
   *
   * This will give us some fun checking whether k-NN works
   * correctly.
   */
  std::vector<std::pair<Point<2>, char> > values;
  values.push_back(std::make_pair(MakePoint(0.0, 0.0), 'a'));
  values.push_back(std::make_pair(MakePoint(0.0, 0.5), 'b'));
  values.push_back(std::make_pair(MakePoint(0.0, 1.0), 'a'));
  values.push_back(std::make_pair(MakePoint(0.5, 0.0), 'b'));
  values.push_back(std::make_pair(MakePoint(0.5, 0.5), 'a'));
  values.push_back(std::make_pair(MakePoint(0.5, 1.0), 'b'));
  values.push_back(std::make_pair(MakePoint(1.0, 0.0), 'b'));
  values.push_back(std::make_pair(MakePoint(1.0, 0.5), 'b'));
  values.push_back(std::make_pair(MakePoint(1.0, 1.0), 'b'));

  /* Build the tree by converting the list to a tree. */
  KDTree<2, char> kd;
  for (size_t i = 0; i < values.size(); ++i)
    kd[values[i].first] = values[i].second;

  /* Look for the the four points closes to the top-middle.  This should give
  us 'a' even though
   * the point itself is 'b'.
   */
  pass &= CheckCondition(kd.kNNValue(MakePoint(0.0, 0.5), 4) == 'a',
                         "Nearest neighbors are correct.");

  /* Check for the nine points closest to the center.  This should give us 'b'
  even though the
   * point itself is 'a'
   */
  pass &= CheckCondition(kd.kNNValue(MakePoint(0.0, 0.5), 9) == 'b',
                         "Nearest neighbors are correct.");

  /* Check for the five points closest to the center.  This should give us 'b'
  even though the
   * point itself is 'a'
   */
  pass &= CheckCondition(kd.kNNValue(MakePoint(0.5, 0.5), 5) == 'b',
                         "Nearest neighbors are correct.");

  /* Look in the center of the square in the bottom-right corner.  All the
  points are 'b' and
   * so we'd better get 'b' back!
   */
  pass &= CheckCondition(kd.kNNValue(MakePoint(0.75, 0.75), 4) == 'b',
                         "Nearest neighbors are correct.");

  /* Choose a point way out of the box and get the 9-NN value.  Since there
  are only 9 data points,
   * this should be equivalent to asking "what's the most common element?"
   (The answer is 'b')
   */
  pass &=
      CheckCondition(kd.kNNValue(MakePoint(+10.0, +10.0), 9) == 'b',
                     "Nearest neighbor at distant point is majority element.");

  /* Similar check but for negative numbers. */
  pass &= CheckCondition(kd.kNNValue(MakePoint(-10.0, -10.0), 9) == 'b',
                         "No problems with negative values.");

  /* Try getting the 25 nearest neighbors to a point.  There are only nine
  points, but this should
   * still work correctly.
   */
  pass &= CheckCondition(
      kd.kNNValue(MakePoint(-10.0, -10.0), 25) == 'b',
      "No problems with looking up more neighbors than elements.");

  EndTest();
  return pass ? TestResult::kPass : TestResult::kFail;
#else
  TestDisabled("MoreNearestNeighborTest");
  return TestResult::kTestDisabled;
#endif
} catch (const std::exception& e) {
  FailTest(e);
  return TestResult::kFail;
}

/* Tests basic behavior of the copy constructor and assignment operator. */
TestResult BasicCopyTest() try {
#if BasicCopyTestEnabled
  bool pass = true;

  PrintBanner("Basic Copy Test");

  /* For simplicity, we'll use one-dimensional KDTrees in this step. */
  KDTree<1, size_t> one;
  for (size_t i = 0; i < 10; ++i)
    one[MakePoint(2 * i)] = i;  // Load with 0, 2, 4, ..., 18

  {
    /* Create a clone of one and confirm that everything copied correctly.
     * This uses the copy constructor.
     */
    KDTree<1, size_t> clone = one;

    /* Basic checks. */
    pass &=
        CheckCondition(one.size() == clone.size(),
                       "Copy has the same number of elements as the original.");
    pass &= CheckCondition(one.empty() == clone.empty(),
                           "Copy and original agree on emptiness.");
    pass &= CheckCondition(one.dimension() == clone.dimension(),
                           "Copy and original agree on dimension.");

    /* Check that everything in one is there. */
    for (size_t i = 0; i < 10; ++i)
      pass &= CheckCondition(clone.at(MakePoint(2 * i)) == i,
                             "Element from original present in copy.");

    /* Check that nothing else is. */
    for (size_t i = 0; i < 10; ++i)
      pass &= CheckCondition(!clone.contains(MakePoint(2 * i + 1)),
                             "Other elements not present in copy.");
  }

  {
    /* Create a clone of one and confirm that everything copied correctly.
     * This uses the assignment operator.
     */
    KDTree<1, size_t> clone;
    clone = one;

    /* Basic checks. */
    pass &=
        CheckCondition(one.size() == clone.size(),
                       "Copy has the same number of elements as the original.");
    pass &= CheckCondition(one.empty() == clone.empty(),
                           "Copy and original agree on emptiness.");
    pass &= CheckCondition(one.dimension() == clone.dimension(),
                           "Copy and original agree on dimension.");

    /* Check that everything in one is there. */
    for (size_t i = 0; i < 10; ++i)
      pass &= CheckCondition(clone.at(MakePoint(2 * i)) == i,
                             "Element from original present in copy.");

    /* Check that nothing else is. */
    for (size_t i = 0; i < 10; ++i)
      pass &= CheckCondition(!clone.contains(MakePoint(2 * i + 1)),
                             "Other elements not present in copy.");
  }

  EndTest();
  return pass ? TestResult::kPass : TestResult::kFail;
#else
  TestDisabled("BasicCopyTest");
  return TestResult::kTestDisabled;
#endif
} catch (const std::exception& e) {
  FailTest(e);
  return TestResult::kFail;
}

/* A more merciless test of copy behavior.. */
TestResult ModerateCopyTest() try {
#if ModerateCopyTestEnabled
  bool pass = true;

  PrintBanner("Moderate Copy Test");

  /* For simplicity, we'll use one-dimensional KDTrees in this step. */
  KDTree<1, size_t> one;
  for (size_t i = 0; i < 10; ++i)
    one[MakePoint(2 * i)] = i;  // Load with 0, 2, 4, ..., 18

  {
    /* Create a clone of one and confirm that everything copied correctly.
     * This uses the copy constructor.
     */
    KDTree<1, size_t> clone = one;

    /* Add odd numbers to the clone. */
    for (size_t i = 0; i < 10; ++i) clone[MakePoint(2 * i + 1)] = i;

    /* Confirm that they didn't appear in one. */
    pass &= CheckCondition(one.size() == 10,
                           "Adding to clone change original size.");
    for (size_t i = 0; i < 10; ++i)
      pass &= CheckCondition(!one.contains(MakePoint(2 * i + 1)),
                             "Modifying copy doesn't modify original.");
  }

  /* Check the integrity of the original out here as well to see that the dtor
   * didn't hose things. */
  pass &=
      CheckCondition(one.size() == 10, "After dtor, original is still good.");
  for (size_t i = 0; i < 10; ++i) {
    pass &= CheckCondition(!one.contains(MakePoint(2 * i + 1)),
                           "After dtor, missing elements still missing.");
    pass &= CheckCondition(one[MakePoint(2 * i)] == i,
                           "After dtor, original elements are still there.");
  }

  {
    /* Create a clone of one and confirm that everything copied correctly.
     * This uses the assignment operator.
     */
    KDTree<1, size_t> clone;
    clone = one;

    /* Do awful, awful things to the copy. */
    clone = clone = (clone = clone);
    (clone = one) = clone;
    clone = clone = clone = clone = clone;
  }

  EndTest();
  return pass ? TestResult::kPass : TestResult::kFail;
#else
  TestDisabled("ModerateCopyTest");
  return TestResult::kTestDisabled;
#endif
} catch (const std::exception& e) {
  FailTest(e);
  return TestResult::kFail;
}

/* Main entry point simply runs all the tests.  Note that these functions might
 * be no-ops
 * if they are disabled by the configuration settings at the top of the program.
 */
int main() {
  Test tests[] = {
      /* Step Two Tests */
      {"BasicKDTreeTest", BasicKDTreeTest},
      {"ModerateKDTreeTest", ModerateKDTreeTest},
      {"HarderKDTreeTest", HarderKDTreeTest},
      {"EdgeCaseKDTreeTest", EdgeCaseKDTreeTest},
      {"MutatingKDTreeTest", MutatingKDTreeTest},
      {"ThrowingKDTreeTest", ThrowingKDTreeTest},
      {"ConstKDTreeTest", ConstKDTreeTest},
      /* Step Three Tests */
      {"NearestNeighborTest", NearestNeighborTest},
      {"MoreNearestNeighborTest", MoreNearestNeighborTest},
      /* Step Four Tests */
      {"BasicCopyTest", BasicCopyTest},
      {"ModerateCopyTest", ModerateCopyTest},
  };

  int test_total = sizeof(tests) / sizeof(tests[0]);
  int test_passed = 0;
  for (auto& test : tests) {
    test.test_result = test.test_function();
    if (test.test_result == TestResult::kPass) {
      test_passed++;
    }
  }

  PrintBanner("Test Result");

  for (auto& test : tests) {
    if (test.test_result == TestResult::kPass)
      std::cout << "pass " << test.test_name << std::endl;
    else if (test.test_result == TestResult::kFail)
      std::cout << "FAIL " << test.test_name << std::endl;
    else
      std::cout << "???? " << test.test_name << std::endl;
  }

  if (test_passed == test_total)
    std::cout << "All " << test_total << " tests passed." << std::endl;
  else
    std::cout << test_passed << " of " << test_total << " tests passed."
              << std::endl;

  return 0;
}
