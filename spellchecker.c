#define _POSIX_C_SOURCE 200809L

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include <ctype.h>
#include <unistd.h>

#define MAX_FILENAME_LENGTH 256
#define MAX_WORD_LENGTH 50
#define MAX_MISSPELLED_WORDS 3
#define MAX_DICTIONARY_WORDS 110000

// Structure to hold spell-checking task information
typedef struct {
    char text_file[MAX_FILENAME_LENGTH];
    char dictionary_file[MAX_FILENAME_LENGTH];
    int num_errors;
    char most_misspelled_words[MAX_MISSPELLED_WORDS][MAX_WORD_LENGTH];
} SpellCheckTask;

//Structure to add or update the misspelled words
typedef struct {
    char word[MAX_WORD_LENGTH];
    int count;
} MisspelledWord;

// Global variables
int totalFilesProcessed = 0;
int totalSpellingErrors = 0;
char dictionary[MAX_DICTIONARY_WORDS][MAX_WORD_LENGTH];
int dictionarySize = 0;
MisspelledWord misspelledWords[MAX_MISSPELLED_WORDS];
int misspelledWordsCount = 0;
char outputFileName[MAX_FILENAME_LENGTH] = "mchowd07_A2.out";
char *commonMisspelledWords[3] = {"word1", "word2", "word3"};
int commonMisspelledWordsCounts[3] = {0};
pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;
int saveTOFile;
int running_threads = 0;
FILE *text_file;

// Function prototypes
void *spell_check_thread(void *arg);
void display_main_menu();
void start_spell_check_task();
void exit_program();
void save_result_to_file(SpellCheckTask *task);
void display_summary();
void toLowerCase(char* str);
void loadDictionary(const char* filename);
int isWordInDictionary(const char* word);


int main(int argc, char *argv[]) {
    char choice;


    for (int i = 1; i < argc; i++) {
        if (strcmp(argv[i], "-l") == 0) {
            //a flag so that it indicates whether to save it in a file or not
            saveTOFile = 1;
            // If "-l" is found, outputFileName is updated
            strcpy(outputFileName, "mchowd07_A2.out"); // Update with your desired file name
            break; // Exit the loop once "-l" is found and handled
        }
    }

    // Opening text file
    text_file = fopen("mchowd07_A2.out", "w");
    if (text_file == NULL) {
        perror("Error opening text file");
        exit(EXIT_FAILURE);
    }

    // Main menu loop
    do {
        display_main_menu();
        scanf(" %c", &choice);

        switch (choice) {
            case '1':
                start_spell_check_task();
                break;
            case '2':
                exit_program();
                break;
            default:
                printf("Invalid choice. Please try again.\n");
        }
    } while (choice != '2');

    // Closing text file
    fclose(text_file);

    return 0;
}

// Function to display the main menu
void display_main_menu(){
    printf("\nMain Menu\n");
    printf("1. Start a new spellchecking task\n");
    printf("2. Exit\n");
    printf("Enter your choice: ");
}

// Function to start a new spell-checking task
void start_spell_check_task(){
    char text_file[MAX_FILENAME_LENGTH];
    char dictionary_file[MAX_FILENAME_LENGTH];

    printf("Enter the name of the text file: ");
    scanf("%s", text_file);
    
    printf("\nEnter the name of the dictionary file: ");
    scanf("%s", dictionary_file);
    

    // Create a new spell-checking task
    SpellCheckTask *task = (SpellCheckTask *)malloc(sizeof(SpellCheckTask));
    if (task == NULL) {
        perror("Memory allocation failed");
        return;
    }
    strcpy(task->text_file, text_file);
    strcpy(task->dictionary_file, dictionary_file);

    // Creating a new thread for the spell-checking task
    pthread_t tid;
    int result = pthread_create(&tid, NULL, spell_check_thread, (void *)task);
    if (result != 0) {
        fprintf(stderr, "Error creating thread\n");
        free(task);
        return;
    }

    // Incrementing the running thread count
    pthread_mutex_lock(&mutex);
    running_threads++;
    pthread_mutex_unlock(&mutex);
}

//Function to make all the words lowercase
void toLowerCase(char* str){
    for (int i = 0; str[i]; i++) {
        str[i] = tolower(str[i]);
    }
}

