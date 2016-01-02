#ifndef _DICT_H_
#define _DICT_H_

#include "common.h"

#define MAX_CAT_COUNT 8		// Maximal number of grammatical categories.
#define MAX_CASE_COUNT 255
#define MAX_WORD_CLASS_COUNT 64

#define CAT_FIRST_CHAR 1

typedef char * Text;

// https://en.wikipedia.org/wiki/Grammatical_category

typedef struct {
	Text name;
	Text grammemes[MAX_CASE_COUNT];
	UInt8  grammeme_count;
} GrammaticalCategory;

typedef struct {
	Text name;
	UInt8  used_categories[MAX_CAT_COUNT];
	UInt16 used_categories_count;
} WordClass;

/*
Sentence state specifies values of grammatical categories at some position in the sentence.
*/

typedef struct {
	UInt8 state[MAX_CAT_COUNT];
} SentenceState;

/*
Definition of a language has a set of grammatical categories and a set of word classes.
*/

typedef struct {

	GrammaticalCategory categories[MAX_CAT_COUNT];		// grammatical categories are stored here
	UInt8 category_count;

	WordClass word_classes[MAX_WORD_CLASS_COUNT];
	UInt8 word_class_count;

	void (*set_count_fn)(SentenceState * state, Int32 n);
	// set_count_fn will be used to modify sentence state based on a number specified.

} Language;

typedef struct {
	Language * lang;

	char * words[2048];
	UInt32 word_count;

	char * sentences[2048];
	int sentence_count;

} Dictionary;

void LangInit(Language * lang);
void LangPrint(Language * lang);
void LangLoad(Language * lang, char * file);

void DictInit(Dictionary * dict, Language * lang);
void DictLoad(Dictionary * dict, char * filename);
Bool DictFindWord(Dictionary * dict, char * word, UInt16 word_len, int * p_word_cat, int * p_word_idx, SentenceState * state);
/*
Purpose:
	Find specified word in the dictionary.
	If the word is found, the function returns true and specified SenteceState will contain grammatical categories implied by that word.
*/

typedef int (*DictOutFn)(void * ctx, char * text, int len);

void FormatText(char * text_key, Dictionary * dict, char * text_arguments[], int * num_args, DictOutFn out_fn, void * ctx);

void DictTest(Dictionary * dict, char * filename);

#endif
