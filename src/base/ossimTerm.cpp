/**
 * @author	Dylan Thomas Cannisi
 * @date	07/24/2017
 *
 * @brief	Mathematical term data structure for ossim.
 * 
 *        See top level LICENSE.txt file.
 */
#include <ossim/base/ossimTerm.h>


#include <iostream>


/**
 * 	OSSIM TERM
 */


/**
 * @brief      Returns a copy of the two terms in a polynomial
 *
 * @param      rhs   The right hand term of the polynomial
 *
 * @return     A polynomial with !COPIES! of the two terms
 */
inline ossimTerm* ossimTerm::operator+(ossimTerm* rhs) const{
	return new ossimPolynomial(this->copy(), rhs->copy());
}
/**
 * @brief      Returns a copy of the two terms in a product
 *
 * @param      rhs   The right hand term of the product
 *
 * @return     A product with !COPIES! of the two terms
 */
inline ossimTerm* ossimTerm::operator*(ossimTerm* rhs) const{
	return new ossimProd(this->copy(), rhs->copy());
}



/**
 * 	OSSIM MONOMIAL AS A TERM
 */


/**
 * @brief      Calculates the derivative for a monomial
 *
 * @param[in]  variable  The variable in which the derivative is with respect too.
 *
 * @return     A pointer to that contains the derivative of the term.
 */
ossimTerm* ossimMonom::derivative(const ossim_uint32& variable) const{
	return new ossimMonom(coef*exp, var->derivative(variable), exp-1);
}


/**
 * @brief      Creates a copy of the monomial
 *
 * @return     A pointer to a copy of the monomial
 */
ossimTerm* ossimMonom::copy() const{
	return new ossimMonom(coef, var->copy(), exp);
}



/**
 * 	OSSIM POLYNOMIAL AS A TERM
 */


/**
 * @brief      Calculates the derivative for a polynomial
 *
 * @param[in]  variable  The variable in which the derivative is with respect too.
 *
 * @return     A pointer to that contains the derivative of the term.
 */
ossimTerm* ossimPolynomial::derivative(const ossim_uint32& variable) const{
	return new ossimPolynomial(lTerm->derivative(variable), rTerm->derivative(variable));
}


/**
 * @brief      Creates a copy of the polynomial
 *
 * @return     A pointer to a copy of the polynomial
 */
ossimTerm* ossimPolynomial::copy() const{
	return new ossimPolynomial(lTerm->copy(), rTerm->copy());
}



/**
 * 	OSSIM PRODUCT AS A TERM
 */


/**
 * @brief      Calculates the derivative for a product
 *
 * @param[in]  variable  The variable in which the derivative is with respect too.
 *
 * @return     A pointer to that contains the derivative of the term.
 */
ossimTerm* ossimProd::derivative(const ossim_uint32& variable) const{
	return new ossimPolynomial(new ossimProd(lTerm->copy(), rTerm->derivative(variable)), new ossimProd(lTerm->derivative(variable), rTerm->copy()));
}


/**
 * @brief      Creates a copy of the product
 *
 * @return     A pointer to a copy of the product
 */
ossimTerm* ossimProd::copy() const{
	return new ossimProd(lTerm->copy(), rTerm->copy());
}



/**
 * 	OSSIM VARIABLE AS A TERM
 */


/**
 * @brief      Calculates the derivative for a variable
 *
 * @param[in]  variable  The variable in which the derivative is with respect too.
 *
 * @return     A pointer to that contains the derivative of the term.
 */
ossimTerm* ossimVar::derivative(const ossim_uint32& variable) const{
	if(var == variable) return new ossimVar(var);
return new ossimConst(0);
}


/**
 * @brief      Creates a copy of the variable
 *
 * @return     A pointer to a copy of the variable
 */
ossimTerm* ossimVar::copy() const{
	return new ossimVar(var);
}



/**
 * 	OSSIM CONSTANT AS A TERM
 */


/**
 * @brief      Calculates the derivative for a constant
 *
 * @param[in]  variable  The variable in which the derivative is with respect too.
 *
 * @return     A pointer to that contains the derivative of the term.
 */
ossimTerm* ossimConst::derivative(const ossim_uint32& /* variable */ ) const
{
   return new ossimConst(0);
}


/**
 * @brief      Creates a copy of the constant
 *
 * @return     A pointer to a copy of the constant
 */
ossimTerm* ossimConst::copy() const{
	return new ossimConst(val);
}
