package plot;

/*
Copyright (c) 2007 Pascal S. de Kloe. All rights reserved.

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

import java.awt.*;
import java.awt.event.*;
import java.awt.image.*;
import java.io.*;
import java.util.prefs.*;
import javax.imageio.*;
import javax.swing.*;


/**
 @author Pascal S. de Kloe
 */
final class ExportListener implements MouseListener {

ExportListener(Graph source) {
	graph = source;
}


public void
mouseClicked(MouseEvent event) {
	int modifiers = event.getModifiers();
	if ((modifiers & InputEvent.BUTTON1_MASK) != 0) {
		final JComponent button = (JComponent) event.getSource();
		button.setEnabled(false);
		new Thread() {

			public void run() {
				try {
					showDialog(graph);
				} catch (Exception e) {
					e.printStackTrace();
				} finally {
					button.setEnabled(true);
				}
			}

		}.start();
	}
}


// Unused interface methods:
public void mousePressed(MouseEvent event) { }
public void mouseReleased(MouseEvent event) { }
public void mouseEntered(MouseEvent event) { }
public void mouseExited(MouseEvent event) { }


static void
showDialog(Graph graph) {
	JFileChooser chooser;
	try {
		String directory = getDefaultDirectory();
		chooser = new JFileChooser(directory);
		chooser.addChoosableFileFilter(PNG);
		chooser.setAcceptAllFileFilterUsed(false);

		int result = chooser.showSaveDialog(graph);
		if (result != JFileChooser.APPROVE_OPTION)
			return;
	} catch (SecurityException e) {
		String message = "The current security context does not allow access to local files.";
		String title = "Export dialog failed";
		JOptionPane.showMessageDialog(graph, message, title, JOptionPane.ERROR_MESSAGE);
		return;
	}

	File file = chooser.getSelectedFile();
	setDefaultDirectory(file);

	FileNameExtensionFilter filter = (FileNameExtensionFilter) chooser.getFileFilter();
	// Append extension if needed:
	if (! filter.accept(file))
		file = new File(file.getPath() + filter.getSuffix());

	try {
		if (file.exists() && ! file.canWrite()) {
			String message = "You do not have write permissions to this file.";
			String title = "Export failed";
			JOptionPane.showMessageDialog(graph, message, title, JOptionPane.ERROR_MESSAGE);
		}else {
			assert filter == PNG;
			writeAsPNG(graph, file);
		}
	} catch (SecurityException e) {
		String message = "A security manager prevented acces to the file.";
		String title = "Export failed";
		JOptionPane.showMessageDialog(graph, message, title, JOptionPane.ERROR_MESSAGE);
	} catch (IOException e) {
		String message = e.getMessage();
		String title = "Export failed";
		JOptionPane.showMessageDialog(graph, message, title, JOptionPane.ERROR_MESSAGE);
	}
}


private static String
getDefaultDirectory() {
	try {
		Preferences preferences = Preferences.userNodeForPackage(ExportListener.class);
		return preferences.get("previous_dir", (String) null);
	} catch (SecurityException e) {
		return null;
	}
}


private static void
setDefaultDirectory(File file) {
	try {
		Preferences preferences = Preferences.userNodeForPackage(ExportListener.class);
		if (! file.isDirectory())
			file = file.getParentFile();
		preferences.put("previous_dir", file.getAbsolutePath());
	} catch (SecurityException e) { }
}


private static void
writeAsPNG(Graph graph, File file)
throws IOException {
	BufferedImage buffer = new BufferedImage(graph.getWidth(), graph.getHeight(), BufferedImage.TYPE_INT_ARGB);
	Graphics2D graphics = buffer.createGraphics();
	graph.print(graphics);
	graphics.dispose();
	boolean writerAvailable = ImageIO.write(buffer, "PNG", file);
	if (! writerAvailable) {
		String message = "This Java runtime doesn't support PNG.";
		String title = "PNG export failed";
		JOptionPane.showMessageDialog(graph, message, title, JOptionPane.ERROR_MESSAGE);
	}
}

private static final FileNameExtensionFilter PNG = new FileNameExtensionFilter("Raster Image", "png");

private final Graph graph;

}
