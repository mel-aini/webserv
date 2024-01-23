<?php
session_start();

// Check if the user is not logged in
if (!isset($_SESSION['user_id'])) {
    header("Location: login.php");
    exit();
}

// Check if the user_cookie is set
if (isset($_COOKIE['user_cookie'])) {
    $username = $_COOKIE['user_cookie'];
} else {
    $username = 'Guest';
}
?>

<!DOCTYPE html>
<html lang="en">
<head>
    <meta charset="UTF-8">
    <meta name="viewport" content="width=device-width, initial-scale=1.0">
    <title>WEBSERV Admin Page</title>
</head>
<body>
    <h1>Welcome to the WEBSERV Page, <?php echo $username; ?>!</h1>
    <p>This is a secure area.</p>
    <a href="logout.php">Logout</a>
</body>
</html>
