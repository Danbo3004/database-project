/**
 * File: KDTree.h
 * ------------------------
 * An interface representing a kd-tree in some number of dimensions. The tree
 * can be constructed from a set of data and then queried for membership and
 * nearest neighbors.
 */
#ifndef KDTREE_H_
#define KDTREE_H_

#include <cstdlib>
#include <map>
#include <stdexcept>
#include <string>
#include <utility>
#include <set>

#include "bounded_priority_queue.h"
#include "point.h"

template <int N, typename ElemType>
struct Node {
  Point<N> point;
  ElemType value;
  struct Node<N, ElemType> *left_child;
  struct Node<N, ElemType> *right_child;
};

template <int N, typename ElemType>
struct Head {
  int size;
  struct Node<N, ElemType> *root;
};

template <int N, typename ElemType>
class KDTree {
 public:
  // Constructor: KDTree();
  // Usage: KDTree<3, int> myTree;
  // ----------------------------------------------------
  // Constructs an empty KDTree.
  KDTree();

  // KDTree(const KDTree& rhs);
  // KDTree& operator=(const KDTree& rhs);
  // Usage: KDTree<3, int> one = two;
  // Usage: one = two;
  // -----------------------------------------------------
  // Deep-copies the contents of another KDTree into this one.
  KDTree(const KDTree& rhs);
  KDTree& operator=(const KDTree& rhs);

  // Destructor: ~KDTree()
  // Usage: (implicit)
  // ----------------------------------------------------
  // Cleans up all resources used by the KDTree.
  ~KDTree();

  // size_t dimension() const;
  // Usage: size_t dim = kd.dimension();
  // ----------------------------------------------------
  // Returns the dimension of the points stored in this KDTree.
  int dimension() const;

  // size_t size() const;
  // bool empty() const;
  // Usage: if (kd.empty())
  // ----------------------------------------------------
  // Returns the number of elements in the kd-tree and whether the tree is
  // empty.
  int size() const;
  bool empty() const;

  // bool contains(const Point<N>& pt) const;
  // Usage: if (kd.contains(pt))
  // ----------------------------------------------------
  // Returns whether the specified point is contained in the KDTree.
  bool contains(const Point<N>& point) const;

  // void insert(const Point<N>& pt, const ElemType& value);
  // Usage: kd.insert(v, "This value is associated with v.");
  // ----------------------------------------------------
  // Inserts the point pt into the KDTree, associating it with the specified
  // value. If the element already existed in the tree, the new value will
  // overwrite the existing one.
  void insert(const Point<N>& point, const ElemType& value);

  // ElemType& operator[](const Point<N>& pt);
  // Usage: kd[v] = "Some Value";
  // ----------------------------------------------------
  // Returns a reference to the value associated with point pt in the KDTree.
  // If the point does not exist, then it is added to the KDTree using the
  // default value of ElemType as its key.
  ElemType& operator[](const Point<N>& point);

  // ElemType& at(const Point<N>& pt);
  // const ElemType& at(const Point<N>& pt) const;
  // Usage: cout << kd.at(v) << endl;
  // ----------------------------------------------------
  // Returns a reference to the key associated with the point pt. If the point
  // is not in the tree, this function throws an out_of_range exception.
  ElemType& at(const Point<N>& point);
  const ElemType& at(const Point<N>& point) const;

  // ElemType kNNValue(const Point<N>& key, size_t k) const
  // Usage: cout << kd.kNNValue(v, 3) << endl;
  // ----------------------------------------------------
  // Given a point v and an integer k, finds the k points in the KDTree
  // nearest to v and returns the most common value associated with those
  // points. In the event of a tie, one of the most frequent value will be
  // chosen.
  ElemType kNNValue(const Point<N>& key, int k) const;

 private:
   struct Head<N, ElemType> head;

