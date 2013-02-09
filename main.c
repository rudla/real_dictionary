#include "dict.h"

void main()
{
	Language lang;
	Dictionary dict;

	PrintInit();

	Print("Language....\n");
	LangInit(&lang);
	LangLoad(&lang, "cs.lang");
	LangPrint(&lang);
	DictInit(&dict, &lang);
	DictLoad(&dict, "cs.dict");

	DictTest(&dict, "cs.test");

	PrintCleanup();
	exit(0);
}