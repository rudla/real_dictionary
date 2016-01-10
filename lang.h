#ifndef _LANG_H_
#define _LANG_H_

#include "common.h"

#define LANG_MAX_CATEGORY_COUNT 16		// Maximal number of grammatical categories.
#define LANG_MAX_GRAMMEME_COUNT 64		// Maximum number of grammemes in category
#define LANG_MAX_WORD_CLASS_COUNT 64	// Maximum number of word types
#define LANG_MAX_SUFFIX_COUNT 100

/*
Types defining language properties.
They must be big enough to hold 0..MAX_<x>_COUNT
*/

typedef UInt8 Grammeme;						// 0..LANG_MAX_GRAMMEME_COUNT
typedef UInt8 GrammaticalCategory;			// 0..LANG_MAX_CATEGORY_COUNT
typedef UInt8 WordClass;

// https://en.wikipedia.org/wiki/Grammatical_category

typedef struct {
	Text   name;
	Text   grammemes[LANG_MAX_GRAMMEME_COUNT];
	Grammeme  grammeme_count;
} GrammaticalCategoryDef;

typedef struct {
	Text   name;
	GrammaticalCategory  used_categories[LANG_MAX_CATEGORY_COUNT];
	GrammaticalCategory  used_categories_count;
} WordClassDef;

typedef struct {
	Text name;
	Text txt;
} WordSuffixDef;

//TODO: Grammatical category as: number: <1>jednotné<#>množné

/*
Sentence state specifies values of grammatical categories at some position in the sentence.
*/

typedef struct {
	Grammeme state[LANG_MAX_CATEGORY_COUNT];
} SentenceState;

/*
Definition of a language has a set of grammatical categories and a set of word classes.
*/

typedef struct {

	GrammaticalCategoryDef categories[LANG_MAX_CATEGORY_COUNT];		// grammatical categories are stored here
	GrammaticalCategory    category_count;

	WordClassDef  word_classes[LANG_MAX_WORD_CLASS_COUNT];
	WordClass     word_class_count;

	WordSuffixDef suffixes[LANG_MAX_SUFFIX_COUNT];
	int           suffix_count;

	void (*set_count_fn)(SentenceState * state, Int32 n);
	// set_count_fn will be used to modify sentence state based on a number specified.

} Language;


Bool LangFindGrammeme(Language * lang, Text txt, GrammaticalCategory * p_category, Grammeme * p_grammeme);
Bool LangFindSuffix(Language * lang, Text txt, int * p_suffix);
Bool LangFindWordClass(Language * lang, Text name, WordClass * p_cls);

void CutEnd(char * line, char c);

#endif