package clients_logic;

import chat.ChatClient;
import chat.ChatClientThread;

import java.util.Scanner;

public class Main {

    public static void main(String[] args) {

        Scanner scanner = new Scanner(System.in);
        System.out.println("Enter server adress: ");
        String serverAddress = scanner.nextLine();
        System.out.println("Enter port number: ");
        int port = scanner.nextInt();
        System.out.println("Enter your name");
        String name = scanner.nextLine();

        ChatClient chatClient = new ChatClient(serverAddress,port, name);
        Thread messageReader = new Thread(new ChatClientThread(chatClient));
        messageReader.start();

        while(true){
            String message = scanner.nextLine();
            chatClient.sendMessage(message);

        }

    }
}
