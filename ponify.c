/*
    This code is redistributed under MIT License, which means:
        - Do whatever you want
        - Please keep this notice and include the license file to your project
        - I provide no warranty
    Created by github.com/kyrylo-sovailo, k.sovailo@gmail.com
    Reinventing bicycles since 2020
*/

#define N_RESULTS 10

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdbool.h>

#define WAY_DOWN 1
#define WAY_RIGHT 2
#define WAY_DIAGONAL 4
#define WAY_RIGHT_DIAGONAL 8
#define WAY_DOWN_DIAGONAL 16
#define WAY_DOUBLE_DOWN 32
#define WAY_DOUBLE_RIGHT 64
#define WAY_DOUBLE_DIAGONAL 128

#define DIST_VOWEL_VOWEL 0.75f
#define DIST_VOWEL_CONSONANT 1.25f
#define DIST_CONSONANT_CONSONANT 0.5f
#define DIST_CHEAP_BEGIN_END 0.1f
#define DIST_SIMMILAR_VOWEL 0.25f
#define DIST_SIMMILAR_CONSONANT 0.25f
#define DIST_RIGHT 1.0f
#define DIST_DOWN 1.0f

struct Task
{
    float distance;
    unsigned int way;
};

struct Result
{
    char *ponified, *ponyword;
    float distance;
};

struct PonifyInfo
{
    //The table of complexities of tasks of transforming [i] symbols of "ponyword" into [j] symbols of "word"
    struct Task tasks[33][33];
    const char *word;
    const char *ponyword;
    unsigned int wordlen;
    unsigned int ponywordlen;
    unsigned int cheapbegin;
};

//Transforms string to lower case and returns true if string is valid (a..z, A..Z)
bool lowercase_check(char *s)
{
    unsigned int len = strlen(s);
    if (len >= 32) return false;
    for (unsigned int i = 0; i < len; i++)
    {
        if ((*s >= 'a') && (*s <= 'z')) {}
        else if ((*s >= 'A') && (*s <= 'Z')) *s = *s + 'a' - 'A';
        else return false;
    }
    return true;
}

//Returns true if char is vowel
bool vowel(char c)
{
    return ((c == 'a') || (c == 'e') || (c == 'i') || (c == 'o') || (c == 'u') || (c == 'y'));
}

//Returns true if two characters need to be understood as one sound
unsigned int composite(const char c[2])
{
    return ((c[0] == c[1])
        || ((c[0] == 'c') && (c[1] == 'k'))
        || ((c[0] == 'c') && (c[1] == 'h'))
        || ((c[0] == 't') && (c[1] == 'h'))
        || ((c[0] == 'g') && (c[1] == 'h'))
        || ((c[0] == 'p') && (c[1] == 'h')));
}

#define RENEWMIN(NEWWAY) if (dist < mindist) { mindist = dist; minway = NEWWAY; } \
                    else if (dist == mindist) minway |= NEWWAY;

//Heuristic char-char similarity compare
float char_compare(char c1, char c2)
{
    if (c1 == c2) return 0.0f;
    
    if (c1 > c2)
    {
        char b = c1;
        c1 = c2;
        c2 = b;
    }

    unsigned int vow = vowel(c1);
    if (vow == vowel(c2))
    {
        if (vow) return DIST_VOWEL_VOWEL;
        else
            if(((c1 == 'b') && (c2 == 'p'))
            || ((c1 == 'd') && (c2 == 't'))
            || ((c1 == 'c') && (c2 == 'k'))
            || ((c1 == 'f') && (c2 == 'w'))
            || ((c1 == 'f') && (c2 == 'v'))
            || ((c1 == 'g') && (c2 == 'k'))
            || ((c1 == 'h') && (c2 == 'k'))
            || ((c1 == 'k') && (c2 == 'q'))
            || ((c1 == 'l') && (c2 == 'r'))
            || ((c1 == 'm') && (c2 == 'n'))
            || ((c1 == 's') && (c2 == 'z'))
            || ((c1 == 'v') && (c2 == 'w'))) return DIST_SIMMILAR_CONSONANT;
        else return DIST_CONSONANT_CONSONANT;
    }
    else return DIST_VOWEL_CONSONANT;
}

