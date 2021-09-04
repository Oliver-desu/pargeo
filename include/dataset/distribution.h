#pragma once
#include <iostream>
#include "pargeo/point.h"
#include "domain.h"

namespace pargeo {
	template <typename floatT, class pointT>
	class _Distribution {
	public:
		virtual floatT getDensity(int) = 0;
		virtual floatT getProbability(int) = 0;
		virtual pointT generatePoint(floatT) = 0;
	};

	template <typename floatT, class pointT>
	class _uniform : public _Distribution<floatT, pointT> {
		using Domain = _Domain<floatT, pointT>;
		using Solid = _solid<floatT, pointT>;
	public:
		floatT getDensity(int index) { return 1 / _totalArea; }
		floatT getProbability(int index) {
			return _domain->getArea(index) * getDensity(index);
		}

		_uniform(Domain* domain) {
			_domain = domain;
			_totalArea = _domain->getTotalArea();
			_size = _domain->getSize();
			build();
		}
		_uniform(Solid solid) : _uniform(&solid) {}
		_uniform() : _uniform(Solid()) {}
		void build() {
			_cdf = (floatT*)(std::malloc(sizeof(floatT) * _size));
			_cdf[0] = getProbability(0);
			for (int i = 1; i < _size; i++) {
				_cdf[i] = _cdf[i - 1] + getProbability(i);
			}
		}
		pointT generatePoint(floatT f) {
			f = 1 - f;
			int p = 0;
			int q = _size - 1;
			if (f <= _cdf[p]) return _domain->getRandomNeighbor(p);
			int m;
			while (q - p > 1) {
				m = (p + q) / 2;
				if (_cdf[m] == f) return m;
				else if (_cdf[m] < f) p = m;
				else q = m;
			}
			return _domain->getRandomNeighbor(q);
		}
	private:
		int _size;
		floatT* _cdf;
		floatT _totalArea;
		Domain* _domain;
	};

	template <int dim>
	using uniform = _uniform<typename point<dim>::floatT, point<dim>>;

	void testDistribution() {
		std::cout << "------------- testDistribution started ----------------\n";
		const int dim = 2;
		using Uniform = uniform<dim>;
		Uniform u = Uniform();
		std::cout << "density: " << u.getDensity(0) << "\n";				// expect to get 0.25;
		std::cout << "probability: " << u.getProbability(0) << "\n";		// expect to get 0.000025;
		std::cout << "------------- testDistribution finished ----------------\n";
	}
}
