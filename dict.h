#ifndef _DICT_H_
#define _DICT_H_

#include "lang.h"

#define CAT_FIRST_CHAR 1		// first character used to encode grammatical category

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
