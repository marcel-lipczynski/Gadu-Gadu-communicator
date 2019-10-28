package login;

import javafx.fxml.FXML;
import javafx.fxml.FXMLLoader;
import javafx.scene.Parent;
import javafx.scene.Scene;
import javafx.scene.control.Button;
import javafx.stage.Stage;

public class LoginPageController {
    @FXML
    private Button loginButton;

    @FXML
    public void handleButtonClick() throws Exception{

            Stage primaryStage = (Stage)loginButton.getScene().getWindow();
            Parent root = FXMLLoader.load(getClass().getResource("../chat/chat.fxml"));

            primaryStage.setTitle("Simple chat");
            primaryStage.setScene(new Scene(root, 900, 550));
            primaryStage.setResizable(false);
            primaryStage.show();

    }
}
