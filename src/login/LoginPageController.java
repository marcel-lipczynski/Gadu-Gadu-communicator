package login;

import chat.ChatPageController;
import javafx.fxml.FXML;
import javafx.fxml.FXMLLoader;
import javafx.scene.Parent;
import javafx.scene.Scene;
import javafx.scene.control.Button;
import javafx.scene.control.TextField;
import javafx.stage.Stage;

import javax.xml.bind.annotation.XmlList;

public class LoginPageController {


    private static LoginPageController instance;

    public LoginPageController() {
        instance = this;
    }

    @FXML
    private Button loginButton;

    @FXML
    private TextField address;

    @FXML
    private TextField port;

    @FXML
    private TextField username_id;

    @FXML
    public void handleButtonClick() throws Exception{

            Stage primaryStage = (Stage)loginButton.getScene().getWindow();
            Parent root = FXMLLoader.load(getClass().getResource("../chat/chat.fxml"));


            primaryStage.setTitle("Simple chat");
            primaryStage.setScene(new Scene(root, 900, 550));
            primaryStage.setResizable(false);

            ChatPageController.getInstance().getLoggedAs().setText("Logged as: " + username_id.getText());
            primaryStage.show();


    }

    public TextField getAddress() {
        return address;
    }

    public TextField getPort() {
        return port;
    }

    public TextField getUsername_id() {
        return username_id;
    }

    public static LoginPageController getInstance() {
        return instance;
    }
}