   void deleteNode(struct Node<N, ElemType>* root);
   struct Node<N, ElemType>* insertNode(struct Node<N, ElemType>* root, const Point<N>& point, const ElemType& value, int depth);
   struct Node<N, ElemType>* findNode(struct Node<N, ElemType>* root, const Point<N>& point, int depth) const;
   void copyNode(struct Node<N, ElemType>** dst, struct Node<N, ElemType>*const* src);
   void searchKNNValue(struct Node<N, ElemType>* root, const Point<N>& key, BoundedPriorityQueue<ElemType>& bpq, int depth) const;
   ElemType decideKNNValue(BoundedPriorityQueue<ElemType> bpq) const;
};

/** KDTree class implementation details */

template <int N, typename ElemType>
KDTree<N, ElemType>::KDTree() {
  head.size = 0;
  head.root = NULL;
}

template <int N, typename ElemType>
KDTree<N, ElemType>::KDTree(const KDTree& rhs) {
  *this = rhs;
}

template <int N, typename ElemType>
KDTree<N, ElemType>& KDTree<N, ElemType>::operator=(const KDTree& rhs) {
  if (this != &rhs) {
    head.size = rhs.head.size;
    if (rhs.head.root != NULL)
      copyNode(&head.root, &rhs.head.root);
    else
      head.root = NULL;
  }
  return *this;
}

template <int N, typename ElemType>
KDTree<N, ElemType>::~KDTree() {
  if (head.root != NULL)
    deleteNode(head.root);
}

template <int N, typename ElemType>
int KDTree<N, ElemType>::dimension() const {
  return N;
}

template <int N, typename ElemType>
int KDTree<N, ElemType>::size() const {
  return head.size;
}

template <int N, typename ElemType>
bool KDTree<N, ElemType>::empty() const {
  return size() == 0;
}

template <int N, typename ElemType>
bool KDTree<N, ElemType>::contains(const Point<N>& point) const {
  if (head.root != NULL)
    return findNode(head.root, point, 0) != NULL;
  else
    return false;
}

template <int N, typename ElemType>
void KDTree<N, ElemType>::insert(const Point<N>& point, const ElemType& value) {
  if (head.root == NULL) {
    head.root = (struct Node<N, ElemType>*) malloc(sizeof(struct Node<N, ElemType>));
    head.root->point = point;
    head.root->value = value;
    head.root->left_child = NULL;
    head.root->right_child = NULL;
    head.size++;
  }
  else {
    insertNode(head.root, point, value, 0);
  }
}

template <int N, typename ElemType>
ElemType& KDTree<N, ElemType>::operator[](const Point<N>& point) {
  struct Node<N, ElemType> *node;
  node = findNode(head.root, point, 0);
  if (node == NULL)
    if (head.root == NULL) {
      head.root = (struct Node<N, ElemType>*) malloc(sizeof(struct Node<N, ElemType>));
      head.root->point = point;
      head.root->value = *(new ElemType());
      head.root->left_child = NULL;
      head.root->right_child = NULL;
      head.size++;
      return head.root->value;
    }
    else {
      return insertNode(head.root, point, *(new ElemType()), 0)->value;
    }
  else
    return node->value;
}

template <int N, typename ElemType>
ElemType& KDTree<N, ElemType>::at(const Point<N>& point) {
  return const_cast<ElemType&>(
      static_cast<const KDTree<N, ElemType>&>(*this).at(point));
}

template <int N, typename ElemType>
const ElemType& KDTree<N, ElemType>::at(const Point<N>& point) const {
  if (head.root == NULL) {
    throw std::out_of_range("Function at: out of range error");
  }
  else {
    struct Node<N, ElemType> *node;
    node = findNode(head.root, point, 0);
    if (node == NULL)
      throw std::out_of_range("Function at: out of range error");
    else
      return node->value;
  }
}

template <int N, typename ElemType>
ElemType KDTree<N, ElemType>::kNNValue(const Point<N>& key, int k) const {
  BoundedPriorityQueue<ElemType> bpq(k);
  searchKNNValue(head.root, key, bpq, 0);
  return decideKNNValue(bpq);
}

template<int N, typename ElemType>
void KDTree<N, ElemType>::deleteNode(struct Node<N, ElemType>* root) {
  if (root->left_child != NULL)
    deleteNode(root->left_child);
  if (root->right_child != NULL)
    deleteNode(root->right_child);
  free(root);
  root = NULL;
  head.size--;
}

