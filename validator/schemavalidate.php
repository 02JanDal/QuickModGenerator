<?php

require_once('Validator.php');

$json = json_decode(file_get_contents($argv[1]));

$validator = new Json\Validator($argv[2]);

try
{
    $validator->validate($json);
}
catch (Json\ValidationException $e)
{
    echo $e->getMessage();
    echo "\n";
}

?>
