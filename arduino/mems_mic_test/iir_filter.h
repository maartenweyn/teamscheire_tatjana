/*
 * Modified version of 
 *   https://github.com/tttapa/Filters/blob/master/src/IIRFilter.h
 * Part of:
 *   Arduino finite impulse response and infinite impulse response filter library 
 *   https://github.com/tttapa/Filters
 *   
 * Original code is licensed under GPL-3.0
 *
 * (c)2019 Ivan Kostoski
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *    
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <https://www.gnu.org/licenses/>.
 */

#include <stdint.h>

/******************************************************************************/

/******************************************************************************/

#define IIR_FILTER         IIRFilter<double, double>

#define INLINE            __attribute__((optimize("-O3"), always_inline)) inline


/******************************************************************************
 * IIR Filter implementation, direct form I, template
 ******************************************************************************/
template<typename AccuT, typename CoeffT>
class IIRFilter {
  public:  
    template <size_t B, size_t A>    
    IIRFilter(const double (&b)[B], const double (&_a)[A]) : lenB(B), lenA(A-1) {
      x = new CoeffT[lenB]();
      y = new CoeffT[lenA]();
      coeff_b = new CoeffT[2*lenB-1];
      coeff_a = new CoeffT[2*lenA-1];
      double a0 = -1 / _a[0];
      const double *a = &_a[1];
      for (uint8_t i = 0; i < 2*lenB-1; i++) coeff_b[i] = b[(2*lenB - 1 - i) % lenB] * a0;
      for (uint8_t i = 0; i < 2*lenA-1; i++) coeff_a[i] = a[(2*lenA - 2 - i) % lenA] * a0;
    }
    
    ~IIRFilter() {
      delete[] x;
      delete[] y;
      delete[] coeff_a;
      delete[] coeff_b;
    }

    INLINE CoeffT filter(CoeffT value) {
      AccuT filtered = 0;
      x[i_b] = value;
      CoeffT *b_shift = &coeff_b[lenB - i_b - 1];
      for(uint8_t i = 0; i < lenB; i++) filtered += x[i] * b_shift[i];
      CoeffT *a_shift = &coeff_a[lenA - i_a - 1];
      for(uint8_t i = 0; i < lenA; i++) filtered += y[i] *  a_shift[i];
      y[i_a] = filtered;
      i_b++;
      if(i_b == lenB) i_b = 0;
      i_a++;
      if(i_a == lenA) i_a = 0;
      return filtered;
    }
    
  protected:
    const uint8_t lenB, lenA;
    uint8_t i_b = 0, i_a = 0;
    CoeffT *x, *y, *coeff_b, *coeff_a;
};