template<int N, typename ElemType>
struct Node<N, ElemType>* KDTree<N, ElemType>::insertNode(
  Node<N, ElemType>* root, const Point<N>& point, const ElemType & value, int depth) {
  if (root->point == point) {
    root->value = value;
    return root;
  }

  int index = depth % N;
  if (point[index] < root->point[index])
    if (root->left_child == NULL) {
      root->left_child = (struct Node<N, ElemType>*) malloc(sizeof(struct Node<N, ElemType>));
      root->left_child->point = point;
      root->left_child->value = value;
      root->left_child->left_child = NULL;
      root->left_child->right_child = NULL;
      head.size++;
      return root->left_child;
    }
    else {
      return insertNode(root->left_child, point, value, depth + 1);
    }

  else
    if (root->right_child == NULL) {
      root->right_child = (struct Node<N, ElemType>*) malloc(sizeof(struct Node<N, ElemType>));
      root->right_child->point = point;
      root->right_child->value = value;
      root->right_child->left_child = NULL;
      root->right_child->right_child = NULL;
      head.size++;
      return root->right_child;
    }
    else {
      return insertNode(root->right_child, point, value, depth + 1);
    }
}

template<int N, typename ElemType>
struct Node<N, ElemType>* KDTree<N, ElemType>::findNode(
  struct Node<N, ElemType>* root, const Point<N>& point, int depth) const {
  if (root == NULL)
    return NULL;
  else if (root->point == point)
    return root;

  int index = depth % N;
  if (point[index] < root->point[index])
    return findNode(root->left_child, point, depth + 1);
  else
    return findNode(root->right_child, point, depth + 1);
}

template<int N, typename ElemType>
void KDTree<N, ElemType>::copyNode(struct Node<N, ElemType>** dst, struct Node<N, ElemType>*const* src) {
  (*dst) = (struct Node<N, ElemType> *) malloc(sizeof(struct Node<N, ElemType>));
  (*dst)->point = (*src)->point;
  (*dst)->value = (*src)->value;
  if ((*src)->left_child != NULL)
    copyNode(&(*dst)->left_child, &(*src)->left_child);
  else
    (*dst)->left_child = NULL;
  if ((*src)->right_child != NULL)
    copyNode(&(*dst)->right_child, &(*src)->right_child);
  else
    (*dst)->right_child = NULL;
}

template<int N, typename ElemType>
void KDTree<N, ElemType>::searchKNNValue(
  Node<N, ElemType>* root, const Point<N>& key, BoundedPriorityQueue<ElemType>& bpq, int depth) const {
  if (root == NULL)
    return;

  double dist = distance(root->point, key);
  bpq.enqueue(root->value, dist);

  int index = depth % N;
  if (key[index] < root->point[index])
    searchKNNValue(root->left_child, key, bpq, depth + 1);
  else
    searchKNNValue(root->right_child, key, bpq, depth + 1);

  if (bpq.size() < bpq.maxSize() || fabs(key[index] - root->point[index]) < bpq.worst())
    if (key[index] < root->point[index])
      searchKNNValue(root->right_child, key, bpq, depth + 1);
    else
      searchKNNValue(root->left_child, key, bpq, depth + 1);
  
  return;
}

template<int N, typename ElemType>
ElemType KDTree<N, ElemType>::decideKNNValue(BoundedPriorityQueue<ElemType> bpq) const
{
  std::multiset<ElemType> value_set;
  ElemType value;
  while (!bpq.empty()) {
    value = bpq.dequeueMin();
    value_set.insert(value);;
  }

  int max = 0;
  ElemType KNNValue;
  for (std::multiset<ElemType>::const_iterator i(value_set.begin()), end(value_set.end()); i != end; ++i) {
    if (max < value_set.count(*i)) {
      max = value_set.count(*i);
      KNNValue = *i;
    }
  }
  return KNNValue;
}

#endif  // KDTREE_H_
