<?php
	// Process form submission to add a new task
	if ($_SERVER['REQUEST_METHOD'] === 'POST') {
		$newTask = trim($_POST['task']);
		if (!empty($newTask)) {
			// Append the new task to the file
			file_put_contents('tasks.txt', $newTask . PHP_EOL, FILE_APPEND);
			header('Location: index3.php');
			exit();
		}
	}

	// Process task deletion
	if (isset($_GET['delete'])) {
		$taskToDelete = urldecode($_GET['delete']);
		$tasks = file_exists('tasks.txt') ? file('tasks.txt', FILE_IGNORE_NEW_LINES | FILE_SKIP_EMPTY_LINES) : array();
		// Remove the task from the file
		$tasks = array_diff($tasks, array($taskToDelete));
		file_put_contents('tasks.txt', implode(PHP_EOL, $tasks));
		header('Location: index3.php');
		exit();
	}
?>