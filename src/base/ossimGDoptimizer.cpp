/**
 * @author	Dylan Thomas Cannisi
 * @date	07/24/2017
 *
 * @brief	Simple Gradient Descent optimizer for ossim.
 *
 *		See top level LICENSE.txt file.
 */
#include <ossim/base/ossimGDoptimizer.h>

#include <cmath>
#include <iostream>

/**
 * @brief      Public Copy constructor
 */
ossimGDoptimizer::ossimGDoptimizer(){
	m_func = nullptr;
}


/**
 * @brief      Public Copy constructor
 *
 * @param[in]  src   The object to be copied (copies pointer not a copy of function)
 */
ossimGDoptimizer::ossimGDoptimizer(const ossimGDoptimizer& src){
	m_func = src.m_func;
}


/**
 * @brief      Destroys the object !DOES NOT DELETE FUNCTION!
 */
ossimGDoptimizer::~ossimGDoptimizer(){
}


/**
 * @brief      Public Constructor with a preset function
 *
 * @param      function  The function to be optimized
 */
ossimGDoptimizer::ossimGDoptimizer(ossimTerm* function){
	m_func = function;
}


/**
 * @brief      Sets the function to be optimized
 *
 * @param      function  The function to be optimized
 *
 * @return     whether or not the optimizer already contained a function
 */
bool ossimGDoptimizer::setFunction(ossimTerm* function){
	bool retVal = false;

	if(m_func != nullptr) retVal = true;
	m_func = function;

return retVal;
}


/**
 * @brief      Optimizes the function
 *
 * @param[in]  startPoint  The start point at which the algorithm starts its descent
 * @param[in]  gamma       Determins the step size as a function of the gradient
 * @param[in]  precision   The stopping point at which the algorithm will no longer descend
 * @param[in]  maxEpoch    The stopping point to limit infinite loops.
 *
 * @return     the optimial minima
 */
std::vector<ossim_float64> ossimGDoptimizer::optimize(std::vector<ossim_float64> point, ossim_float64 gamma, ossim_float64 precision, ossim_uint32 maxEpoch) const {

	ossim_uint32 iter = 0;
	ossim_float64 step = 10000;

	std::vector<ossimTerm*> derivative;
	for(size_t i = 0; i < point.size(); ++i){
		derivative.push_back(m_func->derivative(i));
	}

	std::vector<ossim_float64> prevPoint;
	while(step > precision && iter < maxEpoch){
		step = 0;
		prevPoint = point;
		for(size_t i = 0; i < point.size(); ++i){
			point[i] += -gamma * derivative[i]->evaluate(point);
			step += pow(point[i]-prevPoint[i], 2);
		}
		step = sqrt(step);
	}

	for(size_t i = 0; i < derivative.size(); ++i){
		delete derivative[i];
	}

return point;
}