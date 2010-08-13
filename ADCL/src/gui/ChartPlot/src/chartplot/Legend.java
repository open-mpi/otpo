package chartplot;

import java.awt.Color;
import java.awt.Font;
import java.awt.GridBagConstraints;
import java.awt.GridBagLayout;
import java.awt.Insets;

import javax.swing.JLabel;
import javax.swing.JPanel;

@SuppressWarnings("serial")
public class Legend extends JPanel
{
	JPanel panel = new JPanel();
	int counter = 0;
	
	public Legend()
	{
		setLayout(new GridBagLayout());
		panel.setBounds(0, 100, 180, 600);
		panel.setLayout(new GridBagLayout());
		this.setOpaque(false);
		GridBagConstraints c = new GridBagConstraints();
		c.fill = GridBagConstraints.HORIZONTAL;		
		c.gridx = 0;
		c.insets = new Insets(0,0,0,10);
		c.gridy = 0;		
		add(panel,c);
	}
	
	public void addToLegendList(Color color,String functionName)
	{
		GridBagConstraints c = new GridBagConstraints();
		//add the color box
		JPanel colorPanel = new JPanel();
		colorPanel.setBackground(color);
		c.fill = GridBagConstraints.HORIZONTAL;		
		c.gridx = 0;
		c.insets = new Insets(0,10,0,0);
		c.gridy = counter;
		panel.add(colorPanel,c);	
		
		//add the function name label
		JLabel functionNameLabel = new JLabel(functionName);
		Font font = new Font("",1,10);
		functionNameLabel.setFont(font);
		c.fill = GridBagConstraints.HORIZONTAL;		
		c.gridwidth = 3;
		c.insets = new Insets(0,0,0,0);
		c.gridx =1;
		c.gridy = counter;
		panel.add(functionNameLabel,c);
		counter++;
	}
}
