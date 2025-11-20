module com.example.cs3502filemanagementsystem {
    requires javafx.controls;
    requires javafx.fxml;


    opens com.example.cs3502filemanagementsystem to javafx.fxml;
    exports com.example.cs3502filemanagementsystem;
}