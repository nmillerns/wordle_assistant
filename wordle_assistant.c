#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef char FiveLetterWord[5 + 1];  // extra null termination character
#define MAX_NUM_WORDS 10000

#define CLUE_CORRECT_POS_LETTER_GREEN 'G'
#define CLUE_CORRECT_LETTER_POS_WRONG_YELLOW 'Y'
#define CLUE_INCORRECT_LETTER_GRAY 'X'

/**
 * Represents a clue given by Worlde website when a guess word is entered
 * Each letter has a corresponding response: either correct, correct but wrong position, or incorrect
 */
typedef struct {
    FiveLetterWord guess;
    FiveLetterWord feedback;
} Clue;

/**
 * Finds the first position of a given letter in a word
 * @param word
 * @param letter
 * @return the position of the first occurance of the letter. -1 if no such letter is found
 */
int findLetter(const char* word, const char letter) {
    for (size_t i = 0; i < strlen(word); ++i) {
        if (word[i] == letter) {
            return (int) i;
        }
    }
    return -1;
}

/**
 * Determine if a word contains a given letter
 * @param word
 * @param letter
 * @return 1 if letter appears in the word 0 otherwise
 */
int containsLetter(const char* word, const char letter) {
    return findLetter(word, letter) >= 0;
}

/**
 * Determine if a string contains only lower case alphabetic characters
 * @param str
 * @return 1 if all letters are lower alpha else 0
 */
int stringIsExclusiveLowerAlpha(char* str) {
    for (size_t i = 0; i < strlen(str); ++i) {
        if (str[i] < 'a' || str[i] > 'z') {
            return 0;
        }
    }
    return 1;
}

/**
 * A collection of five letter words
 */
typedef struct {
    FiveLetterWord words[MAX_NUM_WORDS];
    size_t num_words;
} FiveLetterWordPool;

/**
 * Appends a word to a FiveLetterWordPool
 */
void appendWord(FiveLetterWordPool* word_pool, FiveLetterWord word) {
    strcpy(word_pool->words[word_pool->num_words], word);
    word_pool->num_words++;
}

/**
 * Populates FiveLetterWordPool from words contained in a dictionary file
 * @param dictionary_file
 * @param five_letter_words_out
 */
void populateFiveLetterWords(FILE* dictionary_file, FiveLetterWordPool* five_letter_words_out) {
    five_letter_words_out->num_words = 0;
    const size_t BUFFER_SIZE = 2048;
    char buffer[BUFFER_SIZE];

    while(!feof(dictionary_file)) {
        fscanf(dictionary_file, "%s", buffer);
        if (5 == strlen(buffer) && stringIsExclusiveLowerAlpha(buffer)) {
            FiveLetterWord word;
            strcpy(word, buffer);
            appendWord(five_letter_words_out, word);
        }
    }
}

/**
 * Determine if a given string is a validly formatted clue
 * A valid clue is 5 letters long
 * where each letter is either G (correct), Y (correct but wrong position), or X (incorrect)
 * @param str
 * @return 1 if str is a valid clue else 0
 */
int isClueFormat(const char* str) {
    if (5 != strlen(str)) {
        return 0;
    }
    for (size_t i = 0; i < 5; ++i) {
        if (str[i] != CLUE_CORRECT_POS_LETTER_GREEN && str[i] != CLUE_CORRECT_LETTER_POS_WRONG_YELLOW && str[i] != CLUE_INCORRECT_LETTER_GRAY) {
            return 0;
        }
    }
    return 1;
}

/**
 * Determine if a given FiveLetterWord respects the constraints of the given clue
 * @param clue
 * @param word
 * @return 1 if word respects clue else 0
 */
int respectsClue(FiveLetterWord word, const Clue* clue) {
    FiveLetterWord accounted_word;
    strcpy(accounted_word, word);
    // Check that it agrees with all green boxes
    for (size_t i = 0; i < 5; ++i) {
        if (clue->feedback[i] == CLUE_CORRECT_POS_LETTER_GREEN) {
            if (word[i] != clue->guess[i]) {
                return 0;
            } else {
                accounted_word[i] = ' ';
            }
        }
    }

    // Check that all yellow boxes are accounted for
    for (size_t i = 0; i < 5; ++i) {
        if (clue->feedback[i] == CLUE_CORRECT_LETTER_POS_WRONG_YELLOW) {
            int loc = findLetter(accounted_word, clue->guess[i]);
            if (word[i] == clue->guess[i] || loc < 0) {
                return 0;
            } else {
                accounted_word[loc] = ' ';
            }
        }
    }

    // Check that no grey box incorrect letter is included
    for (size_t i = 0; i < 5; ++i) {
        if (clue->feedback[i] == CLUE_INCORRECT_LETTER_GRAY
            && containsLetter(accounted_word, clue->guess[i])) {
            return 0;
        }
    }

    return 1;
}

/**
 * Scan for clue from stdin
 * @param clue_out Pointer to Clue output variable. Clue.feedback will be populated
 * @return 1 if a valid clue is scanned 0 otherwise
 */
int scanForClueResult(Clue* clue_out) {
    char clue_text[2048];
    if (scanf("%s", clue_text) == 1) {
        strcpy(clue_out->feedback, clue_text);
        return isClueFormat(clue_out->feedback);
    }
    return 0;
}

/**
 * Checks if a word respects a set of n given clues
 * @param word word to check
 * @param clues a pointer to a set (array) of clues
 * @param num_clues number of clues in the set
 * @return
 */
int respectsAllClues(FiveLetterWord word, Clue* clues, int num_clues) {
    for (size_t j = 0; j < num_clues; ++j) {
        if (!respectsClue(word, &clues[j])) {
            return 0;
        }
    }
    return 1;
}

int main(int argc, char** argv) {
    FiveLetterWordPool pool;
    FILE* f = fopen("/etc/dictionaries-common/words", "r");
    populateFiveLetterWords(f, &pool);
    fclose(f);

    Clue given_clues[10];
    size_t num_clues = 0;
    FiveLetterWord guess;
    strcpy(guess, "slate");  // Strategy is to start with "slate"

    while(1) {
        Clue* next_clue = &given_clues[num_clues];
        strcpy(next_clue->guess, guess);

        printf("GUESS:  %s\nRESULT> ", next_clue->guess);
        if (scanForClueResult(next_clue)) {
            num_clues++;
        } else if (strcmp(next_clue->feedback, "q") == 0 || strcmp(next_clue->feedback, "Q") == 0) {
            printf("Bye\n");
            return 0;
        } else {
            printf("Invalid RESULT format\n");
            return 1;
        }

        FiveLetterWordPool valid_candidates;
        valid_candidates.num_words = 0;
        for (size_t i = 0; i < pool.num_words && valid_candidates.num_words < 100; ++i) {
            // If it respects all clues given so far, it is a valid word
            if (respectsAllClues(pool.words[i], given_clues, num_clues)) {
                appendWord(&valid_candidates, pool.words[i]);
            }
        }

        // Print out valid candidates with index
        for (size_t i = 0; i < valid_candidates.num_words; ++i) {
            if (i % 5 == 0) {
                printf("\n");
            }
            printf("%3lu: %s    ", i, valid_candidates.words[i]);
        }

        // The user selects a guess giving valid word by index
        printf("\nCHOOSE> ");
        int s = -1;
        if (scanf("%d", &s) == 1) {
            if (0 <= s && s < valid_candidates.num_words) {
                strcpy(guess, valid_candidates.words[s]);
            } else {
                printf("\nOriginal guess> ");
                scanf("%s", guess);
            }
        }
    }
    return 0;
}

