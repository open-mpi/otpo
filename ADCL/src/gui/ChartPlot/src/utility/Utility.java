package utility;

import java.awt.Image;
import java.io.IOException;
import java.net.URL;

import javax.imageio.ImageIO;

public class Utility 
{
	static private Utility util = null;
	
	private Utility()
	{
		
	}
	
	static public Utility getInstance()
	{		
		if(util == null)
			util = new Utility();	
		return util;		
	}
	
	public int byteArrayToInt(byte[] b, int offset) 
    {
        int value = 0;
        for (int i = 0; i < 4; i++) {
            int shift = (4 - 1 - i) * 8;
            value += (b[3 - i + offset] & 0x000000FF) << shift;
        }
        return value;
    }
	
	public double byteArrayToDouble (byte[] arr, int offset) 
	{
		int i = 0;
		int len = 8;
		int cnt = 0;
		byte[] tmp = new byte[len];
		for (i = offset; i < (offset + len); i++) 
		{
			tmp[cnt] = arr[i];
			cnt++;
		}
		long accum = 0;
		i = 0;
		for ( int shiftBy = 0; shiftBy < 64; shiftBy += 8 ) {
			accum |= ( (long)( tmp[i] & 0xff ) ) << shiftBy;
			i++;
		}
		return Double.longBitsToDouble(accum);
	}
	
	public Image getImage(String location) 
	{
		try {
			URL url = getClass().getResource(location);
			if (url == null)
				throw new Error(location + " is missing");
			return ImageIO.read(url);
		} catch (IOException e) {
			throw new Error("IOException while retreiving " + location + ":\n\t" + e.getMessage());
		}
	}

}
