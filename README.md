# Connect-4-Project
Connect 4 game made using the HCS12 microcontroller

For this group project we were tasked with designing a final project to display our knowledge of the HCS12 microcontroller. My group made the game Connect 4 using a self-made LED matrix as the game board. To create the game we used the Dragon12-Light Rev D board. To play the user would simply press the column they would like to drop their piece into by using the keypad attached the board. The LCD was used to prompt the user.

My personal contribution to this project was using embedded C to program the game in software. I accomplished this by created my own unique solution to store the moves in software and then lighting the LEDs with the correct colors.

This project was made using the Freescale CodeWarrior integrated development environment (IDE). 

The Project.c file includes all of the game logic. This code stores the moves in software and writes to the correct ports on the Dragon12 board. The powerpoint and report includes specific details about the project.
