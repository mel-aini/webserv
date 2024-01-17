<!DOCTYPE html>
<html lang="en">
<head>
    <meta charset="UTF-8">
    <meta name="viewport" content="width=device-width, initial-scale=1.0">
    <title>Random Color Generator</title>
</head>
<body style="background-color: <?php echo generateRandomColor(); ?>;">
    <h1 style="text-align: center; color: white;">Random Color Generator</h1>

    <p style="text-align: center; color: white;">
        The background color of this page is: <?php echo generateRandomColor(); ?>
    </p>

    <?php
    function generateRandomColor() {
        // Generate random values for RGB
        $red = mt_rand(0, 255);
        $green = mt_rand(0, 255);
        $blue = mt_rand(0, 255);

        $color = "rgb($red, $green, $blue)";

        return $color;
    }
    ?>
</body>
</html>