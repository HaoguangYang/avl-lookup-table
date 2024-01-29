/**
 * @file avl_tree.hpp
 * @author Haoguang Yang (yang1510@purdue.edu)
 * @version 0.1
 * @date 2023-11-26
 *
 * @copyright Copyright (c) 2023 Haoguang Yang
 *
 * Licensed under the Apache License, Version 2.0 (the "License"); you may not use this file except
 * in compliance with the License. You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software distributed under the License
 * is distributed on an "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express
 * or implied. See the License for the specific language governing permissions and limitations under
 * the License.
 *
 */

#ifndef ND_LOOKUP_TABLE__AVL_TREE_HPP_
#define ND_LOOKUP_TABLE__AVL_TREE_HPP_

#include <algorithm>
#include <utility>

namespace LUT {

template <typename DataT>
class BinaryTreeNode {
 protected:
  unsigned int height_;
  BinaryTreeNode* left_;
  BinaryTreeNode* right_;

 public:
  DataT data;

  explicit BinaryTreeNode(const DataT& data)
      : height_(1), left_(nullptr), right_(nullptr), data(data) {}

  ~BinaryTreeNode() {
    if (left_ != nullptr) delete left_;
    if (right_ != nullptr) delete right_;
  }

  unsigned int getHeight() const { return height_; }

  static unsigned int getHeight(BinaryTreeNode* node) {
    return (node != nullptr) ? node->getHeight() : 0;
  }

  static int getBalance(BinaryTreeNode* node) {
    return (node != nullptr)
               ? static_cast<int>(getHeight(node->getLeft()) - getHeight(node->getRight()))
               : 0;
  }

  BinaryTreeNode* getLeft() const { return left_; }
  BinaryTreeNode* getRight() const { return right_; }

  void setLeft(BinaryTreeNode* node) {
    left_ = node;
    height_ = 1U + std::max(getHeight(left_), getHeight(right_));
  }

  void setRight(BinaryTreeNode* node) {
    right_ = node;
    height_ = 1U + std::max(getHeight(left_), getHeight(right_));
  }
};

template <typename DataT, typename CompareFunc>
class AVL_Tree {
 protected:
  typedef BinaryTreeNode<DataT> NodeT;
  CompareFunc comp_;
  NodeT* root_;

  static NodeT* rightRotate(NodeT* y) {
    NodeT* x = y->getLeft();
    NodeT* subtree = x->getRight();

    y->setLeft(subtree);
    x->setRight(y);

    return x;
  }

  static NodeT* leftRotate(NodeT* x) {
    NodeT* y = x->getRight();
    NodeT* subtree = y->getLeft();

    x->setRight(subtree);
    y->setLeft(x);

    return y;
  }

  static NodeT* insert(NodeT* node, const DataT& data, CompareFunc& comp) {
    if (node == nullptr) {
      return new NodeT(data);
    }

    auto comp_this = comp(data, node->data);
    if (comp_this < 0) {
      node->setLeft(insert(node->getLeft(), data, comp));
    } else if (comp_this > 0) {
      node->setRight(insert(node->getRight(), data, comp));
    } else {
      // Duplicate keys are not allowed
      return node;
    }

    int balance = NodeT::getBalance(node);

    // Left Right / Right Right Case
    if (balance > 1) {
      if (comp(data, node->getLeft()->data) > 0) node->setLeft(leftRotate(node->getLeft()));
      return rightRotate(node);
    }

    // Right Left / Left Left Case
    if (balance < -1) {
      if (comp(data, node->getRight()->data) < 0) node->setRight(rightRotate(node->getRight()));
      return leftRotate(node);
    }

    return node;
  }

  static NodeT* remove(NodeT* node, const DataT& data, CompareFunc& comp) {
    if (node == nullptr) return node;

    auto comp_this = comp(data, node->data);
    if (comp_this < 0) {
      node->setLeft(remove(node->getLeft(), data, comp));
    } else if (comp_this > 0) {
      node->setRight(remove(node->getRight(), data, comp));
    } else {
      // remove this node
      NodeT* left = node->getLeft();
      NodeT* right = node->getRight();
      if (!left) {
        if (!right) {
          // no child
          delete node;
          return nullptr;
        }
        // right child only
        node->setRight(nullptr);
        delete node;
        // subtrees are already balanced. Directly return it.
        return right;
      } else if (!right) {
        // left child only
        node->setLeft(nullptr);
        delete node;
        // subtrees are already balanced. Directly return it.
        return left;
      }

      // both left and right children. First get the smallest value in the right tree (in-order
      // successor)
      NodeT* successor = right;
      while (NodeT* temp = successor->getLeft()) {
        successor = temp;
      }
      node->data = successor->data;
      node->setRight(remove(node->getRight(), successor->data, comp));
    }

    int balance = NodeT::getBalance(node);

    // Left Right / Right Right Case
    if (balance > 1) {
      if (NodeT::getBalance(node->getLeft()) < 0) node->setLeft(leftRotate(node->getLeft()));
      return rightRotate(node);
    }

    // Right Left / Left Left Case
    if (balance < -1) {
      if (NodeT::getBalance(node->getRight()) > 0) node->setRight(rightRotate(node->getRight()));
      return leftRotate(node);
    }

    return node;
  }

 public:
  explicit AVL_Tree(const CompareFunc& comp) : comp_{comp}, root_{nullptr} {}

  ~AVL_Tree() {
    if (root_ != nullptr) delete root_;
  }

  const NodeT* getRoot() { return root_; }

  void insert(const DataT& data) { root_ = insert(root_, data, comp_); }

  void remove(const DataT& data) { root_ = remove(root_, data, comp_); }

  std::pair<const DataT*, const DataT*> lookup(const DataT& input) const {
    std::pair<const DataT*, const DataT*> ret{nullptr, nullptr};
    const NodeT* node = root_;
    const NodeT* lower = nullptr;
    const NodeT* upper = nullptr;

    while (node != nullptr) {
      if (comp_(input, node->data) < 0) {
        upper = node;
        node = node->getLeft();
      } else if (comp_(input, node->data) > 0) {
        lower = node;
        node = node->getRight();
      } else {
        // Exact match
        ret.first = &(node->data);
        ret.second = &(node->data);
        return ret;
      }
    }

    if (lower != nullptr) {
      // Input key is smaller than any key in the table
      ret.first = &(lower->data);
    }
    if (upper != nullptr) {
      ret.second = &(upper->data);
    }
    return ret;
  }
};

}  // namespace LUT

#endif  // ND_LOOKUP_TABLE__AVL_TREE_HPP_
