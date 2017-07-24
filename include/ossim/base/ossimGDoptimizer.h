/**
 * @author	Dylan Thomas Cannisi
 * @date	07/24/2017
 *
 * @brief	Simple Gradient Descent optimizer for ossim.
 * 
 *        See top level LICENSE.txt file.
 */
#ifndef OSSIM_GDOPTIMIZER_H
#define OSSIM_GDOPTIMIZER_H

#include <ossim/base/ossimConstants.h>
#include <ossim/base/ossimTerm.h>

#include <vector>



class OSSIMDLLEXPORT ossimGDoptimizer{
public:
	/**
	 * @brief      Public Copy constructor
	 */
	ossimGDoptimizer();
	/**
	 * @brief      Public Copy constructor
	 *
	 * @param[in]  src   The object to be copied (copies pointer not a copy of function)
	 */
	ossimGDoptimizer(const ossimGDoptimizer& src);
	/**
	 * @brief      Destroys the object !DOES NOT DELETE FUNCTION!
	 */
	~ossimGDoptimizer();
 

	/**
	 * @brief      Public Constructor with a preset function
	 *
	 * @param      function  The function to be optimized
	 */
	ossimGDoptimizer(ossimTerm* function);


	/**
	 * @brief      Sets the function to be optimized
	 *
	 * @param      function  The function to be optimized
	 *
	 * @return     whether or not the optimizer already contained a function
	 */
	bool setFunction(ossimTerm* function);
	
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
	std::vector<ossim_float64> optimize(std::vector<ossim_float64> startPoint, ossim_float64 gamma = 0.001, ossim_float64 precision = 0.0001, ossim_uint32 maxEpoch = 10000) const;

private:
	// Stores the multivariant function to be optimized.
	ossimTerm* m_func;

};



#endif // OSSIM_GDOPTIMIZER_H