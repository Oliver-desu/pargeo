#include "dataset/generator.h"
#include "pargeo/parseCommandLine.h"

void test() {
	using namespace pargeo;
	testDomain();
	testDistribution();
	testConstraint();
	const int dim = 2;
	using Uniform = uniform<dim>;
	Uniform u = Uniform();
	plot<typename point<dim>::floatT, point<dim>>(&u, 500, "a.txt");
}

template <int dim>
int generate(char shape, bool user, int n, char* fName) {
	using namespace pargeo;
	using Solid = solid<dim>;
	using Uniform = uniform<dim>;
	Solid s;
	if (shape == 's') s = *scanSphere<dim>(user);
	else if (shape == 'c') s = *scanCube<dim>(user);
	else return 1;	
	Uniform u = Uniform(s);
	plot<typename point<dim>::floatT, point<dim>>(&u, n, fName);
	return 0;
}

int main(int argc, char** argv) {
	using namespace pargeo;

	std::string text = "[-s {shape}] [-u] [-d {2-9}] n <outfile>";
	text += "\n polygon type: 'c' cube; 's' sphere";
	text += "\n -u: also use user constraint(edit in include/dataset/constraint.h/userConstraint)";
	text += "\n  o.w. default to generate using uniform distribution, in shape cube with dimension 2";
	commandLine P(argc, argv, text);

	std::pair<size_t, char*> in = P.sizeAndFileName();
	size_t n = in.first;
	char* fName = in.second;

	int dim = P.getOptionIntValue("-d", 2);
	char shape = P.getOptionValue("-s", "c")[0];
	bool user = P.getOption("-u");

	int status = 0;
	if (dim == 2) { if (status = generate<2>(shape, user, n, fName)) P.badArgument(); }
	else if (dim == 3) { if (status = generate<3>(shape, user, n, fName)) P.badArgument(); }
	else if (dim == 4) { if (status = generate<4>(shape, user, n, fName)) P.badArgument(); }
	else if (dim == 5) { if (status = generate<5>(shape, user, n, fName)) P.badArgument(); }
	else if (dim == 6) { if (status = generate<6>(shape, user, n, fName)) P.badArgument(); }
	else if (dim == 7) { if (status = generate<7>(shape, user, n, fName)) P.badArgument(); }
	else if (dim == 8) { if (status = generate<8>(shape, user, n, fName)) P.badArgument(); }
	else if (dim == 9) { if (status = generate<9>(shape, user, n, fName)) P.badArgument(); }
	else {
		fprintf(stderr, "dim %d not supported, please use dim 2-9\n", dim);
		status = 1;
	}
	return status;
}