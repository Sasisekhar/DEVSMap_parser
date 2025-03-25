#ifndef __DEVSMAP__PARSER__GENERATOR__HPP__
#define __DEVSMAP__PARSER__GENERATOR__HPP__

#include <iostream>
#include "cadmium/modeling/devs/atomic.hpp"

using namespace cadmium;

struct generatorState {
	int inc;

	generatorState() : inc() {}
};
std::ostream& operator<<(std::ostream& out, const generatorState& s) {
	out << "{" << "inc:" << s.inc << "}";
	return out;
}

class generator: public Atomic<generatorState>{

	public:

	Port<int> inc_out;

	generator(const std::string id) : Atomic<generatorState>(id, generatorState()) {
		inc_out = addOutPort<int>("inc_out");
	}

	void internalTransition(generatorState& state) const override {

			state.inc = 1;

	}

	void externalTransition(generatorState& state, double e) const override {


	}

	void confluentTransition(generatorState& state, double e) const override {


	}

	void output(const generatorState& state) const override {

			inc_out->addMessage(state.inc);

	}

	[[nodiscard]] double timeAdvance(const generatorState& state) const override {

			return 1.0;

	}

};

#endif //__DEVSMAP__PARSER__GENERATOR_HPP__

