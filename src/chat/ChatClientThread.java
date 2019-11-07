package chat;

import chat.ChatClient;
import javafx.application.Platform;
import javafx.scene.control.SelectionMode;

import java.io.BufferedReader;
import java.io.IOException;
import java.io.InputStreamReader;

public class ChatClientThread implements Runnable {

    private ChatClient chatClient;
    private BufferedReader reader;
    private String receivedMessage = null;

    //zmienna ktora posluzy do pobrania listy uzytkownikow z serwera
    private boolean newlyConnected = true;

    public ChatClientThread(ChatClient chatClient) {
        this.chatClient = chatClient;
        try {
            reader = new BufferedReader(new InputStreamReader(chatClient.getSocket().getInputStream()));
        } catch (IOException e) {
            System.out.println("Unable to read from the socket: " + e.getMessage());
        }
    }

    @Override
    public void run() {
        while(true){
            read();
        }
    }

    private void read() {
        try {
            if(newlyConnected){
                while(!(receivedMessage = reader.readLine()).trim().equals("#")){
                    System.out.println("Received message: " + receivedMessage + " " +
                            "length: " + receivedMessage.trim().length());

                    ChatPageController.getInstance().getUsers().add(receivedMessage.trim());

                }
                System.out.println("I received stopReadingUsers character: " + receivedMessage );
                Platform.runLater(new Runnable() {
                    @Override
                    public void run() {
                        ChatPageController.getInstance().getUserList().getItems().setAll(ChatPageController.getInstance().getUsers());
                        ChatPageController.getInstance().getUserList().getSelectionModel().setSelectionMode(SelectionMode.SINGLE);
                        System.out.println("We did it");
                    }
                });
                newlyConnected = false;
            }


            if ((receivedMessage = reader.readLine()) != null) {
                if(receivedMessage.trim().equals("#")){
                    //If server sends "#" character, it means that next message to send is new Clients nickname;
                    ChatPageController.getInstance().getUsers().add(reader.readLine().trim());
                    Platform.runLater(new Runnable() {
                        @Override
                        public void run() {
                            ChatPageController.getInstance().getUserList().getItems().setAll(ChatPageController.getInstance().getUsers());
                            ChatPageController.getInstance().getUserList().getSelectionModel().setSelectionMode(SelectionMode.SINGLE);
                        }
                    });

                } else{
                    System.out.println(receivedMessage.trim());
                    ChatPageController.getInstance().getMessages().add(receivedMessage.trim());

                    Platform.runLater(new Runnable() {
                        @Override
                        public void run() {
                            ChatPageController.getInstance().setApropriateMessagesInWindow();
//                            ChatPageController.getInstance().getChatPane().getItems().setAll(ChatPageController.getInstance().getMessages());
//                            ChatPageController.getInstance().getChatPane().getSelectionModel().setSelectionMode(SelectionMode.SINGLE);

                        }
                    });
                }

            }

        } catch (IOException e) {
            System.out.println("Reading data from server went wrong: " + e.getMessage());
        }
    }


}

