package plot;

/*
Copyright (c) 2006, 2007 Pascal S. de Kloe. All rights reserved.

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

import java.awt.Graphics;
import java.awt.geom.*;
import java.io.*;
import java.lang.reflect.*;
import java.math.BigDecimal;
import java.text.Format;
import java.util.*;
import javax.print.*;
import javax.print.attribute.*;
import javax.print.attribute.standard.*;
import javax.swing.*;
import org.w3c.dom.*;


/**
 * A collection of utility methods.
 * All methods are relative to the {@link Graph#render() current render}.
 @author Pascal S. de Kloe
 @since 1.2
 @see Graph#setDomain(GraphDomain)
 */
public final class RenderUtils {

private
RenderUtils() {
}


/**
 * Gets the x-coordinate from a graphical position.
 @since 1.2
 @see java.awt.event.MouseEvent#getPoint()
 */
public static BigDecimal
getXValue(Graph graph, Point2D position) {
	if (graph == null || position == null)
		throw new IllegalArgumentException();
	GraphInstance render = graph.getRender();
	if (render == null) return null;
	return render.getXValue((int) position.getX());
}


/**
 * Gets the y-coordinate from a graphical position.
 @since 1.2
 @see java.awt.event.MouseEvent#getPoint()
 */
public static BigDecimal
getYValue(Graph graph, Point2D position) {
	if (graph == null || position == null)
		throw new IllegalArgumentException();
	GraphInstance render = graph.getRender();
	if (render == null) return null;
	return render.getYValue((int) position.getY());
}


/**
 * Gets the formatted x-coordinate from a graphical position.
 @since 1.2
 */
public static String
getXFormatted(Graph graph, Point2D position) {
	BigDecimal value = getXValue(graph, position);
	if (value == null) return null;
	return graph.getXAxis().getFormat().format(value);
}


/**
 * Gets the formatted y-coordinate from a graphical position.
 @since 1.2
 */
public static String
getYFormatted(Graph graph, Point2D position) {
	BigDecimal value = getXValue(graph, position);
	if (value == null) return null;
	return graph.getXAxis().getFormat().format(value);
}


/**
 * Streams the graph to a SVG.
 * This method requires <a href="http://xmlgraphics.apache.org/batik/" rel="help">Apache Batik</a>.
 @since 1.5
 */
public static void
writeAsSVG(Graph graph, Writer target)
throws IOException {
	if (graph == null || target == null)
		throw new IllegalArgumentException();
	try {
		Class domClass = Class.forName("org.apache.batik.dom.GenericDOMImplementation");
		@SuppressWarnings("unchecked")
		Method buildDOM = domClass.getDeclaredMethod("getDOMImplementation");
		Object dom = buildDOM.invoke(null);

		@SuppressWarnings("unchecked")
		Method buildDocument = domClass.getDeclaredMethod("createDocument", String.class, String.class, DocumentType.class);
		Object document = buildDocument.invoke(dom, "http://www.w3.org/2000/svg", "svg", null);

		Class svgClass = Class.forName("org.apache.batik.svggen.SVGGraphics2D");
		@SuppressWarnings("unchecked")
		Constructor<Graphics> svgConstructor = svgClass.getDeclaredConstructor(Document.class);
		Graphics svg = svgConstructor.newInstance(document);

		graph.print(svg);
		@SuppressWarnings("unchecked")
		Method svgStream = svgClass.getDeclaredMethod("stream", Writer.class, boolean.class);
		boolean useCSS = true;
		svgStream.invoke(svg, target, useCSS);
		svg.dispose();
		target.flush();
	} catch (ClassNotFoundException e) {
		e.printStackTrace();
		String message = "This feature requires the Apache Batik library.";
		String title = "SVG stream failed";
		JOptionPane.showMessageDialog(graph, message, title, JOptionPane.ERROR_MESSAGE);
	} catch (NoSuchMethodException e) {
		e.printStackTrace();
		String message = "The Apache Batik library is incompatible.";
		String title = "SVG stream failed";
		JOptionPane.showMessageDialog(graph, message, title, JOptionPane.ERROR_MESSAGE);
        } catch (IOException e) {
		throw e;
	} catch (Exception e) {
		e.printStackTrace();
		String message = "Internal error.\n" + e.getMessage();
		String title = "SVG stream failed";
		JOptionPane.showMessageDialog(graph, message, title, JOptionPane.ERROR_MESSAGE);
	}
}


/**
 * Streams the coordinates as comma-separated values.
 * The first field is the argument and the following are all the values for each function. The functions appear in the same order as
 * they where {@link Graph#addFunction(Function, ChartStyle) added to the graph}.
 * This method requires <a href="http://quies.net/java/tech/csv/" rel="help">QN CSV</a>.
 @since 1.3
 */
public static void
writeAsCSV(Graph graph, Writer target)
throws IOException {
	if (graph == null || target == null)
		throw new IllegalArgumentException();
	GraphInstance render = graph.getRender();
	if (render == null) return;
	List<FunctionInstance> functions = render.getFunctionInstances();
	if (functions == null || functions.size() == 0) return;
	Axis xAxis = graph.getXAxis();
	Axis yAxis = graph.getYAxis();

	try {
		Class csvClass = Class.forName("net.quies.tech.csv.CSVWriter");
		@SuppressWarnings("unchecked")
		Constructor<Writer> csvConstructor = csvClass.getDeclaredConstructor(Writer.class);
		Writer csv = csvConstructor.newInstance(target);

		@SuppressWarnings("unchecked")
		Method writeHeader = csvClass.getDeclaredMethod("writeHeader", String[].class);
		writeCSVHeader(csv, writeHeader, xAxis.toString(), yAxis.toString(), functions);

		@SuppressWarnings("unchecked")
		Method writeRecord = csvClass.getDeclaredMethod("writeRecord", String[].class);
		try {
			writeRecord.setAccessible(true);
		} catch (SecurityException e) { }

		if (functions.size() == 1)
			writeCSVData(csv, writeRecord, xAxis.getFormat(), yAxis.getFormat(), functions.get(0));
		else
			writeCSVData(csv, writeRecord, xAxis.getFormat(), yAxis.getFormat(), functions);

		csv.flush();
	} catch (ClassNotFoundException e) {
		e.printStackTrace();
		String message = "This feature requires the QN CSV library.";
		String title = "CSV stream failed";
		JOptionPane.showMessageDialog(graph, message, title, JOptionPane.ERROR_MESSAGE);
        } catch (NoSuchMethodException e) {
		e.printStackTrace();
		String message = "The QN CSV library is incompatible.";
		String title = "CSV stream failed";
		JOptionPane.showMessageDialog(graph, message, title, JOptionPane.ERROR_MESSAGE);
        } catch (IOException e) {
		throw e;
	} catch (Exception e) {
		e.printStackTrace();
		String message = "Internal error.\n" + e.getMessage();
		String title = "CSV stream failed";
		JOptionPane.showMessageDialog(graph, message, title, JOptionPane.ERROR_MESSAGE);
	}
}


private static void
writeCSVHeader(Object csv, Method writeHeader, String xDescription, String yDescription, List<FunctionInstance> functions)
throws Exception {
	final int FUNCTIONS = functions.size();
	String[] name = new String[FUNCTIONS + 1];

	int i = 0;
	name[i] = xDescription;
	do {
		String fDescription = functions.get(i).toString();
		++i;
		if (fDescription.length() == 0)
			name[i] = yDescription;
		else if (yDescription.length() == 0)
			name[i] = fDescription;
		else {
			StringBuffer buffer = new StringBuffer();
			buffer.append(yDescription).append(" \u2015 ").append(fDescription);
			name[i] = buffer.toString();
		}
	} while (i < FUNCTIONS);

	writeHeader.invoke(csv, (Object) name);
}


private static void
writeCSVData(Object csv, Method writeRecord, Format xf, Format yf, FunctionInstance function)
throws Exception {
	BigDecimal[] xCoordinate = function.getXCoordinates();
	BigDecimal[] yCoordinate = function.getYCoordinates();
	int coordinates = xCoordinate.length;
	assert coordinates == yCoordinate.length;
	for (int i = 0; i < coordinates; ++i) {
		BigDecimal x = xCoordinate[i];
		BigDecimal y = yCoordinate[i];
		String argument = xf.format(x);
		String value = y == null ? "" : yf.format(y);
		String[] field = new String[] {argument, value};
		writeRecord.invoke(csv, (Object) field);
	}
}


private static void
writeCSVData(Object csv, Method writeRecord, Format xf, Format yf, List<FunctionInstance> functions)
throws Exception {
	// Collect all arguments:
	TreeSet<BigDecimal> arguments = new TreeSet<BigDecimal>();
	for (FunctionInstance f : functions)
		arguments.addAll(Arrays.asList(f.getXCoordinates()));

	int fields = functions.size() + 1;
	for (BigDecimal x : arguments) {
		String[] field = new String[fields];
		int i = 0;
		field[i] = xf.format(x);
		for (FunctionInstance f : functions) {
			int position = Arrays.binarySearch(f.getXCoordinates(), x);
			if (position >= 0) {
				BigDecimal y = functions.get(i).getYCoordinates()[position];
				field[++i] = y != null ? yf.format(y) : "\u2204";
			} else
				field[++i] = "";
		}
		assert i == field.length;
		writeRecord.invoke(csv, (Object) field);
	}
}


/**
 * Runs a dialog to print the graph.
 @return whether the graph was printed.
 @since 1.2
 */
public static boolean
print(Graph graph) {
	PrintRequestAttributeSet attributes = new HashPrintRequestAttributeSet();
	attributes.add(new JobName("Graph", java.util.Locale.ENGLISH));
	DocFlavor flavor = DocFlavor.SERVICE_FORMATTED.PRINTABLE;

	// Get the available services:
	PrintService[] availableService = PrintServiceLookup.lookupPrintServices(flavor, null);
	if (availableService.length == 0) {
		String message = "No capable print service found.";
		String title = "Print Service";
		JOptionPane.showMessageDialog(graph, message, title, JOptionPane.ERROR_MESSAGE);
		return false;
	}

	// Define the default service:
	PrintService defaultService = PrintServiceLookup.lookupDefaultPrintService();
	if (notInArray(defaultService, availableService))
		defaultService = availableService[0];

	// Show the dialog:
	PrintService service = ServiceUI.printDialog(graph.getGraphicsConfiguration()
			, graph.getLocationOnScreen().x, graph.getLocationOnScreen().y
			, availableService, defaultService, flavor, attributes);
	if (service == null)
		return false;

	// Print the graph:
	try {
		DocPrintJob job = service.createPrintJob();
		job.print(new SimpleDoc(graph, flavor, null), attributes);
	} catch (PrintException exception) {
		String message = exception.getMessage();
		String title = "Print job failed";
		JOptionPane.showMessageDialog(graph, message, title, JOptionPane.ERROR_MESSAGE);
		return false;
	}
	return true;
}


private static boolean
notInArray(PrintService key, PrintService[] array) {
	if (key == null)
		return false;
	int i = array.length;
	while (--i >= 0)
		if (array[i].equals(key))
			return false;
	return true;
}

}
