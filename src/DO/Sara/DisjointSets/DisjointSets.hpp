// ========================================================================== //
// This file is part of Sara, a basic set of libraries in C++ for computer
// vision.
//
// Copyright (C) 2015-2016 David Ok <david.ok8@gmail.com>
//
// This Source Code Form is subject to the terms of the Mozilla Public
// License v. 2.0. If a copy of the MPL was not distributed with this file,
// you can obtain one at http://mozilla.org/MPL/2.0/.
// ========================================================================== //

#ifndef DO_SARA_DISJOINTSETS_DISJOINTSETS_HPP
#define DO_SARA_DISJOINTSETS_DISJOINTSETS_HPP

#include <unordered_map>

#include <DO/Sara/Defines.hpp>

#include <DO/Sara/DisjointSets/AdjacencyList.hpp>


namespace DO { namespace Sara {

  //! @brief Disjoint-set data structure.
  class DO_SARA_EXPORT DisjointSets
  {
    class Node;

  public:
    using node_type = Node;
    using node_ptr = node_type *;

    using size_type = AdjacencyList::size_type;
    using vertex_type = AdjacencyList::vertex_type;
    using adjacent_vertex_iterator = AdjacencyList::const_out_vertex_iterator;

    DisjointSets(size_t num_elements, const AdjacencyList& adjacency_list)
      : _adj_list(adjacency_list)
      , _vertices(num_elements)
    {
    }

    inline void make_set(vertex_type v)
    {
      _vertices[v].set_parent(&_vertices[v]);
      _vertices[v].set_rank(0);
    }

    inline node_ptr find_set(node_ptr x) const
    {
      if (x != x->parent())
        return find_set(x->parent());
      return x->parent();
    }

    inline void link(node_ptr x, node_ptr y)
    {
      if (x->rank() < y->rank())
        x->set_parent(y);
      else
        y->set_parent(x);
      if (x->rank() == y->rank())
        y->set_rank(y->rank() + 1);
    }

    inline void join(node_ptr x, node_ptr y)
    {
      link(find_set(x), find_set(y));
    }

    inline vertex_type component(vertex_type v) const
    {
      node_ptr vertex = const_cast<node_ptr>(&_vertices[v]);
      node_ptr parent = const_cast<node_ptr>(find_set(vertex));
      return parent - &_vertices[0];
    }

    void compute_connected_components();

    std::vector<std::vector<vertex_type>> get_connected_components() const;

  private:
    class Node
    {
    public:
      // Constructor
      inline Node() : _parent(nullptr), _rank(0) {}
      // Setters.
      void set_parent(Node *head) { _parent = head; }
      void set_rank(int rank) { _rank = rank; }
      // Getters.
      Node * parent() const { return _parent; }
      int rank() const { return _rank; }
    private:
      Node *_parent;
      int _rank;
    };

  private:
    const AdjacencyList& _adj_list;
    std::vector<Node> _vertices;
  };

} /* namespace Sara */
} /* namespace DO */


#endif /* DO_SARA_DISJOINTSETS_DISJOINTSETS_HPP */
