#pragma once
#include <iostream>
#include "distribution.h"
#include "pargeo/point.h"
#include "pargeo/pointIO.h"
#include "parlay/parallel.h"
#include "parlay/utilities.h"

namespace pargeo {
	template <typename floatT, class pointT>
	void plot(_Distribution<floatT, pointT>* t, int N, const char* fName) {
		using namespace parlay;
		sequence<pointT> p = sequence<pointT>(N);
		parallel_for(0, N, [&](size_t i) {
			floatT f = randFloat<floatT>();
			pointT x = t->generatePoint(f);
			p[i] = t->generatePoint(f);
			});
		pargeo::pointIO::writePointsToFile(p, fName);
	}

	template <int dim>
	point<dim> scanPoint(const std::string name) {
		double f[dim];
		for (int i = 0; i < dim; i++) {
			std::cout << "Please enter coordinate " << i << " of " << name << " point: ";
			while (scanf("%lf", f + i) == EOF) { ; }
		}
		return point<dim>(f);
	}

	double scanDouble(const std::string name) {
		double f;
		std::cout << "Please enter the " << name << " of domain: ";
		while (scanf("%lf", &f) == EOF) { ; }
		return f;
	}

	template <int dim>
	solid<dim>* scanCube(bool user) {
		using Point = point<dim>;
		using Constraint = _Constraint<Point>;
		using Solid = solid<dim>;
		double f1[dim];
		double f2[dim];
		Solid* result = static_cast<Solid*>(std::malloc(sizeof(Solid)));
		for (int i = 0; i < dim; i++) {
			std::cout << "Please enter lowerbond of coordinate " << i << " of cube domain: ";
			while (scanf("%lf", f1 + i) == EOF) { ; }
			std::cout << "Please enter upperbond of coordinate " << i << " of cube domain: ";
			while (scanf("%lf", f2 + i) == EOF) { ; }
		}
		double t = scanDouble("thickness");
		Constraint* constraint = static_cast<Constraint*>(std::malloc(sizeof(Constraint) * 2));
		int num_constraint = 0;
		int num_param[] = { 1,2 };
		if (user) {
			constraint[num_constraint] = Constraint(&userConstraint<Point>);
			num_constraint++;
		}
		if (t <= 0) {
			*result = Solid(Point(f1), Point(f2), constraint, num_constraint);
			return result;
		}
		else {
			double param[] = { -1 };
			Point param_p[] = { Point(f1) + Point(t), Point(f2) - Point(t) };
			constraint[num_constraint] = Constraint(num_param, param, param_p, &cubeConstraint<Point>);
			*result = Solid(Point(f1) - Point(t), Point(f2) + Point(t), constraint, num_constraint + 1);
			return result;
		}
	}

	template <int dim>
	solid<dim>* scanSphere(bool user) {
		using Point = point<dim>;
		using Constraint = _Constraint<Point>;
		using Solid = solid<dim>;
		Point p = scanPoint<dim>("centre");
		double r = scanDouble("radius");
		double t = scanDouble("thickness");
		Solid* result = static_cast<Solid*>(std::malloc(sizeof(Solid)));
		Constraint* constraint = static_cast<Constraint*>(std::malloc(sizeof(Constraint) * 3));
		int num_constraint = 0;
		if (user) {
			constraint[num_constraint] = Constraint(&userConstraint<Point>);
			num_constraint++;
		}
		Point param_p[] = { p };
		int num_param[] = { 2,1 };
		if (t <= 0 || t >= r) {
			double param[] = { 1 , r };
			constraint[num_constraint] = Constraint(num_param, param, param_p, &sphereConstraint<Point>);
			*result = Solid(p - Point(r), p + Point(r), constraint, num_constraint + 1);
			return result;
		}
		else {
			double param[] = { -1 , r - t };
			double param2[] = { 1 ,r + t };
			constraint[num_constraint] = Constraint(num_param, param, param_p, &sphereConstraint<Point>);
			constraint[num_constraint + 1] = Constraint(num_param, param2, param_p, &sphereConstraint<Point>);
			*result = Solid(p - Point(r + t), p + Point(r + t), constraint, num_constraint + 2);
			return result;
		}
	}

	void testUtilities() {
		const int dim = 2;
		using Point = point<dim>;
		using Solid = solid<dim>;
		Solid s = *scanSphere<dim>(false);
		s.print(NULL);
		Solid c = *scanCube<dim>(false);
		c.print(NULL);
		using Uniform = uniform<dim>;
		Uniform u = Uniform(s);
	}
}