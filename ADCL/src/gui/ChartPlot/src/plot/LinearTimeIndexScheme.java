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
 * Makes stepsizes of 1, 2, 4, 5, 10, 15, 20 or 30 for seconds and minutes. Fractions of a second and hours will make a
 * {@link LinearDecimalIndexScheme decimal stepsize}.
 @author Pascal S. de Kloe
 @since 1.1
 */
public class LinearTimeIndexScheme extends LinearIndexScheme {

/**
 @since 1.1
 */
public
LinearTimeIndexScheme() {
}


/**
 @since 1.1
 */
protected BigDecimal
getStepsize(BigDecimal preferredStepsize) {
	BigDecimal seconds = preferredStepsize;
	if (seconds.compareTo(BigDecimal.valueOf(2L).divide(BigDecimal.valueOf(3L), CONTEXT)) < 0)
		return LinearDecimalIndexScheme.getDecimalStepsize(seconds);

	if (seconds.compareTo(BigDecimal.valueOf(40L)) < 0)
		return getSexagesimalStepsize(seconds);

	if (seconds.compareTo(BigDecimal.valueOf(2400L)) < 0) {
		BigDecimal minutes = seconds.divide(BigDecimal.valueOf(60L), CONTEXT);
		minutes = getSexagesimalStepsize(minutes);
		return minutes.multiply(BigDecimal.valueOf(60L));
	}

	BigDecimal hours = seconds.divide(BigDecimal.valueOf(3600L), CONTEXT);
	hours = LinearDecimalIndexScheme.getDecimalStepsize(hours);
	return hours.multiply(BigDecimal.valueOf(3600L));
}


static BigDecimal
getSexagesimalStepsize(BigDecimal x) {
	if (x.compareTo(BETWEEN1AND2)   < 0) return BigDecimal.valueOf( 1L);
	if (x.compareTo(BETWEEN2AND4)   < 0) return BigDecimal.valueOf( 2L);
	if (x.compareTo(BETWEEN4AND5)   < 0) return BigDecimal.valueOf( 4L);
	if (x.compareTo(BETWEEN5AND10)  < 0) return BigDecimal.valueOf( 5L);
	if (x.compareTo(BETWEEN10AND15) < 0) return BigDecimal.valueOf(10L);
	if (x.compareTo(BETWEEN15AND20) < 0) return BigDecimal.valueOf(15L);
	if (x.compareTo(BETWEEN20AND30) < 0) return BigDecimal.valueOf(20L);
	                                     return BigDecimal.valueOf(30L);
}


private final static MathContext CONTEXT = MathContext.DECIMAL32;
private final static BigDecimal BETWEEN1AND2 = BigDecimal.valueOf(4).divide(BigDecimal.valueOf(3), CONTEXT);
private final static BigDecimal BETWEEN2AND4 = BigDecimal.valueOf(16).divide(BigDecimal.valueOf(6), CONTEXT);
private final static BigDecimal BETWEEN4AND5 = BigDecimal.valueOf(40).divide(BigDecimal.valueOf(9), CONTEXT);
private final static BigDecimal BETWEEN5AND10 = BigDecimal.valueOf(100).divide(BigDecimal.valueOf(15), CONTEXT);
private final static BigDecimal BETWEEN10AND15 = BigDecimal.valueOf(300).divide(BigDecimal.valueOf(25), CONTEXT);
private final static BigDecimal BETWEEN15AND20 = BigDecimal.valueOf(600).divide(BigDecimal.valueOf(35), CONTEXT);
private final static BigDecimal BETWEEN20AND30 = BigDecimal.valueOf(1200).divide(BigDecimal.valueOf(50), CONTEXT);

}
