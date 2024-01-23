<?php
session_start();

// Destroy the session
session_destroy();

// Unset all session variables
$_SESSION = array();

// Delete the user_cookie
setcookie('user_cookie', '', time() - 3600, '/');

// Redirect to the login page
header("Location: login.php");
exit();
?>
