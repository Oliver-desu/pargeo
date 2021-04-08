#include <iostream>
#include <algorithm>
#include "parlay/parallel.h"
#include "geometry/point.h"
#include "common/get_time.h"
#include "common/geometryIO.h"
#include "common/parse_command_line.h"
#include "knn.h"

using namespace std;
using namespace benchIO;
using namespace pargeo;

template<int dim>
void timeKnn(parlay::sequence<pargeo::point<dim>> &P, size_t k, int rounds, char const *outFile) {
  timer t; t.start();
  parlay::sequence<size_t> I;
  for(int i=0; i<rounds; ++i) {
    I = knn<dim>(P, k);
    cout << "round-time = " << t.get_next() << endl;
  }
  t.stop();
  if (outFile != NULL) writeIntSeqToFile(I, outFile);
}

int main(int argc, char* argv[]) {
  commandLine P(argc,argv,"[-o <outFile>] [-r <rounds>] <inFile>");
  char* iFile = P.getArgument(0);
  size_t k = P.getOptionIntValue("-k",2);
  char* oFile = P.getOptionValue("-o");
  int rounds = P.getOptionIntValue("-r",1);

  int dim = readDimensionFromFile(iFile);//todo make cheaper

  if (dim == 2) {
    parlay::sequence<pargeo::point<2>> Points = readPointsFromFile<pargeo::point<2>>(iFile);
    timeKnn<2>(Points, k, rounds, oFile);
  } else if (dim == 3) {
    parlay::sequence<pargeo::point<3>> Points = readPointsFromFile<pargeo::point<3>>(iFile);
    timeKnn<3>(Points, k, rounds, oFile);
  }
}
