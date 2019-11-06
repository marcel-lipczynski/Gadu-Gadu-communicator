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

    //    private LoginPageController loginPageController;

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
        messages.add(text);
//        users.add(LoginPageController.getInstance().getUsername_id().getText());

        chatPane.getItems().setAll(LoginPageController.getInstance().getUsername_id().getText()+": "+messages);
        chatPane.getSelectionModel().setSelectionMode(SelectionMode.SINGLE);
        chatClient.sendMessage(text);

//        userList.getItems().setAll(users);
//        userList.getSelectionModel().setSelectionMode(SelectionMode.SINGLE);


        messageBox.clear();
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
}
