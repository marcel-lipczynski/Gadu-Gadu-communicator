package chat;

import java.io.IOException;
import java.io.PrintWriter;
import java.net.Socket;

public class ChatClient {

    private Socket socket;
    private PrintWriter msgToServer;
    private String name;

    public ChatClient(String serverAddress, int port, String name) {
        try {
            this.name = name;
            this.socket = new Socket(serverAddress, port);
            msgToServer = new PrintWriter(socket.getOutputStream(),true);
//            msgToServer.println(name); -> Message should be send after creating new ChatClientThread
            // so we are sure that client will receive clients names
        } catch (IOException e) {
            System.out.println("Unable to create socket - no Server to connect: " + e.getMessage());
        }
    }

    public void sendMessage(String message) {
//        message = message + "\n";
//        System.out.println("Message length: " + message.length());
//        msgToServer.println(message);
        msgToServer.write(message, 0 , message.length());
        msgToServer.flush();
    }

    public void sendLength(int length){
        msgToServer.write(length);
        msgToServer.flush();
    }

    public Socket getSocket() {
        return socket;
    }

    public String getName() {
        return name;
    }

    public PrintWriter getMsgToServer() {
        return msgToServer;
    }
}
