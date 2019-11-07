package chat;

import javafx.fxml.FXML;
import javafx.scene.control.ListView;
import javafx.scene.control.SelectionMode;
import javafx.scene.control.TextArea;
import javafx.scene.input.KeyCode;
import javafx.scene.input.KeyEvent;
import javafx.scene.text.Text;
import login.LoginPageController;


import java.io.IOException;
import java.util.ArrayList;
import java.util.List;


public class ChatPageController {

    private ChatClient chatClient ;
//            = new ChatClient(LoginPageController.getInstance().getAddress().getText(),
//            Integer.parseInt(LoginPageController.getInstance().getPort().getText()),
//            LoginPageController.getInstance().getUsername_id().getText());

    private Thread messageReader ;
//            = new Thread(new ChatClientThread(chatClient));



    private static ChatPageController instance;

    public ChatPageController() {
        instance = this;
        try{
            chatClient = new ChatClient(LoginPageController.getInstance().getAddress().getText(),
                    Integer.parseInt(LoginPageController.getInstance().getPort().getText()),
                    LoginPageController.getInstance().getUsername_id().getText());
            messageReader = new Thread(new ChatClientThread(chatClient));
            messageReader.start();
            chatClient.sendMessage(chatClient.getName());
        }catch(Exception e){
            System.out.println("Something went horribly wrong: " + e.getMessage());
        }

    }

    //whoToSend - zmienna ktora wskaze komu wyslac wiadomosc, ustawia sie po wybraniu uzytkownika z listy!
    private String whoToSend;
    private List<String> messagesToShowOnPane = new ArrayList<>();
    private List<String> messages = new ArrayList<>();
    private List<String> users = new ArrayList<>();

    @FXML
    private ListView userList;

    @FXML
    private Text loggedAs;

    @FXML
    private TextArea messageBox;

    @FXML
    private ListView chatPane;

    @FXML
    public void handleSendButtonClick() {

        String text = messageBox.getText();
        text = chatClient.getName() + ":" + whoToSend + ":" + text;
        messages.add(text);

//      Informujemy serwer do kogo ma zostać przekazana wiadomość
//        chatClient.sendMessage(whoToSend);
//
        chatPane.getItems().setAll(messages);
        chatPane.getSelectionModel().setSelectionMode(SelectionMode.SINGLE);
        chatClient.sendMessage(text);


        messageBox.clear();
    }

    @FXML
    public void handleClickListView(){
        //kiedy klikasz na jakiegos uzytkownika to czyscisz
        messageBox.clear();
        messagesToShowOnPane.clear();

        String[] splittedMessage;
        whoToSend = (String) userList.getSelectionModel().getSelectedItem();
        System.out.println(whoToSend);

        for (String message: messages){
            splittedMessage = message.split(":",3);
            if(splittedMessage[0].equals(chatClient.getName()) && splittedMessage[1].equals(whoToSend)){
                messagesToShowOnPane.add(splittedMessage[0] + ":" + splittedMessage[2]);

            }
            if(splittedMessage[0].equals(whoToSend) && splittedMessage[1].equals(chatClient.getName())){
                messagesToShowOnPane.add(splittedMessage[0] + ":" + splittedMessage[2]);
            }
        }
        chatPane.getItems().setAll(messagesToShowOnPane);
        chatPane.getSelectionModel().setSelectionMode(SelectionMode.SINGLE);
    }


    @FXML
    public void handleEnterKey(KeyEvent event){
        if (event.getCode() == KeyCode.ENTER) {
            handleSendButtonClick();
        }
    }

    public Text getLoggedAs() {
        return loggedAs;
    }

    public static ChatPageController getInstance() {
        return instance;
    }

    public List<String> getMessages() {
        return messages;
    }

    public ListView getChatPane() {
        return chatPane;
    }

    public ChatClient getChatClient() {
        return chatClient;
    }

    public List<String> getUsers() {
        return users;
    }

    public ListView getUserList() {
        return userList;
    }

    public String getWhoToSend() {
        return whoToSend;
    }

    public void setWhoToSend(String whoToSend) {
        this.whoToSend = whoToSend;
    }
}
