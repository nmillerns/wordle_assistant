#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int findLetter(const char* word, const char letter) {
    for (size_t i = 0; i < strlen(word); ++i) {
        if (word[i] == letter) {
            return (int) i;
        }
    }
    return -1;
}

int containsLetter(const char* word, const char letter) {
    return findLetter(word, letter) >= 0;
}

int stringIsExclusiveLowerAlpha(char* str) {
    for (size_t i = 0; i < strlen(str); ++i) {
        if (str[i] < 'a' || str[i] > 'z') {
            return 0;
        }
    }
    return 1;
}

typedef char FiveLetterWord[5 + 1];  // extra null termination character
#define MAX_NUM_WORDS 10000

typedef struct {
    FiveLetterWord words[MAX_NUM_WORDS];
    size_t num_words;
} FiveLetterWordPool;

void appendWord(FiveLetterWordPool* word_pool, FiveLetterWord word) {
    strcpy(word_pool->words[word_pool->num_words], word);
    word_pool->num_words++;
}

void populateFiveLetterWords(FILE* dictionary_file, FiveLetterWordPool* five_letter_words_out) {
    five_letter_words_out->num_words = 0;
    const size_t BUFFER_SIZE = 2048;
    char buffer[BUFFER_SIZE];

    while(!feof(dictionary_file)) {
        fscanf(dictionary_file, "%s", buffer);
        if (5 == strlen(buffer) && stringIsExclusiveLowerAlpha(buffer)) {
            printf("Word %s\n", buffer);
            FiveLetterWord word;
            strcpy(word, buffer);
            appendWord(five_letter_words_out, word);
        }
    }
}

#define CLUE_CORRECT_POS_LETTER_GREEN 'G'
#define CLUE_CORRECT_LETTER_POS_WRONG_YELLOW 'Y'
#define CLUE_INCORRECT_LETTER_GRAY 'X'
typedef struct {
    FiveLetterWord guess;
    FiveLetterWord feedback;
} Clue;

int stringIsValidFeedback(const char* str) {
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

int respectsClue(const Clue* clue, FiveLetterWord word) {
    FiveLetterWord accounted_word;
    strcpy(accounted_word, word);
    for (size_t i = 0; i < 5; ++i) {
        if (clue->feedback[i] == CLUE_CORRECT_POS_LETTER_GREEN) {
            if (word[i] != clue->guess[i]) {
                return 0;
            } else {
                accounted_word[i] = ' ';
            }
        }
    }
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
    for (size_t i = 0; i < 5; ++i) {
        if (clue->feedback[i] == CLUE_INCORRECT_LETTER_GRAY
            && containsLetter(accounted_word, clue->guess[i])) {
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

    printf("Num of 5 letter words %lu\n", pool.num_words);

    Clue given_clues[10];
    size_t num_clues = 0;
    FiveLetterWord guess;
    strcpy(guess, "crane");  // Strategy is to start with "crane"

    while(1) {
        printf("GUESS:  %s\nRESULT> ", guess);
        Clue* next_clue = &given_clues[num_clues];
        strcpy(next_clue->guess, guess);
        char result[2048];
        if (scanf("%s", result) == 1 && stringIsValidFeedback(result)) {
            strcpy(next_clue->feedback, result);
            num_clues++;
        } else if (strlen(result) == 1 && result[0] == 'Q' || result[0] == 'q') {
            return 0;
        } else {
            printf("Invalid RESULT format\n");
            return 1;
        }
        FiveLetterWord valid_candidates[15];
        size_t num_valid_candidates = 0;
        for (size_t i = 0; i < pool.num_words && num_valid_candidates < 15; ++i) {
            int is_valid_candidate = 1;
            for (size_t j = 0; j < num_clues && is_valid_candidate; ++j) {
                if (!respectsClue(&given_clues[j], pool.words[i])) {
                    is_valid_candidate = 0;
                }
            }
            if (is_valid_candidate) {
                if (num_valid_candidates % 5 == 0) {
                    printf("\n");
                }
                strcpy(valid_candidates[num_valid_candidates], pool.words[i]);
                printf("%3lu: %s    ", num_valid_candidates, valid_candidates[num_valid_candidates]);
                num_valid_candidates++;
            }
        }
        printf("\nCHOOSE> ");
        int s = -1;
        if (scanf("%d", &s) == 1) {
            if (0 <= s && s < 15) {
                strcpy(guess, valid_candidates[s]);
            } else {
                printf("\nOriginal guess> ");
                scanf("%s", guess);
            }
        }
    }
    return 0;
}

