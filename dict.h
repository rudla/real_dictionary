#ifndef _DICT_H_
#define _DICT_H_

#include "common.h"

#define CAT_COUNT 8
#define CAT_FIRST_CHAR 1

typedef struct {
	char state[CAT_COUNT];
} SentenceState;

typedef struct {
	char * name;
	char * cases[255];
	int case_count;

} GrammaticalCategory;

typedef struct {
	char * name;
	UInt16 categories_used;
} WordClass;

typedef struct {

	GrammaticalCategory categories[CAT_COUNT];		// grammatical cases are stored here
	int category_count;

	WordClass word_classes[64];
	int word_class_count;

	void (*set_count_fn)(SentenceState * state, Int32 n);

} Language;

typedef struct {
	Language * lang;

	char * words[2048];
	int word_count;

	char * sentences[2048];
	int sentence_count;

} Dictionary;

void LangInit(Language * lang);
void LangPrint(Language * lang);
void LangLoad(Language * lang, char * file);

void DictInit(Dictionary * dict, Language * lang);
void DictLoad(Dictionary * dict, char * filename);

void FormatText(char * text_key, Dictionary * dict, char * text_arguments[], int * num_args, void (*out_fn)(void * ctx, char * text, int len), void * ctx);

void DictTest(Dictionary * dict, char * filename);

#endif
