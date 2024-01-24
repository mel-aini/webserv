<!DOCTYPE html>
<html lang="en">
<head>
    <meta charset="UTF-8">
    <meta name="viewport" content="width=device-width, initial-scale=1.0">
    <title>Simple To-Do List</title>
</head>
<body>
    <h1>To-Do List</h1>

    <!-- Form to add new tasks -->
    <form action="tasks.php" method="post">
        <label for="task">Add a new task:</label>
        <input type="text" id="task" name="task" required>
        <button type="submit">Add Task</button>
    </form>

    <!-- Display existing tasks -->
    <h2>Tasks:</h2>
    <ul>
        <?php
        // Read tasks from the file and display them
        $tasks = file_exists('tasks.txt') ? file('tasks.txt', FILE_IGNORE_NEW_LINES | FILE_SKIP_EMPTY_LINES) : array();

        foreach ($tasks as $task) {
            echo "<li>$task <a href='tasks.php?delete=" . urlencode($task) . "'>Delete</a></li>";
        }
        ?>
    </ul>

</body>
</html>