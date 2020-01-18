#pragma once

#include <cmath>
#include <limits>
#include <cassert>

//Not always Thread-Safe
class ExpoDampedPositiveCoeff {
private:
	//The speed at which the coefficient
	//damps to expected value
	//0 indicates constant
	//inf indicates immediate
	//actually constant and immediate do not need this
	double lambda = 100;
	double increm = 1 / 48000.0;
	//the minimum where computation stops
	double delta = exp(-5);
	//may be templated in the future
	double end_val = 0;
	double cur_val = 0;
	bool damping = false;
public:
	double getVal() {
		return cur_val;
	}
	void incVal() {
		if (damping) {
			double temp = end_val - cur_val;
			if (abs(temp) > delta)
				cur_val += temp * lambda * increm;
			else {
				cur_val = end_val;
				damping = false;
			}
		}
	}
	void setVal(double target) {
		end_val = target;
		if (lambda != 0 && lambda != std::numeric_limits<double>::infinity()) {
			if (abs(end_val - cur_val) > delta)
				damping = true;
		}
	}
	void setLambda(double Lambda) {
		assert(!(Lambda < 0));
		lambda = Lambda;
	}
	void setIncrem(double inc) {
		assert(inc > 0);
		increm = inc;
	}
	void setDelta(double d) {
		delta = d;
	}
	bool verify_inc() {
		return lambda > 0 && increm > 0 && (lambda * increm < 1);
	}
};

