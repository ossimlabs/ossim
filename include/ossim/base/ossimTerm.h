/**
 * @author	Dylan Thomas Cannisi
 * @date	07/24/2017
 *
 * @brief	Mathematical term data structure for ossim.
 * 
 *        See top level LICENSE.txt file.
 */
#ifndef OSSIM_FUNCTION_H
#define OSSIM_FUNCTION_H

#include <ossim/base/ossimConstants.h>

#include <vector>
#include <cmath>



/**
 * @brief      Different types for terms in a function
 */
enum termType : ossim_uint32 {
	monomial,
	polynomial,
	variable,
	constant,
	product
};



/**
 * An abstract class to containing a multivariable function.
 */
class OSSIMDLLEXPORT ossimTerm{
public:


	/**
	 * @brief      Virtual destructor for destroying tree from the top node.
	 */
	virtual ~ossimTerm() {}


	/**
	 * @brief      Evaluates a term at a given point PURE VIRTUAL
	 *
	 * @param[in]  point  The point to be evaluated
	 *
	 * @return     the value at a point
	 */
	virtual ossim_float64	evaluate(std::vector<ossim_float64> point) const = 0;
	/**
	 * @brief      Calculates the derivative of the term
	 *
	 * @param[in]  variable  The variable in which the derivative is with respect too
	 *
	 * @return     A pointer containing the derivative of the term.
	 */
	virtual ossimTerm*		derivative(const ossim_uint32& variable) const = 0;


	/**
	 * @brief      Gets the type
	 *
	 * @return     The type
	 */
	virtual ossim_uint32	getType() const = 0;
	/**
	 * @brief      Returns a pointer to a copy of the term
	 *
	 * @return     A pointer to a copy of the term
	 */
	virtual ossimTerm*		copy() const = 0;


	/**
	 * @brief      Evaluates the function for a point
	 *
	 * @param      point   The point at which the function is being evaluated
	 *
	 * @return     The value at a point
	 */
	inline ossim_float64	operator()(std::vector<ossim_float64> point) const{ return this->evaluate(point); }


	/**
	 * @brief      Returns a copy of the two terms in a polynomial
	 *
	 * @param      rhs   The right hand term of the polynomial
	 *
	 * @return     A polynomial with !COPIES! of the two terms
	 */
	inline ossimTerm*	operator+(ossimTerm* rhs) const;
	/**
	 * @brief      Returns a copy of the two terms in a product
	 *
	 * @param      rhs   The right hand term of the product
	 *
	 * @return     A product with !COPIES! of the two terms
	 */
	inline ossimTerm*	operator*(ossimTerm* rhs) const;

};



/**
 * A monomial class to hold a monomial as a term. Example (c_0 * x_0 ^ e_0)
 */
class OSSIMDLLEXPORT ossimMonom : public ossimTerm{
public:


	/**
	 * @brief     Public Constructor
	 *
	 * @param[in]  coeffient    The coeffient of the term
	 * @param      variable     The variable the variable number of the term
	 * @param[in]  exponential  The exponential of the term
	 */
	ossimMonom(const ossim_float64& coeffient, ossimTerm* variable, const ossim_float64& exponential) : coef(coeffient), var(variable), exp(exponential) {}
	/**
	 * @brief      Public Copy Constructor
	 *
	 * @param[in]  src   The source to be copied
	 */
	ossimMonom(const ossimMonom& src) : coef(src.coef), var(src.var->copy()), exp(src.exp) {}
	/**
	 * @brief      Destroys the monomial
	 */
	~ossimMonom() { delete var; }


	/**
	 * @brief      Evaluates a monomial for a point
	 *
	 * @param[in]  point  The point to be evaluated
	 *
	 * @return     The value at a point.
	 */
	ossim_float64	evaluate(std::vector<ossim_float64> point) const { return (coef * pow(var->evaluate(point), exp)); }
	/**
	 * @brief      Calculates the derivative for a monomial
	 *
	 * @param[in]  variable  The variable in which the derivative is with respect too.
	 *
	 * @return     A pointer to that contains the derivative of the term.
	 */
	ossimTerm*		derivative(const ossim_uint32& variable) const;