//Heuristic char-compositee similarity compare
float char_composite_compare(char c1, const char c2[2])
{
    unsigned int vow = vowel(c1);
    if (vow == vowel(c2[1]))
    {
        if (vow)
        {
            if ((c2[0] == 'o') && (c2[1] == 'o') && (c1 == 'u')) return DIST_SIMMILAR_VOWEL;
            else return DIST_VOWEL_VOWEL;
        }
        else
        {
            if(((c2[0] == 'c') && (c2[1] == 'k') && (c1 == 'c'))
            || ((c2[0] == 'c') && (c2[1] == 'k') && (c1 == 'h'))
            || ((c2[0] == 'c') && (c2[1] == 'k') && (c1 == 'k'))
            || ((c2[0] == 't') && (c2[1] == 'h') && (c1 == 's'))
            || ((c2[0] == 't') && (c2[1] == 'h') && (c1 == 'z'))
            || ((c2[0] == 'g') && (c2[1] == 'h') && (c1 == 'g'))) return DIST_SIMMILAR_CONSONANT;
            else return DIST_CONSONANT_CONSONANT;
        }
    }
    else return DIST_VOWEL_CONSONANT;
}

//Heuristic composite-composite similarity compare
float composite_compare(const char c1[2], const char c2[2])
{
    if ((c1[0] == c2[0]) && (c1[1] == c2[1])) return 0.0f;

    unsigned int vow = vowel(c1[1]);
    if (vow == vowel(c2[1]))
    {
        if (vow) return DIST_VOWEL_VOWEL;
        else return DIST_CONSONANT_CONSONANT;
    }
    else return DIST_VOWEL_CONSONANT;
}

//Determines [i][j] element of tasks table
float task_distance(const struct PonifyInfo *inf, unsigned int i, unsigned int j, unsigned int *way)
{
    float mindist = 10000.0f;
    float dist;
    unsigned int minway;
    unsigned int inverse = 0;

    char c1[2];
    c1[0] = (i > 1) ? inf->ponyword[i - 2] : 0;
    c1[1] = inf->ponyword[i - 1];
        
    char c2[2];
    c2[0] = (j > 1) ? inf->word[j - 2] : 0;
    c2[1] = inf->word[j - 1];

    //classic:
    //WAY_RIGHT team
    if ((i == inf->ponywordlen) && (inf->cheapbegin == 0)) dist = inf->tasks[i][j - 1].distance + DIST_CHEAP_BEGIN_END;
    else dist = inf->tasks[i][j - 1].distance + DIST_RIGHT;
    RENEWMIN(WAY_RIGHT)

    //WAY_DOWN team
    dist = inf->tasks[i - 1][j].distance + DIST_DOWN;
    RENEWMIN(WAY_DOWN)

    //WAY_DIAGONAL team
    dist = inf->tasks[i - 1][j - 1].distance + char_compare(c1[1], c2[1]);
    RENEWMIN(WAY_DIAGONAL)

    //extensions:
    //WAY_DOUBLE_RIGHT team
    if (composite(c2))
    {
        if ((i == inf->ponywordlen) && (inf->cheapbegin == 0)) dist = inf->tasks[i][j - 2].distance + DIST_CHEAP_BEGIN_END;
        else dist = inf->tasks[i][j - 2].distance + DIST_RIGHT;
        RENEWMIN(WAY_DOUBLE_RIGHT)
    }

    //WAY_DOUBLE_DOWN team
    if (composite(c1))
    {
        dist = inf->tasks[i - 2][j].distance + DIST_DOWN;
        RENEWMIN(WAY_DOUBLE_DOWN)
    }

    //WAY_DOWN_DIAGONAL team
    if (composite(c1))
    {
        dist = inf->tasks[i - 2][j - 1].distance + char_composite_compare(c2[1], c1);
        RENEWMIN(WAY_DOWN_DIAGONAL)
    }
        
    //WAY_RIGHT_DIAGONAL team
    if (composite(c2))
    {
        dist = inf->tasks[i - 1][j - 2].distance + char_composite_compare(c1[1], c2);
        RENEWMIN(WAY_RIGHT_DIAGONAL)
    }

    //WAY_DOUBLE_DIAGONAL team
    if (composite(c1) && composite(c2))
    {
        dist = inf->tasks[i - 2][j - 2].distance + composite_compare(c1, c2);
        RENEWMIN(WAY_DOUBLE_DIAGONAL)
    }

    *way = minway;
    return mindist;
}

