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
import java.text.*;


/**
 @author Pascal S. de Kloe
 @since 1.0
 */
public final class YAxis extends Axis {

/**
 @since 1.0
 */
public
YAxis() {
	this(getDefaultDescription());
}


/**
 @param description a short description.
 @since 1.0
 */
public
YAxis(String description) {
	this(description, getDefaultIndexScheme(), getDefaultFormat());
}


/**
 @param description a short description.
 @param format the format for the y-coordinates.
 @since 1.0
 */
public
YAxis(String description, IndexScheme scheme, Format format) {
	super(description, scheme, format, getDefaultLabelOffsetX(), getDefaultLabelOffsetY());
}


/**
 @return {@code "y"}.
 @since 1.0
 */
public static String
getDefaultDescription() {
	return "y";
}


/**
 @return a {@link LinearDecimalIndexScheme}.
 @since 1.0
 */
public static IndexScheme
getDefaultIndexScheme() {
	return new LinearDecimalIndexScheme();
}


/**
 @return a {@link java.text.NumberFormat#getNumberInstance() NumberFormat}.
 @since 1.0
 */
public static Format
getDefaultFormat() {
	return NumberFormat.getNumberInstance();
}


/**
 @return -7.
 @since 1.0
 @see Axis#setLabelOffsetX(float)
 */
public static float
getDefaultLabelOffsetX() {
	return -7f;
}


/**
 @return 0.
 @since 1.0
 @see Axis#setLabelOffsetY(float)
 */
public static float
getDefaultLabelOffsetY() {
	return 0f;
}


AxisInstance
getInstance(BigDecimal min, BigDecimal max, int length) 
{
	//BigDecimal max1 = new BigDecimal(20);
	return new YAxisInstance(this, min, max, length);
}

}
