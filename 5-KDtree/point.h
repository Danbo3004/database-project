/**
 * File: point.h
 * -------------
 * A class representing a point in N-dimensional space. Unlike the other class
 * templates you've seen before, Point is parameterized over an integer rather
 * than a type. This allows the compiler to verify that the type is being used
 * correctly.
 */
#ifndef POINT_H_
#define POINT_H_

#include <algorithm>
#include <cmath>

template <int n>
class Point {
 public:
    // Type: iterator
    // Type: const_iterator
    // ------------------------------------------------------------------------
    // Types representing iterators that can traverse and optionally modify the
    // elements of the Point.
  using iterator = double*;
  using const_iterator = const double*;

    // size_t size() const;
    // Usage: for (size_t i = 0; i < myPoint.size(); ++i)
    // ------------------------------------------------------------------------
    // Returns N, the dimension of the point.
  int size() const;

    // double& operator[](size_t index);
    // double operator[](size_t index) const;
    // Usage: myPoint[3] = 137;
    // ------------------------------------------------------------------------
    // Queries or retrieves the value of the point at a particular point. The
    // index is assumed to be in-range.
  double& operator[](int index);
  double operator[](int index) const;

    // iterator begin();
    // iterator end();
    // const_iterator begin() const;
    // const_iterator end() const;
    // Usage: for (Point<3>::iterator itr = myPoint.begin(); itr != myPoint.end(); ++itr)
    // ------------------------------------------------------------------------
    // Returns iterators delineating the full range of elements in the Point.
  iterator begin();
  iterator end();

  const_iterator begin() const;
  const_iterator end() const;

 private:
  double coordinates_[n];
};

// double Distance(const Point<N>& one, const Point<N>& two);
// Usage: double d = Distance(one, two);
// ----------------------------------------------------------------------------
// Returns the Euclidean distance between two points.
template <int n>
double distance(const Point<n>& one, const Point<n>& two);

// bool operator==(const Point<N>& one, const Point<N>& two);
// bool operator!=(const Point<N>& one, const Point<N>& two);
// Usage: if (one == two)
// ----------------------------------------------------------------------------
// Returns whether two points are equal or not equal.
template <int n>
bool operator==(const Point<n>& one, const Point<n>& two);

template <int n>
bool operator!=(const Point<n>& one, const Point<n>& two);

/** Point class implementation details */

template <int n>
int Point<n>::size() const {
  return n;
}

template <int n>
double& Point<n>::operator[](int index) {
  return coordinates_[index];
}

template <int n>
double Point<n>::operator[](int index) const {
  return coordinates_[index];
}

template <int n>
typename Point<n>::iterator Point<n>::begin() {
  return coordinates_;
}

template <int n>
typename Point<n>::const_iterator Point<n>::begin() const {
  return coordinates_;
}

template <int n>
typename Point<n>::iterator Point<n>::end() {
  return coordinates_ + n;
}

template <int n>
typename Point<n>::const_iterator Point<n>::end() const {
  return coordinates_ + n;
}

// Computing the distance uses the standard distance formula: the square root of
// the sum of the squares of the differences between matching components.
template <int n>
double distance(const Point<n>& one, const Point<n>& two) {
  double result = 0.0;
  for (int i = 0; i < n; i++) result += (one[i] - two[i]) * (one[i] - two[i]);

  return sqrt(result);
}

// Equality is implemented using the equal algorithm, which takes in two ranges
// and reports whether they contain equal values.
template <int n>
bool operator==(const Point<n>& one, const Point<n>& two) {
  return std::equal(one.begin(), one.end(), two.begin());
}

template <int n>
bool operator!=(const Point<n>& one, const Point<n>& two) {
  return !(one == two);
}

#endif  // POINT_H_
