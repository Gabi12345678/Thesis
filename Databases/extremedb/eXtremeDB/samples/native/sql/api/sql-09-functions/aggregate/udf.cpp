/*
*   Aggregating UDF
*/

#include "mcoapi.h"
#include "mcosql.h"
#include "sqlcpp.h"

#include <math.h>

#ifdef _WIN32
  const unsigned long nan_val[2]={0xffffffff, 0x7fffffff};
  #define getnan()  (*( double* )nan_val)
#else
  #define getnan() (0.0/0.0)
#endif

extern "C" {
  McoSql::Value* bx_wt_avg(McoSql::Runtime* runtime, McoSql::Vector<McoSql::Value>* params);
}

McoSql::Value* bx_wt_avg(McoSql::Runtime* runtime, McoSql::Vector<McoSql::Value>* params)
{
	const double NaN = getnan();
        McoSql::Allocator* allocator = &runtime->memory;
        McoSql::Array* state = (McoSql::Array*)params->items[0];
        McoSql::Array* ops = (McoSql::Array*)params->items[1];
	params->items[0] = NULL; // prevent destruction of state
	if (!ops->isNull())  {
		double el = ops->getAt(0)->realValue();
		double wt = ops->getAt(1)->realValue();
		if (state->isNull()) {
			state = McoSql::Array::create(allocator, McoSql::tpReal, sizeof(double), 4);
		}
		double sum_of_products = state->getAt(0)->realValue();
		double current_sum = state->getAt(1)->realValue();
		double current_count = state->getAt(2)->realValue();
		double sum_of_sq_products = state->getAt(3)->realValue();

		sum_of_products += el * wt;
		current_sum += wt;
		current_count += 1;
		sum_of_sq_products += el * el * wt;

		McoSql::RealValue s1(sum_of_products);
		McoSql::RealValue s2(current_sum);
		McoSql::RealValue s3(current_count);
		McoSql::RealValue s4(sum_of_sq_products);
		state->setAt(0, &s1);
		state->setAt(1, &s2);
		state->setAt(2, &s3);
		state->setAt(3, &s4);

	} else if (!state->isNull()) {
		double sum_of_products = state->getAt(0)->realValue();
		double current_sum = state->getAt(1)->realValue();
		double current_count = state->getAt(2)->realValue();
		double sum_of_sq_products = state->getAt(3)->realValue();
		double stddev;		
        McoSql::Array* result = McoSql::Array::create(allocator, McoSql::tpReal, sizeof(double), 7);

		if (current_sum == 0)
		{
			result->setAt(0, McoSql::RealValue::create(allocator, NaN));
			result->setAt(1, McoSql::RealValue::create(allocator, NaN));
			result->setAt(2, McoSql::RealValue::create(allocator, NaN));
			result->setAt(3, McoSql::RealValue::create(allocator, NaN));
			result->setAt(4, McoSql::RealValue::create(allocator, NaN));
			result->setAt(5, McoSql::RealValue::create(allocator, NaN));
			result->setAt(6, McoSql::RealValue::create(allocator, NaN));
		} else {
			if (current_count > 1) {
				double stddev = (current_sum * sum_of_sq_products - sum_of_products * sum_of_products) / (current_sum * (current_sum - 1));
				if (stddev >= 0) {
					stddev = sqrt(stddev);
				} else if (stddev > -1) {
					stddev = 0;
				}	else {
					stddev = NaN;
				}
			} else {
				stddev = 0;
			}
			result->setAt(0, McoSql::RealValue::create(allocator, sum_of_products / current_sum));
			result->setAt(1, McoSql::RealValue::create(allocator, sum_of_products / 10000));
			result->setAt(2, McoSql::RealValue::create(allocator, sum_of_products));
			result->setAt(3, McoSql::RealValue::create(allocator, current_sum));
			result->setAt(4, McoSql::RealValue::create(allocator, current_count));
			result->setAt(5, McoSql::RealValue::create(allocator, stddev));
			result->setAt(6, McoSql::RealValue::create(allocator, sum_of_sq_products));
		}
		state = result;
	}
	return state;
}
