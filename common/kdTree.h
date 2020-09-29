// Copyright (c) 2020 Yiqiu Wang
//
// Permission is hereby granted, free of charge, to any person obtaining a
// copy of this software and associated documentation files (the
// "Software"), to deal in the Software without restriction, including
// without limitation the rights (to use, copy, modify, merge, publish,
// distribute, sublicense, and/or sell copies of the Software, and to
// permit persons to whom the Software is furnished to do so, subject to
// the following conditions:
//
// The above copyright notice and this permission notice shall be included
// in all copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS
// OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
// MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
// NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE
// LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION
// OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION
// WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

#ifndef KD_TREE_H
#define KD_TREE_H

#include <math.h>
#include <vector>
#include "pbbs/parallel.h"
#include "pbbs/utils.h"
#include "pbbs/sequence.h"
#include "pbbs/quickSort.h"
#include "geometry.h"
#include "kdNode.h"

// *************************************************************
//   A generic parallel Euclidean kdTree
// *************************************************************
// objT needs to be templatized with int dim,
// and supports coordinate() that returns floatT[dim],
// and coordinate(i) that returns floatT* A[i]
// and dist(objT p2) that computes euclidean distance with p2
// and objT need an empty constructor for empty value

template<int dim, class objT>
class kdTree {
  typedef double floatT;
  typedef point<dim> pointT;
  typedef kdNode<dim, objT> nodeT;
  objT **items;
  nodeT *root;
  intT n;

  public:
  nodeT* rootNode() {return root;}
  intT size() {return n;}

  kdTree(objT* P, intT nn, bool parallel=true, bool noCoarsen=false): n(nn) {
    items = newA(objT*, n);
    par_for(intT i=0; i<n; ++i) {
      items[i]=&P[i];
    }
    root = newA(nodeT, 2*n-1);
    par_for(intT i=0; i<2*n-1; ++i) {
      root[i].setEmpty();
    }
    if (parallel) {
      objT** scratch = newA(objT*, n);
      intT* flags = newA(intT, n);
      root[0] = nodeT(items, n, root+1, scratch, flags, noCoarsen);
      free(scratch);
      free(flags);
    } else {
      root[0] = nodeT(items, n, root+1, noCoarsen);}
  }
  ~kdTree() {
    free(items);
    free(root);}

  vector<objT*>* rangeNeighbor(objT* query, floatT r) {
    vector<objT*>* accum = new vector<objT*>();
    pointT pMin1 = pointT();
    pointT pMax1 = pointT();
    floatT* center = query->coordinate();
    for (int i=0; i<dim; ++i) {
      pMin1.updateX(i, center[i]-r);
      pMax1.updateX(i, center[i]+r);}
    root->rangeNeighbor(pMin1, pMax1, r, accum);
    return accum;
  }

  //Deprecate
  template<class func>
  void rangeNeighbor(objT* query, floatT r, func* f) {
    pointT pMin1 = pointT();
    pointT pMax1 = pointT();
    floatT* center = query->coordinate();
    for (int i=0; i<dim; ++i) {
      pMin1.updateX(i, center[i]-r);
      pMax1.updateX(i, center[i]+r);}
    root->rangeNeighbor(pMin1, pMax1, r, f);
  }

  template<class func, class func2>
  void rangeNeighbor(objT* query, floatT r, func term, func2 doTerm) {
    pointT pMin1 = pointT();
    pointT pMax1 = pointT();
    floatT* center = query->coordinate();
    for (int i=0; i<dim; ++i) {
      pMin1.updateX(i, center[i]-r);
      pMax1.updateX(i, center[i]+r);}
    root->rangeNeighbor(pMin1, pMax1, r, term, doTerm);
  }

  objT** kNN(objT* q, intT k, objT** R=NULL) {
    return rootNode()->kNN(q, k, R);
  }

};

#endif