	/**
	 * @brief      Gets the type of a monomial
	 *
	 * @return     The type of a monomial
	 */
	ossim_uint32	getType() const { return termType::monomial; }
	/**
	 * @brief      Creates a copy of the monomial
	 *
	 * @return     A pointer to a copy of the monomial
	 */
	ossimTerm*		copy() const;

private:
	ossim_float64	coef;		// The coefficent of the monomial 
	ossimTerm*		var;		// The variable number of the monomial (starts at 0 - n)
	ossim_float64	exp;		// The exponenent of the monomial

};



/**
 * A polynomial class to hold a polynomial as a term.
 */
class OSSIMDLLEXPORT ossimPolynomial : public ossimTerm{
public:


	/**
	 * @brief      Public Constructor
	 *
	 * @param      leftTerm   The left term
	 * @param      rightTerm  The right term
	 */
	ossimPolynomial(ossimTerm* leftTerm, ossimTerm* rightTerm) : lTerm(leftTerm), rTerm(rightTerm) {}
	/**
	 * @brief      Public Copy Constructor
	 *
	 * @param[in]  src   The source to be copied
	 */
	ossimPolynomial(const ossimPolynomial& src) : lTerm(src.lTerm->copy()), rTerm(src.rTerm->copy()) {}
	/**
	 * @brief      Destroys the polynomial
	 */
	~ossimPolynomial() { delete lTerm; delete rTerm; }


	/**
	 * @brief      Evaluates a polynomial for a point
	 *
	 * @param[in]  point  The point to be evaluated
	 *
	 * @return     The value at a point.
	 */
	ossim_float64	evaluate(std::vector<ossim_float64> point) const { return lTerm->evaluate(point) + rTerm->evaluate(point); }
	/**
	 * @brief      Calculates the derivative for a polynomial
	 *
	 * @param[in]  variable  The variable in which the derivative is with respect too.
	 *
	 * @return     A pointer to that contains the derivative of the term.
	 */
	ossimTerm*		derivative(const ossim_uint32& variable) const;


	/**
	 * @brief      Gets the type of a polynomial
	 *
	 * @return     The type of a polynomial
	 */
	ossim_uint32	getType() const { return termType::polynomial; }
	/**
	 * @brief      Creates a copy of the polynomial
	 *
	 * @return     A pointer to a copy of the polynomial
	 */
	ossimTerm*		copy() const;

private:
	ossimTerm* lTerm;		// The left term of the polynomial
	ossimTerm* rTerm;		// The right term of the polynomial

};



/**
 * A polynomial class to hold a product as a term.
 */
class OSSIMDLLEXPORT ossimProd : public ossimTerm{
public:


	/**
	 * @brief      Public Constructor
	 *
	 * @param      leftTerm   The left term
	 * @param      rightTerm  The right term
	 */
	ossimProd(ossimTerm* leftTerm, ossimTerm* rightTerm) : lTerm(leftTerm), rTerm(rightTerm) {}
	/**
	 * @brief      Public Copy Constructor
	 *
	 * @param[in]  src   The source to be copied
	 */
	ossimProd(const ossimProd& src) : lTerm(src.lTerm->copy()), rTerm(src.rTerm->copy()) {}
	/**
	 * @brief      Destroys the product
	 */
	~ossimProd() { delete lTerm; delete rTerm; }


