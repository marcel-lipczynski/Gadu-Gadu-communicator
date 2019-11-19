package chat;

import java.io.DataOutputStream;
import java.io.IOException;
import java.io.PrintWriter;
import java.net.Socket;
import java.nio.ByteBuffer;
import java.nio.ByteOrder;

public class ChatClient {

    private Socket socket;
    private PrintWriter msgToServer2;
    private DataOutputStream msgToServer;
    private String name;


    public ChatClient(String serverAddress, int port, String name) {
        try {
            this.name = name;
            this.socket = new Socket(serverAddress, port);
            msgToServer2 = new PrintWriter(socket.getOutputStream(),true);
            msgToServer = new DataOutputStream(socket.getOutputStream());
            //            msgToServer.println(name); -> Message should be send after creating ne
//            w ChatClientThread
            // so we are sure that client will receive clients names
        } catch (IOException e) {
            System.out.println("Unable to create socket - no Server to connect: " + e.getMessage());
        }
    }

    public void sendMessage(String message) throws IOException {
//        message = message + "\n";
//        System.out.println("Message length: " + message.length());
//        msgToServer.println(message);
//        msgToServer.write(message, 0 , message.length());
//        msgToServer.flush();
//        msgToServer.writeBytes(message);
        msgToServer2.print(message);
        msgToServer2.flush();
    }

    public void sendLength(int length) throws IOException {
//        msgToServer.write(length);
//        msgToServer.flush();
        ByteBuffer buff = ByteBuffer.allocate(4);
        byte[] b = buff.order(ByteOrder.LITTLE_ENDIAN).putInt(length).array();
        msgToServer.write(b);
    }

    public Socket getSocket() {
        return socket;
    }

    public String getName() {
        return name;
    }

//    public PrintWriter getMsgToServer() {
//        return msgToServer;
//    }
}
