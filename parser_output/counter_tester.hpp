#ifndef __DEVSMAP__PARSER__COUNTER_TESTER__HPP__
#define __DEVSMAP__PARSER__COUNTER_TESTER__HPP__

#include <iostream>
#include "cadmium/modeling/devs/coupled.hpp"
#include "counter.hpp"
#include "generator.hpp"

using namespace cadmium;

struct counter_tester: public Coupled {

	Port<bool> direction;
	Port<int> count;
	counter_tester(const std::string& id) : Coupled(id) {
		direction = addInPort<bool>("direction");
		count = addOutPort<int>("count");


		auto counter_model = addComponent<counter>("counter_model");
		auto generator_model = addComponent<generator>("generator_model");

		addCoupling(generator_model->inc_out, counter_model->increment_in);
		addCoupling(direction, counter_model->direction_in);
		addCoupling(counter_model->count_out, count);

	}
};
#endif //__DEVSMAP__PARSER__COUNTER_TESTER__HPP__