	/**
	 * @brief      Evaluates a product for a point
	 *
	 * @param[in]  point  The point to be evaluated
	 *
	 * @return     The value at a point.
	 */
	ossim_float64	evaluate(std::vector<ossim_float64> point) const { return lTerm->evaluate(point) * rTerm->evaluate(point); }
	/**
	 * @brief      Calculates the derivative for a product
	 *
	 * @param[in]  variable  The variable in which the derivative is with respect too.
	 *
	 * @return     A pointer to that contains the derivative of the term.
	 */
	ossimTerm*		derivative(const ossim_uint32& variable) const;


	/**
	 * @brief      Gets the type of a product
	 *
	 * @return     The type of a product
	 */
	ossim_uint32	getType() const { return termType::product; }
	/**
	 * @brief      Creates a copy of the product
	 *
	 * @return     A pointer to a copy of the product
	 */
	ossimTerm*		copy() const;

private:
	ossimTerm* lTerm;		// The left term of the product
	ossimTerm* rTerm;		// The right term of the product

};



/**
 * A variable class to hold a variable as a term.
 */
class OSSIMDLLEXPORT ossimVar : public ossimTerm{
public:


	/**
	 * @brief      Public Constructor
	 *
	 * @param[in]  variable  The variable number
	 */
	ossimVar(const ossim_uint32& variable) : var(variable) {}
	/**
	 * @brief      Public Copy Constructor
	 *
	 * @param[in]  src   The source to be copied
	 */
	ossimVar(const ossimVar& src) : var(src.var) {}
	/**
	 * @brief      Destroys the variable
	 */
	~ossimVar() {}


	/**
	 * @brief      Evaluates a variable for a point
	 *
	 * @param[in]  point  The point to be evaluated
	 *
	 * @return     The value at a point.
	 */
	ossim_float64	evaluate(std::vector<ossim_float64> point) const{ return point[var]; }
	/**
	 * @brief      Calculates the derivative for a variable
	 *
	 * @param[in]  variable  The variable in which the derivative is with respect too.
	 *
	 * @return     A pointer to that contains the derivative of the term.
	 */
	ossimTerm*		derivative(const ossim_uint32& variable) const;


	/**
	 * @brief      Gets the type of a variable
	 *
	 * @return     The type of a variable
	 */
	ossim_uint32	getType() const { return termType::variable; }
	/**
	 * @brief      Creates a copy of the variable
	 *
	 * @return     A pointer to a copy of the variable
	 */
	ossimTerm*		copy() const;

private:
	ossim_uint32 var;		// The number of the variable (0-n)

};



/**
 * A constant class to hold a constant as a term.
 */
class OSSIMDLLEXPORT ossimConst : public ossimTerm{
public:


	/**
	 * @brief      Public Constructor
	 *
	 * @param[in]  value  The value of the constant
	 */
	ossimConst(const ossim_float64& value) : val(value) {}
	/**
	 * @brief      Public Copy Constructor
	 *
	 * @param[in]  src   The source to be copied
	 */
	ossimConst(const ossimConst& src) : val(src.val) {}
	/**
	 * @brief      Destroys the constant
	 */
	~ossimConst() {}


	/**
	 * @brief      Evaluates a constant for a point
	 *
	 * @param[in]  point  The point to be evaluated
	 *
	 * @return     The value at a point.
	 */
	ossim_float64	evaluate(std::vector<ossim_float64> point) const{ return val; }
	/**
	 * @brief      Calculates the derivative for a constant
	 *
	 * @param[in]  variable  The variable in which the derivative is with respect too.
	 *
	 * @return     A pointer to that contains the derivative of the term.
	 */
	ossimTerm*		derivative(const ossim_uint32& variable) const;


	/**
	 * @brief      Gets the type of a constant
	 *
	 * @return     The type of a constant
	 */
	ossim_uint32	getType() const { return termType::constant; }
	/**
	 * @brief      Creates a copy of the constant
	 *
	 * @return     A pointer to a copy of the constant
	 */
	ossimTerm*		copy() const;

private:
	ossim_float64 val;	// The value of the constant

};



#endif // OSSIM_FUNCTION_H