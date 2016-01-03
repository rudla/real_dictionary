#ifndef _LANG_H_
#define _LANG_H_

#include "common.h"

#define MAX_CAT_COUNT 16			// Maximal number of grammatical categories.
#define MAX_GRAMMEME_COUNT 64		// Maximum number of grammemes in category
#define MAX_WORD_CLASS_COUNT 64

// https://en.wikipedia.org/wiki/Grammatical_category

typedef UInt8 Grammeme;
typedef UInt8 GrammaticalCategory;
typedef UInt8 WordClass;

typedef struct {
	Text   name;
	Text   grammemes[MAX_GRAMMEME_COUNT];
	Grammeme  grammeme_count;
} GrammaticalCategoryDef;

typedef struct {
	Text   name;
	GrammaticalCategory  used_categories[MAX_CAT_COUNT];
	GrammaticalCategory  used_categories_count;
} WordClassDef;

/*
Sentence state specifies values of grammatical categories at some position in the sentence.
*/

typedef struct {
	Grammeme state[MAX_CAT_COUNT];
} SentenceState;

/*
Definition of a language has a set of grammatical categories and a set of word classes.
*/

typedef struct {

	GrammaticalCategoryDef categories[MAX_CAT_COUNT];		// grammatical categories are stored here
	GrammaticalCategory    category_count;

	WordClassDef  word_classes[MAX_WORD_CLASS_COUNT];
	WordClass     word_class_count;

	void (*set_count_fn)(SentenceState * state, Int32 n);
	// set_count_fn will be used to modify sentence state based on a number specified.

} Language;


Bool LangFindGrammeme(Language * lang, Text txt, GrammaticalCategory * p_category, Grammeme * p_grammeme);

void CutEnd(char * line, char c);

#endif