//Ponifies one word
float ponify_word(const char word[32], const char ponyword[32], char ponified[32], unsigned int cheapbegin)
{
    //Some very weird modification of Lewenstein algorithm with magical numbers

    //Argument to be passed to comparison function
    struct PonifyInfo inf;
    inf.ponyword = ponyword;
    inf.word = word;
    inf.wordlen = strlen(word);
    inf.ponywordlen = strlen(ponyword);
    inf.cheapbegin = cheapbegin;

    //Transformation of 0 symbols to 0 symbols costs zero
    inf.tasks[0][0].distance = 0.0f;
    inf.tasks[0][0].way = 0;

    //Transformation of word to 0 symbols (first row)
    char c[2] = { 0,0 };
    for (unsigned int i = 1; i <= inf.wordlen; i++)
    {
        c[1] = word[i - 1];
        if (composite(c))
        {
            inf.tasks[0][i].distance = inf.tasks[0][i - 2].distance + (cheapbegin ? DIST_CHEAP_BEGIN_END : DIST_RIGHT);
            inf.tasks[0][i].way = WAY_DOUBLE_RIGHT;
        }
        else
        {
            inf.tasks[0][i].distance = inf.tasks[0][i - 1].distance + (cheapbegin ? DIST_CHEAP_BEGIN_END : DIST_RIGHT);
            inf.tasks[0][i].way = WAY_RIGHT;
        }
        c[0] = c[1];
    }

    //Transformation of ponyword to 0 symbols (first column)
    c[0] = 0; c[1] = 0;
    for (unsigned int i = 1; i <= inf.ponywordlen; i++)
    {
        c[1] = ponyword[i - 1];
        if (composite(c))
        {
            inf.tasks[i][0].distance = inf.tasks[i - 2][0].distance + DIST_DOWN;
            inf.tasks[i][0].way = WAY_DOUBLE_DOWN;
        }
        else
        {
            inf.tasks[i][0].distance = inf.tasks[i - 1][0].distance + DIST_DOWN;
            inf.tasks[i][0].way = WAY_DOWN;
        }
        c[0] = c[1];
    }

    //Main job
    for (unsigned int i = 1; i <= inf.ponywordlen; i++)
    {
        for (unsigned int j = 1; j <= inf.wordlen; j++)
        { 
            inf.tasks[i][j].distance = task_distance(&inf, i, j, &inf.tasks[i][j].way);
        }
    }

    #ifdef DEBUGINFO
        printf("             ");
        for (unsigned int i = 0; i <= inf.wordlen; i++) printf("%c           ", (i == 0) ? '0' : word[i - 1]);
        printf("\n");

        for (unsigned int i = 0; i <= inf.ponywordlen; i++)
        {
            printf("%c            ", (i == 0) ? '0' : ponyword[i - 1]);
            for (unsigned int j = 0; j <= inf.wordlen; j++)
            {
                printf("%2.2f", inf.tasks[i][j].distance);
                unsigned int spaces = 8;
                if ((inf.tasks[i][j].way & WAY_DOUBLE_DOWN)     != 0) { printf("D"); spaces--; }
                if ((inf.tasks[i][j].way & WAY_DOWN)            != 0) { printf("d"); spaces--; }
                if ((inf.tasks[i][j].way & WAY_DOWN_DIAGONAL)   != 0) { printf("u"); spaces--; }
                if ((inf.tasks[i][j].way & WAY_DOUBLE_DIAGONAL) != 0) { printf("Q"); spaces--; }
                if ((inf.tasks[i][j].way & WAY_DIAGONAL)        != 0) { printf("q"); spaces--; }
                if ((inf.tasks[i][j].way & WAY_RIGHT_DIAGONAL)  != 0) { printf("l"); spaces--; }
                if ((inf.tasks[i][j].way & WAY_RIGHT)           != 0) { printf("r"); spaces--; }
                if ((inf.tasks[i][j].way & WAY_DOUBLE_RIGHT)    != 0) { printf("R"); spaces--; }

                while (spaces > 0) { printf(" "); spaces--; }
            }
            printf("\n");
        }
    #endif

    unsigned int i = inf.ponywordlen;
    unsigned int j = inf.wordlen;
    
    while (((inf.tasks[i][j].way & WAY_RIGHT) != 0) || ((inf.tasks[i][j].way & WAY_DOUBLE_RIGHT) != 0)) j--;
    unsigned int fromend = inf.wordlen - j;

    while (i != 0)
    {
            if ((inf.tasks[i][j].way & WAY_DOUBLE_DOWN)     != 0)   { i -= 2;           }
        else if((inf.tasks[i][j].way & WAY_DOWN)            != 0)   { i--;              }
        else if((inf.tasks[i][j].way & WAY_DOWN_DIAGONAL)   != 0)   { i -= 2;   j--;    }
        else if((inf.tasks[i][j].way & WAY_DOUBLE_DIAGONAL) != 0)   { i -= 2;   j -= 2; }
        else if((inf.tasks[i][j].way & WAY_DIAGONAL)        != 0)   { i--;      j--;    }
        else if((inf.tasks[i][j].way & WAY_RIGHT_DIAGONAL)  != 0)   { i--;      j -= 2; }
        else if((inf.tasks[i][j].way & WAY_RIGHT)           != 0)   {           j--;    }
        else { j -= 2; }
    }
    unsigned int frombegin = j;

    memcpy(ponified, word, frombegin * sizeof(char));
    memcpy(ponified + frombegin, ponyword, inf.ponywordlen * sizeof(char));
    memcpy(ponified + frombegin + inf.ponywordlen, word + inf.wordlen - fromend, fromend * sizeof(char));
    ponified[frombegin + inf.ponywordlen + fromend] = '\0';

    return inf.tasks[inf.ponywordlen][inf.wordlen].distance;
};

