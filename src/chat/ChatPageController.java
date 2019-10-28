package chat;

import javafx.fxml.FXML;
import javafx.scene.control.Button;
import javafx.scene.control.ListView;
import javafx.scene.control.SelectionMode;
import javafx.scene.control.TextArea;
import javafx.scene.input.KeyCode;
import javafx.scene.input.KeyEvent;


import java.io.IOException;
import java.util.ArrayList;
import java.util.List;


public class ChatPageController {

    private List<String> messages = new ArrayList<>();
    private List<String> users = new ArrayList<>();

    @FXML
    private ListView userList;

    @FXML
    private TextArea messageBox;

    @FXML
    private ListView chatPane;

    @FXML
    public void handleSendButtonClick() {
        String text = messageBox.getText();

        messages.add(text);

        chatPane.getItems().setAll(messages);
        chatPane.getSelectionModel().setSelectionMode(SelectionMode.SINGLE);



        messageBox.clear();
    }

    @FXML
    public void handleEnterKey(KeyEvent event) throws IOException {
        if (event.getCode() == KeyCode.ENTER) {
            handleSendButtonClick();
        }
    }
}
