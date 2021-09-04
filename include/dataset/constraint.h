#include <iostream>
#include "pargeo/point.h"
#include "parlay/parallel.h"
#include "parlay/utilities.h"

namespace pargeo {
	template<class pointT>
	class _Constraint {
	public:
		bool isSatisfied(pointT p) {
			return _function(p, _param_p, _param);
		}
		_Constraint(int* num_param, double* param, pointT* param_p, bool (*function)(pointT, pointT*, double*)) {
			if (num_param != NULL) {
				_param = (double*)std::malloc(sizeof(double) * num_param[0]);
				std::memcpy(_param, param, sizeof(double) * num_param[0]);
				_param_p = (pointT*)std::malloc(sizeof(pointT) * num_param[1]);
				std::memcpy(_param_p, param_p, sizeof(pointT) * num_param[1]);
			}
			_function = function;
		}
		_Constraint(bool (*function)(pointT, pointT*, double*)) : _Constraint(NULL, NULL, NULL, function) {}
	private:
		double* _param;
		pointT* _param_p;
		bool (*_function)(pointT, pointT*, double*);
	};

	template<class pointT>
	bool sphereConstraint(pointT p, pointT* param_p, double* param) {
		bool in = (param[0] > 0);
		double radius = param[1];
		pointT centre = param_p[0];
		bool result_in = centre.dist(p) <= radius;
		if (in) return result_in;
		else return !result_in;
	}

	template<class pointT>
	bool cubeConstraint(pointT p, pointT* param_p, double* param) {
		bool in = (param[0] > 0);
		pointT a = param_p[0];
		pointT b = param_p[1];
		bool result_in = 1;
		parlay::parallel_for(0, a.dim, [&](size_t i) {
			if (a[i] > p[i] || p[i] > b[i]) {
				result_in = 0;
			}
			});
		if (in) return result_in;
		else return !result_in;
	}

	template <class pointT>
	bool userConstraint(pointT p, pointT* param_p, double* param) {
		int dim = p.dim;
		int result = 1;
		int option = 1;
		switch (option) {
		case 1:
			if (p[0] + p[1] >= 1) result = 0;
			break;
		case 2:
			if (p[1] >= p[0] * p[0]) result = 0;
			break;
		}
		return result;
	}

	void testConstraint() {
		std::cout << "------------- testConstraint started ----------------\n";
		const int dim = 2;
		using Point = point<dim>;
		using Constraint = _Constraint<Point>;
		double param[] = { 1 ,  1 };
		Point param_p[] = { Point(1.0) };
		int num_param[] = { 2,1 };
		Constraint s = Constraint(num_param, param, param_p, &sphereConstraint<Point>);
		std::cout << s.isSatisfied(Point(0.0)) << "\n";	//expected 0
		std::cout << s.isSatisfied(Point(1.5)) << "\n"; //expected 1
		double param2[] = { -1 };
		Point param2_p[] = { Point(1.0), Point(2.0) };
		int num_param2[] = { 1,2 };
		Constraint c = Constraint(num_param2, param2, param2_p, &cubeConstraint<Point>);
		std::cout << c.isSatisfied(Point(0.0)) << "\n";	//expected 1
		std::cout << c.isSatisfied(Point(1.5)) << "\n"; //expected 0
		Constraint u = Constraint(&userConstraint<Point>);
		std::cout << u.isSatisfied(Point(0.0)) << "\n";	//expected 1
		std::cout << u.isSatisfied(Point(1.5)) << "\n"; //expected 0
		std::cout << "Expected 0110??\n";
		std::cout << "------------- testConstraint started ----------------\n";
	}
}