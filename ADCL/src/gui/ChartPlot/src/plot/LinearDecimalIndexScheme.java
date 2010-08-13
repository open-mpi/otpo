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


/**
 * Makes stepsizes of 1, 2, 4 or 5 times a power of 10.
 @author Pascal S. de Kloe
 @since 1.0
 */
public final class LinearDecimalIndexScheme extends LinearIndexScheme {

/**
 @since 1.0
 */
public
LinearDecimalIndexScheme() {
}


/**
 @since 1.0
 */
protected BigDecimal
getStepsize(BigDecimal preferredStepsize) {
	return getDecimalStepsize(preferredStepsize);
}


static BigDecimal
getDecimalStepsize(BigDecimal x) {
	/* Get the leading digit position by using the combination of the scale and the precision.
	 * First make sure we have exactly the number of significant digits as the MathContext specifies.
	 * One way to do this is to add an insignificant small amount within the MathContext:
	 */
	x = x.add(x.movePointLeft(CONTEXT.getPrecision() + 1), CONTEXT);
	int scale = x.scale() - CONTEXT.getPrecision() + 1;
	BigDecimal leadingDigit = x.movePointRight(scale);
	assert leadingDigit.compareTo(BigDecimal.ONE) >= 0;
	assert leadingDigit.compareTo(BigDecimal.TEN) < 0;

	if (leadingDigit.compareTo(BETWEEN1AND2) < 0)
		return BigDecimal.valueOf(1L, scale);
	if (leadingDigit.compareTo(BETWEEN2AND4) < 0)
		return BigDecimal.valueOf(2L, scale);
	if (leadingDigit.compareTo(BETWEEN4AND5) < 0)
		return BigDecimal.valueOf(4L, scale);
	if (leadingDigit.compareTo(BETWEEN5AND10) < 0)
		return BigDecimal.valueOf(5L, scale);
	return BigDecimal.valueOf(10L, scale);
}


private final static MathContext CONTEXT = MathContext.DECIMAL32;
private final static BigDecimal BETWEEN1AND2 = BigDecimal.valueOf(4).divide(BigDecimal.valueOf(3), CONTEXT);
private final static BigDecimal BETWEEN2AND4 = BigDecimal.valueOf(16).divide(BigDecimal.valueOf(6), CONTEXT);
private final static BigDecimal BETWEEN4AND5 = BigDecimal.valueOf(40).divide(BigDecimal.valueOf(9), CONTEXT);
private final static BigDecimal BETWEEN5AND10 = BigDecimal.valueOf(100).divide(BigDecimal.valueOf(15), CONTEXT);

}
