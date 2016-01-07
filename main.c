#include "dict.h"

void main()
{
	Language lang;
	Dictionary dict;
	SentenceState state;
	int word_cat, word_idx;

	PrintInit();

	Print("Language....\n");
	LangInit(&lang);
	LangLoad(&lang, "cs.lang");
	LangPrint(&lang);
	DictInit(&dict, &lang);
	DictLoad(&dict, "cs.dict");

	DictDescribeWord(&dict, 0, "woman");

	DictFindWord(&dict, "mu\xC5\xBEi", 6, &word_cat, &word_idx, &state);
	DictFindWord(&dict, "vid\xC3\xADm", 6, &word_cat, &word_idx, &state);

	DictTest(&dict, "cs.test");

	PrintCleanup();
	exit(0);
}