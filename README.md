This program is a multi-threaded spell-checking application written in C. It allows users to verify the spelling of words in a text file against a dictionary file and identifies and reports the most frequently misspelled words.

Features

Multi-threaded spell checking
Command-line argument support for output customization
Reports the total number of files processed, total spelling errors, and the three most common misspellings
Optionally saves the report to a file instead of displaying it on the terminal


Compilation
To compile the program, use the following command:
make

Usage

-l: Optional argument. If specified, the program saves the output to "mchowd07_A2.out" instead of displaying it on the terminal.

Operation
Starting the Program: Run the compiled executable from the makefile. Optionally, use the -l flag if you prefer to save the output to a file.

Using the Program:

The program displays a main menu with two options: start a new spellchecking task or exit.
To start a spellchecking task, choose the first option and then enter the names of the text file and the dictionary file when prompted.
The program then processes the text file in a separate thread, checks each word against the dictionary, and reports any spelling mistakes found.
Exiting the Program: Choose the exit option from the main menu. The program will then display a summary of all spellchecking tasks or save it to "mchowd07_A2.out" if -l was used.

Name: Mashrur Eusuf Chowdhury
