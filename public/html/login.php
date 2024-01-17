<?php
session_start();

// Check if the user is already logged in
if (isset($_SESSION['user_id'])) {
    header("Location: admin.php");
    exit();
}

// Check if the form is submitted
if ($_SERVER['REQUEST_METHOD'] === 'POST') {
    // Simulated user credentials (you should validate against a database)
    $valid_username = 'admin';
    $valid_password = 'admin';

    // Get user input
    $username = isset($_POST['username']) ? $_POST['username'] : '';
    $password = isset($_POST['password']) ? $_POST['password'] : '';

    // Validate user input
    if ($username === $valid_username && $password === $valid_password) {
        // Set user_id in the session to indicate successful login
        $_SESSION['user_id'] = 1;

        // Set a cookie for user identification
        setcookie('user_cookie', $username, time() + 86400, "/"); // 86400 = 1 day

        // Redirect to the admin page
        header("Location: admin.php");
        exit();
    } else {
        $error_message = "Invalid username or password";
    }
}
?>

<!DOCTYPE html>
<html lang="en">
<head>
    <meta charset="UTF-8">
    <meta name="viewport" content="width=device-width, initial-scale=1.0">
    <title>WEBSERV LOGIN</title>
</head>
<body>
    <h1>WEBSERV LOGIN</h1>

    <?php
    if (isset($error_message)) {
        echo "<p style='color: red;'>$error_message</p>";
    }
    ?>

    <form action="login.php" method="post">
        <label for="username">Username:</label>
        <input type="text" id="username" name="username" required>
        <br>
        <label for="password">Password:</label>
        <input type="password" id="password" name="password" required>
        <br>
        <button type="submit">Login</button>
    </form>
</body>
</html>