//Add result to table of best results
void add_to_results(unsigned int *count, float dist, struct Result results[N_RESULTS], const char *ponified, const char *ponyword)
{
    unsigned int newi = *count;
    while ((newi > 0) && (results[newi - 1].distance > dist))
    {
        if (newi == N_RESULTS) { free(results[N_RESULTS - 1].ponified); free(results[N_RESULTS - 1].ponyword); }
        else results[newi] = results[newi - 1];
        newi--;
    }

    if (newi < N_RESULTS)
    {
        results[newi].distance = dist;
        results[newi].ponified = strdup(ponified);
        results[newi].ponyword = strdup(ponyword);
        if (*count < N_RESULTS) (*count)++;
    }
};

//Ponifies all words from ponywords.txt
int ponify(const char* word)
{
    FILE *file = fopen("ponywords.txt", "r");
    if (file == NULL) file = fopen("../ponywords.txt", "r");
    if (file == NULL) { printf("Cannot open file ponywords.txt\n"); return 1; }

    char ponyword[32];
    struct Result results[N_RESULTS];
    unsigned int count = 0;

    while (fscanf(file, "%31s", ponyword) != EOF)
    {
        if (lowercase_check(ponyword))
        {
            char ponified[2][32];
            float dist[2];
            dist[0] = ponify_word(word, ponyword, ponified[0], 0);
            dist[1] = ponify_word(word, ponyword, ponified[1], 1);

            if (strcmp(ponified[0], ponyword) != 0) add_to_results(&count, dist[0], results, ponified[0], ponyword);
            if (strcmp(ponified[1], ponyword) != 0 && strcmp(ponified[0], ponified[1]) != 0) add_to_results(&count, dist[1], results, ponified[1], ponyword);
        }
    }

    if (count == 0)
    {
        printf("Failed to ponify the word\n");
        fclose(file);
        return 1;
    }

    for (unsigned int i = 0; i < count; i++)
    {
        printf("%s from %s, distance %2.2f\n", results[i].ponified, results[i].ponyword, results[i].distance);
        free(results[i].ponified);
        free(results[i].ponyword);
    }

    fclose(file);
    return 0;
};

//Prints help
void print_help(void)
{
    printf("Welcome to ponify 1.6\n");
    printf("This program ponifies words and names like:\n");
    printf("Manhatten -> Manehatten, Stalingrad -> Stalliongrad etc.\n");
    printf("Only latin alphabet is accepted, case doesn't matter\n");
    printf("The list of pony words that your word can be associated with is located in ponywords.txt file\n");
    printf("Created by Kyrylo Sovailo, k.sovailo@gmail.com\n");
};

int main(int argc, char **argv)
{
    if (argc == 2)
    {
        return ponify(argv[1]);
    }
    else
    {
        print_help();
        return 1;
    }
}