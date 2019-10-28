package com.company;

import java.io.IOException;
import java.io.PrintWriter;
import java.net.Socket;

public class ChatClient {

    private Socket socket;
    private PrintWriter msgToServer;

    public ChatClient(String serverAddress, int port) {
        try {
            this.socket = new Socket(serverAddress, port);
            msgToServer = new PrintWriter(socket.getOutputStream(),true);
        } catch (IOException e) {
            System.out.println("Unable to create socket - no Server to connect: " + e.getMessage());
        }
    }

    public void sendMessage(String message) {
        msgToServer.println(message);
    }

    public Socket getSocket() {
        return socket;
    }

    public PrintWriter getMsgToServer() {
        return msgToServer;
    }
}
