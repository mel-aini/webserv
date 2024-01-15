<!-- <?php
session_start();

   $_SESSION["display_mode"] = "dark";
   $_SESSION["font_color"] = "white";
   echo "Session variables are set.<br><br />";
?> -->

<?php
if( $_GET["name"] && $_GET["age"] ) {
   $cookie_name = "user";
   $cookie_value = "name";
   setcookie($cookie_name, $cookie_value, time() + (86400 * 30), "/"); // 86400 = 1 day

   echo "print form values:<br />";
   echo "Welcome ". $_GET['name']. "<br />";
   echo "You are ". $_GET['age']. " years old. <br /><br />";


   // echo "print session values:<br />";
   // echo "Display mode is " . $_SESSION["display_mode"] . ".<br>";
   // echo "Font color is " . $_SESSION["font_color"] . ".<br><br />";
   

   echo "check cookies:<br />";
   if(!isset($_COOKIE[$cookie_name])) {
      echo "Cookie named '" . $cookie_name . "' is not set! <br />";
   } else {
      echo "Cookie '" . $cookie_name . "' is set!<br>";
      echo "Value is: " . $_COOKIE[$cookie_name] . "<br>";
   }
   
   // exit();
}
else
   echo "empty form <br />";
?>
