#include <algorithm>
#include <tuple>
#include "parlay/parallel.h"
#include "parlay/primitives.h"
#include "geometry/point.h"
#include "common/geometry.h"
#include "common/get_time.h"
#include "spatialGraph.h"
#include "kdt.h"
#include "incremental/delaunay.h"

template<int dim>
parlay::sequence<edge> betaSkeleton(parlay::sequence<pargeo::point<dim>> &P, double beta) {
  using namespace parlay;
  using namespace kdt;
  using namespace std;
  using pt = pargeo::point<dim>;

  if (dim != 2)
    throw std::runtime_error("Error, beta skeleton only supports 2 dimensional inputs.");

  if (beta < 0)
    throw std::runtime_error("Error, beta skeleton only takes beta >= 0.");

#ifndef SILENT
  cout << "beta = " << beta << endl;
  timer t; t.start();
#endif
  sequence<edge> dedges = delaunayGraph<dim>(P);
#ifndef SILENT
  cout << "#delaunay-edges = " << dedges.size() << endl;
  cout << "delaunay-time = " << t.get_next() << endl;
#endif
  // Take a subset of qualifying edges

  kdNode<dim, pt>* tree = buildKdt<dim, pt>(P, true);
#ifndef SILENT
  cout << "build-tree-time = " << t.get_next() << endl;
#endif
  auto skeleton = parlay::filter(dedges,
				 [&](edge e) {
				   if (beta < 1) {
				     typename pt::floatT r = P[e.u].dist(P[e.v]) / (2*beta);
				     return !tree->nonEmptyLune(P[e.u], r, P[e.v], r, &P[e.u], &P[e.v]);
				   } else {
				     typename pt::floatT r = beta * P[e.u].dist(P[e.v]) / 2;
				     pt c1 = P[e.u]*(1-beta/2) + P[e.v]*(beta/2);
				     pt c2 = P[e.u]*(beta/2) + P[e.v]*(1-beta/2);
				     return !tree->nonEmptyLune(c1, r, c2, r, &P[e.u], &P[e.v]);
				   }
				 });
#ifndef SILENT
  cout << "edge-pruning-time = " << t.stop() << endl;
  cout << "#edges = " << skeleton.size() << endl;
#endif
  return skeleton;
}

template parlay::sequence<edge> betaSkeleton<2>(parlay::sequence<pargeo::point<2>> &, double);
template parlay::sequence<edge> betaSkeleton<3>(parlay::sequence<pargeo::point<3>> &, double);