//Function to load the dictionary in an array
void loadDictionary(const char* filename){
    FILE* file = fopen(filename, "r");
    if (!file) {
        printf("Failed to open dictionary file %s", filename);
        exit(EXIT_FAILURE);
    }

    while (fscanf(file, "%49s", dictionary[dictionarySize]) == 1 && dictionarySize < MAX_DICTIONARY_WORDS) {
        toLowerCase(dictionary[dictionarySize]);
        dictionarySize++;
    }

    fclose(file);
}

//Brute force mechanism to find out if the word is in dictionary
int isWordInDictionary(const char* word){
    for (int i = 0; i < dictionarySize; ++i) {
        if (strcmp(word, dictionary[i]) == 0) {
            return 1; // Word found in dictionary
        }
    }
    return 0; // Word not found
}
//Function to keep on updating misspelled words 
void addOrUpdateMisspelledWord(const char* word){
    for (int i = 0; i < misspelledWordsCount; i++) {
        if (strcmp(misspelledWords[i].word, word) == 0) {
            misspelledWords[i].count++;
            return;
        }
    }
    // New misspelled word
    if (misspelledWordsCount < MAX_MISSPELLED_WORDS) {
        strcpy(misspelledWords[misspelledWordsCount].word, word);
        misspelledWords[misspelledWordsCount].count = 1;
        misspelledWordsCount++;
    }
}

// Function to perform spell-checking
void *spell_check_thread(void *arg){
    SpellCheckTask *task = (SpellCheckTask *)arg;

    // Loading dictionary if not already loaded
    if (dictionarySize == 0) {
        loadDictionary(task->dictionary_file);
    }

    FILE *textFp = fopen(task->text_file, "r");
    if (!textFp) {
        perror("Error opening text file");
        return NULL;
    }

    char word[MAX_WORD_LENGTH];
    task->num_errors = 0; // Resetting error count

    while (fscanf(textFp, "%49s", word) != EOF) { //so that no buffer overflow occurs
        toLowerCase(word);
        if (!isWordInDictionary(word)) {
            task->num_errors++;
            addOrUpdateMisspelledWord(word);
        }
    }

    fclose(textFp);

    // Updating global counts and saving the result
    pthread_mutex_lock(&mutex);
    totalFilesProcessed++;
    totalSpellingErrors += task->num_errors;
    running_threads--;
    pthread_mutex_unlock(&mutex);

    save_result_to_file(task);

    free(task);
    pthread_exit(NULL);
}

// Function to handle program exit
void exit_program(){
    // Checking if there are running threads
    if (running_threads > 0) {
        printf("\nWaiting for %d thread(s) to finish...\n", running_threads);
        // Wait for running threads to finish
        while (running_threads > 0) {
            // Sleep for a short duration to avoid busy waiting
            sleep(10);
        }
    }
    if(!saveTOFile){
      display_summary();
    }
    // Closing text file
    fclose(text_file);

    exit(EXIT_SUCCESS);
}

// Function to save spell-checking task result to file
void save_result_to_file(SpellCheckTask *task){
     
    if(saveTOFile){
     // Acquiring lock before writing to file
     pthread_mutex_lock(&mutex);

     // Opening file for appending
     FILE *file = fopen(outputFileName, "a");
     if (file == NULL) {
         perror("Error opening summary file");
         pthread_mutex_unlock(&mutex);
         return;
     }

     // Writing task result to file
     fprintf(file, "%s %d %s %s %s\n", task->text_file, totalSpellingErrors,
             misspelledWords[0].word,misspelledWords[1].word,misspelledWords[2].word);

     // Closing file and releasing lock
     fclose(file);
     pthread_mutex_unlock(&mutex);
    }
}

// Function to display final summary
void display_summary(){
    printf("\nFinal Summary\n");
    printf("Number of files processed: %d\n", totalFilesProcessed);
    printf("Number of spelling errors: %d\n", totalSpellingErrors);
    // Displaying the top 3 most common misspellings
    for (int i = 0; i < 3 && i < misspelledWordsCount; i++) {
       printf("%s (%d times)\n", misspelledWords[i].word, misspelledWords[i].count);
    }
}
