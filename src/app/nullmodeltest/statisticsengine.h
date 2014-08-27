/*
** Copyright (C) 2008 Peter J. Ersts (ersts at amnh.org)
** Creation Date: 2008-02-11
**
** This program is free software: you can redistribute it and/or modify
** it under the terms of the GNU General Public License as published by
** the Free Software Foundation, either version 3 of the License, or
** (at your option) any later version.
**
** This program is distributed in the hope that it will be useful,
** but WITHOUT ANY WARRANTY; without even the implied warranty of
** MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
** GNU General Public License for more details.
**
** You should have received a copy of the GNU General Public License
** along with this program.  If not, see <http://www.gnu.org/licenses/>.
**
** This work was partially funded by the National Aeronautics and Space Administration
** under award(s) NNX09AK19G
**
** This work was partially funded by the National Science Foundation under Grant No.DEB-0641023
**
** Any opinions, findings and conclusions or recommendations expressed in this material are those
** of the author(s) and do not necessarily reflect the views of the National Science Foundation (NSF)
*/
#ifndef STATISTICSENGINE_H
#define STATISTICSENGINE_H

#include <QtGlobal>

#include "cdf/gsl_cdf.h"

#include "boost/math/distributions/binomial.hpp"

using boost::math::binomial;


/*
 * A static class with some basic stat functions
 */
class StatisticsEngine
{

  public:
    /** \brief Returns the binomail probability for, inputs k, n, p -- at least k successes*/
    static double binomialProbability( int k, int n, double p );

    //at most k successess
    static double cumulativeBionomialProbability( int k, int n, double p);

    /** \brief Returns the exact binomail probability for, inputs k, n, p */
    static double exactBinomialProbability( int k, int n, double p );

};
#endif
