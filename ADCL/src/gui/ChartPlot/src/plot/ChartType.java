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
 * Defines the supported charts.
 @since 1.0
 @see ChartStyle#setType(ChartType)
 @author Pascal S. de Kloe
 */
public enum ChartType {

/**
 * A line chart with straight lines from coordinate to coordinate.
 @since 1.0
 */
LINE {

	public FunctionInstance
	getInstance(String description, BigDecimal[] argument, BigDecimal[] value, ChartStyle style) 
	{
		funcInst = new LineChartInstance(description, argument, value, style);
		return funcInst;
	}

},


/**
 * A line chart of a step function.
 @since 1.1
 */
STEP {

	public FunctionInstance
	getInstance(String description, BigDecimal[] argument, BigDecimal[] value, ChartStyle style)
	{
		funcInst = new StepChartInstance(description, argument, value, style);
		return funcInst;
	}

};



/**
 @param description a short description of the function.
 @param xCoordinate the arguments in ascending order.
 @param yCoordinate the corresponding values.
 */
public abstract FunctionInstance
getInstance(String description, BigDecimal[] xCoordinate, BigDecimal[] yCoordinate, ChartStyle style);
private static FunctionInstance funcInst;

public FunctionInstance getFunctionInstance()
{
	return funcInst;
}

}
