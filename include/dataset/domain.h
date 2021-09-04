#pragma once
#include <iostream>
#include <math.h>
#include "pargeo/point.h"
#include "constraint.h"
#include "parlay/parallel.h"
#include "parlay/utilities.h"

#define DEFAULT_PRECISION 0.01
namespace pargeo {

	template <typename floatT>
	floatT randFloat() {
		return floatT(rand()) / RAND_MAX;
	}
	template <typename floatT, class pointT>
	class _Domain {
	public:
		virtual int getSize() = 0;
		virtual pointT getPoint(int) = 0;
		virtual pointT* getPoints() = 0;
		virtual floatT getArea(int) = 0;
		virtual floatT getTotalArea() = 0;
		virtual pointT getRandomNeighbor(int) = 0;
		/*
		virtual int getIndex(pointT) = 0;
		floatT getArea(pointT p) { return getArea(getIndex(p)); }
		*/
	};

	template <typename floatT, class pointT>
	class _solid : public _Domain<floatT, pointT> {
		using Constraint = _Constraint<pointT>;
	public:
		int getSize() { return _size; }
		pointT getPoint(int index) { return _p[index]; }
		pointT* getPoints() { return _p; }
		floatT getArea(int index) {
			if (_satisfyConstraint(_p[index])) return getUnitArea();
			else return 0;
		}
		floatT getUnitArea() { return pow(_precision, _dim); }
		floatT getTotalArea() {
			return _domainSize * getUnitArea();
		}
		pointT getRandomNeighbor(int index) {
			pointT p = _p[index];
			floatT f[_dim];
			parlay::parallel_for(0, _dim, [&](size_t i) {
				f[i] = _precision * (randFloat<floatT>() - 0.5) + p[i];
				});
			return pointT(f);
		}

		void setPrecisionAuto(pointT a, pointT b) {
			float size = 1;
			parlay::parallel_for(0, a.dim, [&](size_t i) {
				size *= b[i] - a[i];
				});
			_precision = pow(size / 100000.0, 1.0 / a.dim);
		}

		_solid(pointT a, pointT b, floatT precision, Constraint* constraint, int num_constraint) {
			_num_constraint = num_constraint;
			_constraint = constraint;
			_precision = precision;
			setPrecisionAuto(a, b);
			_dim = a.dim;
			_setup(a, b);
			if (_domainSize == 0) {
				std::cout << "You are creating an empty domain! Therefore program terminated!\n";
				exit(1);
			}
		}
		_solid(pointT a, pointT b, Constraint* constraint, int num_constraint) :_solid(a, b, DEFAULT_PRECISION, constraint, num_constraint) {}
		_solid(pointT a, pointT b, floatT precision) : _solid(a, b, precision, NULL, 0) {}
		_solid(pointT a, pointT b) : _solid(a, b, DEFAULT_PRECISION) {}
		_solid() : _solid(pointT(-1), pointT(1)) {}
		void print(pointT* p) {
			std::cout << "area: " << getUnitArea() << "\n";	// expect 0.0001;
			std::cout << "total area: " << getTotalArea() << "\n";	// expect around 4;
			std::cout << "point 200 c0: " << getPoints()[200][0] << "\n";	// expect -0.99;
			std::cout << "point 200 c1: " << getPoints()[200][1] << "\n";	// expect around -1;
			std::cout << "point 321 c0: " << getPoints()[321][0] << "\n";	// expect -0.99;
			std::cout << "point 321 c1: " << getPoints()[321][1] << "\n";	// expect around 0.2;
			if (p != NULL) {
				for (int i = 0; i < _num_constraint; i++) {
					std::cout << i << "th constraint: " << _constraint[i].isSatisfied(*p) << "\n";	// expect around 0.2;
				}
			}
		}
	private:
		int _size;
		int _domainSize;
		int _dim;
		floatT _precision;
		pointT* _p;
		int _num_constraint;
		Constraint* _constraint;
		void _setup(pointT a, pointT b) {
			int index[_dim + 1];
			index[_dim] = 1;
			for (int i = _dim - 1; i >= 0; i--) {
				index[i] = floor((b[i] - a[i]) / _precision) * index[i + 1];
			}
			_size = index[0];
			_p = (pointT*)(std::malloc(sizeof(pointT) * _size));
			_domainSize = 0;
			for (int i = 0; i < _size; i++) {
				floatT coord[_dim];
				int k = i;
				for (int j = 0; j < _dim; j++) {
					coord[j] = floatT(k / index[j + 1]) * _precision + a[j];
					k = k % index[j + 1];
				}

				_p[i] = pointT(coord);
				if (_satisfyConstraint(_p[i])) {
					_domainSize++;
				}
			}
		}

		bool _satisfyConstraint(pointT p) {
			int result = 1;
			parlay::parallel_for(0, _num_constraint, [&](size_t i) {
				if (!_constraint[i].isSatisfied(p)) result = 0;
				});
			return result;
		}
	};

	template <int dim>
	using solid = _solid<typename point<dim>::floatT, point<dim>>;

	void testDomain() {
		std::cout << "------------- testDomain started ----------------\n";
		const int dim = 2;
		using Solid = solid<dim>;
		Solid s = Solid();
		s.print(NULL);
		std::cout << "------------- testDomain finished ----------------\n";
	}
}
