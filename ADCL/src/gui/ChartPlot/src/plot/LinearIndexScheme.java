package plot;

/*
Copyright (c) 2005 Pascal S. de Kloe. All rights reserved.

Redistribution and use in source and binary forms, with or without modification,
are permitted provided that the following conditions are met:

1. Redistributions of source code must retain the above copyright notice, this
   list of conditions and the following disclaimer.
2. Redistributions in binary form must reproduce the above copyright notice,
   this list of conditions and the following disclaimer in the documentation
   and/or other materials provided with the distribution.
3. The name of the author may not be used to endorse or promote products derived
   from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE AUTHOR ``AS IS'' AND ANY EXPRESS OR IMPLIED
WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF
MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO
EVENT SHALL THE AUTHOR BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT
OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING
IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY
OF SUCH DAMAGE.
*/

import java.math.*;
import java.util.*;


/**
 * Subclasses of this class provide a linear axis index.
 @author Pascal S. de Kloe
 @since 1.0
 */
public abstract class LinearIndexScheme extends IndexScheme {

protected
LinearIndexScheme() {
}


final ArrayList<BigDecimal>
getIndexValues(BigDecimal min, BigDecimal max, int preferredSteps) {
	assert min.compareTo(max) < 0;
	assert preferredSteps >= 0 && preferredSteps < 100;

	BigDecimal range = max.subtract(min);
	assert range.signum() == 1;
	BigDecimal preferredStepsize = range.divide(BigDecimal.valueOf(Math.max(2, preferredSteps)), MathContext.DECIMAL64);
	assert preferredStepsize.signum() == 1;
	BigDecimal stepsize = getStepsize(preferredStepsize);
	assert stepsize.signum() == 1;

	ArrayList<BigDecimal> values = new ArrayList<BigDecimal>();
	BigDecimal x = floor(min, stepsize);
	values.add(x);
	if (preferredSteps < 2)
		values.add(ceil(max, stepsize));
	else {
		do {
			x = x.add(stepsize);
			values.add(x);
		} while (x.compareTo(max) < 0);
	}
	return values;
}


/**
 * Gets the the largest multiple of {@code stepsize} that is equal or less than {@code x}.
 */
private static BigDecimal
floor(BigDecimal x, BigDecimal stepsize) {
	BigDecimal steps = x.divide(stepsize, 0, BigDecimal.ROUND_FLOOR);
	return stepsize.multiply(steps);
}


/**
 * Gets the the smallest multiple of {@code stepsize} that is equal or more than {@code x}.
 */
private static BigDecimal
ceil(BigDecimal x, BigDecimal stepsize) {
	BigDecimal steps = x.divide(stepsize, 0, BigDecimal.ROUND_CEILING);
	return stepsize.multiply(steps);
}


/**
 * Gets a suitable stepsize which will produce a number of steps as close as possible tho the ideal with {@code preferredStepsize}.
 * The therm <dfn>suitable<dfn> is defined by the subclass of this class. If {@code preferredStepsize} is not a suitable stepsize
 * this method must decide wheather to use a smaller or a larger one with the following algorithm.
 * <p>
 * Let <var>a</var> be the largest suitable stepsize less than {@code preferredStepsize} and let <var>b</var> be the smallest
 * suitable stepsize greather than {@code preferredStepsize}.<br>
 * Let <var>r</var> be the range of the dimension which is the diference between the highest and the lowest value.
 * <p>
 * The break point <var>x</var> lies where <var>a</var> is equally wrong than <var>b</var> so <var>x</var> is where the number of
 * steps to many with <var>a</var> is equal to the number of steps short with <var>b</var>.
 * <pre>
 * r/a - r/x = r/x - r/b
 * 1/a + 1/b = 1/x + 1/x
 * b/ab + a/ab = 2/x
 * 2/x = (a+b)/ab
 * x = 2ab/(a+b)
 * </pre>
 * For example the break point between stepsize 1 and stepsize 2 is 4/3.
 @since 1.0
 */
protected abstract BigDecimal
getStepsize(BigDecimal preferredStepsize);

}
