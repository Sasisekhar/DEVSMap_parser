#ifndef __DEVSMAP__PARSER__COUNTER_HPP__
#define __DEVSMAP__PARSER__COUNTER_HPP__

#include <iostream>
#include "cadmium/modeling/devs/atomic.hpp"

using namespace cadmium;

struct counterState {
	int count;
	bool countUp;
	unsigned int increment;
	double sigma;

	counterState() : count(), countUp(), increment(), sigma() {}
};
std::ostream& operator<<(std::ostream& out, const counterState& s) {
	out << "{" << s.count << ", " << s.countUp << ", " << s.increment << ", " << s.sigma << "}";
	return out;
}

class counter: public Atomic<counterState>{

	public:

	Port<bool> direction_in;
	Port<int> increment_in;
	Port<int> count_out;

	counter(const std::string id) : Atomic<counterState>(id, counterState()) {
		direction_in = addInPort<bool>("direction_in");
		increment_in = addInPort<int>("increment_in");
		count_out = addOutPort<int>("count_out");
	}

	void internalTransition(counterState& state) const override {
		if (state.countUp == false) {
			state.count = state.count - state.increment;
		}
		else if (state.countUp == true) {
			state.count = state.count + state.increment;
		}
		else {
		}
	}

	void externalTransition(counterState& state, double e) const override {
		if (direction_in->getBag().size() != 0) {
			state.countUp = direction_in->getBag().at(direction_in->getBag().size() - 1);
		}
		else if (increment_in->getBag().size() != 0) {
			state.increment = increment_in->getBag().at(increment_in->getBag().size() - 1);
		}
		else {
			state.countUp = direction_in->getBag().at(direction_in->getBag().size() - 1);
			state.increment = increment_in->getBag().at(increment_in->getBag().size() - 1);
		}
	}

	void confluentTransition(counterState& state, double e) const override {
		if (direction_in->getBag().size() != 0) {
			if (direction_in->getBag().at(direction_in->getBag().size() - 1) == false) {
				state.count = state.count - state.increment;
				state.countUp = direction_in->getBag().at(direction_in->getBag().size() - 1);
			}
			else if (direction_in->getBag().at(direction_in->getBag().size() - 1) == true) {
				state.count = state.count + state.increment;
				state.countUp = direction_in->getBag().at(direction_in->getBag().size() - 1);
			}
		}
		else if (increment_in->getBag().size() != 0) {
			if (state.countUp == false) {
				state.count = state.count - increment_in->getBag().at(increment_in->getBag().size() - 1);
				state.increment = increment_in->getBag().at(increment_in->getBag().size() - 1);
			}
			else if (state.countUp == true) {
				state.count = state.count + increment_in->getBag().at(increment_in->getBag().size() - 1);
				state.increment = increment_in->getBag().at(increment_in->getBag().size() - 1);
			}
		}
		else {
			if (direction_in->getBag().at(direction_in->getBag().size() - 1) == false) {
				state.count = state.count - increment_in->getBag().at(increment_in->getBag().size() - 1);
				state.countUp = direction_in->getBag().at(direction_in->getBag().size() - 1);
				state.increment = increment_in->getBag().at(increment_in->getBag().size() - 1);
			}
			else if (direction_in->getBag().at(direction_in->getBag().size() - 1) == true) {
				state.count = state.count + increment_in->getBag().at(increment_in->getBag().size() - 1);
				state.countUp = direction_in->getBag().at(direction_in->getBag().size() - 1);
				state.increment = increment_in->getBag().at(increment_in->getBag().size() - 1);
			}
		}
	}

	void output(const counterState& state) const override {

			count_out->addMessage(state.count);

	}

	[[nodiscard]] double timeAdvance(const counterState& state) const override {

			return state.sigma;

	}

};

#endif //__DEVSMAP__PARSER__COUNTER_HPP__
