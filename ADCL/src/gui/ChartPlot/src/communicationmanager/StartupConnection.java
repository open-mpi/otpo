package communicationmanager;

//~--- JDK imports ------------------------------------------------------------

import java.io.*;

import java.net.*;


public class StartupConnection {
	
    static int                   BNR_MAGIC = 918273;
    private ConfigurationModel   _model;
    private BufferedInputStream  bis;
    int                          bnr_magic;
    private BufferedOutputStream bos;
    private DataInputStream      dis;
    private DataOutputStream     dos;
    private Socket               fd;
    int                          firstnode;
    int                          msglength = 32;
    int                          rank;
    int                          size;

    public StartupConnection() {}

    public StartupConnection(ConfigurationModel model) {
        _model = model;
    }

    public void init(ServerSocket ssocket) {
        try {
            fd  = ssocket.accept();
            bos = new BufferedOutputStream(fd.getOutputStream());
            dos = new DataOutputStream(bos);
            bis = new BufferedInputStream(fd.getInputStream());
            dis = new DataInputStream(bis);
        } catch (IOException g) {
            _model.messageOccured(g + "\n");
        }
    }

    public int readmsglength() {
        try {

            msglength = dis.readInt();
        } catch (IOException e) {
            _model.messageOccured(e + "\n");
        }

        return (msglength);
    }

    public int readmsg(byte[] buf, int msglength, int offset) {
        int ret;
        int rest;

        rest = msglength;

        try {
            while (rest > 0) {
                ret  = bis.read(buf, offset, rest);
                rest -= ret;
            }
        } catch (IOException e) {
            _model.messageOccured(e + "\n");
        }

        return (msglength);
    }
    
    public Header readHeader() 
    {
    	Header head = null;
        byte msgbuf[] = new byte[msglength];
		readmsg(msgbuf, msglength, 0);
		head = new Header(msgbuf);
        return head;
    }

    public void sendmsg(byte[] buf, int length) {
        int offset = 0;

        try {
            dos.writeInt(length);
            bos.write(buf, offset, length);
            bos.flush();
        } catch (IOException e) {
            _model.messageOccured(e + "\n");
        }
    }
    
    public void disconnect()
    {
    	try 
    	{
			bos.flush();
			bos.close();
			bis.close();
			dos.flush();
			dos.close();
			dis.close();
			fd.close();
		} 
    	catch (IOException e)
		{
			e.printStackTrace();
		}
    }
}

