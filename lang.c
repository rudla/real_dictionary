#include "dict.h"

void CutEnd(char * line, char c)
{
	char * p;
	p = line;
	while(*p != 0 && *p != '\n' && *p != '\r' && *p != c) p++;
	while(p > line && (p[-1] == ' ' || p[-1] == '\t')) p--;
	*p = 0;
}

/***********************************************************

 Language

***********************************************************/
//$L

void LangSetCountOneMany(SentenceState * state, Int32 n)
/*
Purpose:
	The function sets the count grammatical category based on a number.
	This is for languages, that use one and many category.
	This function is a default function used for language.
*/
{
	if (n == 1) {
		state->state[0] = 0;
	} else {
		state->state[0] = 1;
	}
}

void LangInit(Language * lang)
/*
Purpose:
	Init language info structure.
*/
{
	lang->category_count = 0;
	lang->set_count_fn = LangSetCountOneMany;
	lang->word_class_count = 0;
	lang->suffix_count = 0;
}

GrammaticalCategory LangAddGrammaticalCategory(Language * lang, Text name)
/*
Purpose:
	Add new grammatical category to the language.
*/
{
	GrammaticalCategory cid = lang->category_count;
	GrammaticalCategoryDef * cat = &lang->categories[cid];
	cat->name = StrAlloc(name);
	cat->grammeme_count = 0;
	cat->grammemes[0] = NULL;
	lang->category_count++;
	return cid;
}

GrammaticalCategory LangFindGrammaticalCategory(Language * lang, Text name)
{
	GrammaticalCategory cid;
	GrammaticalCategoryDef * cat;
	for(cid = 0; cid < lang->category_count; cid++) {
		cat = &lang->categories[cid];
		if (strcmp(name, cat->name) == 0) return cid;
	}
	PrintFmt("Error: Unknown grammatical category '%s'\n", name);
	return 0;
}

void LangAddGrammeme(Language * lang, GrammaticalCategory category, Text txt)
{
	GrammaticalCategoryDef * cat;

	ASSERT(lang != NULL);
	ASSERT(category >= 0 && category < lang->category_count);

	cat = &lang->categories[category];

	ASSERT(cat->grammeme_count < LANG_MAX_GRAMMEME_COUNT);
	cat->grammeme_count++;
	cat->grammemes[cat->grammeme_count] = StrAlloc(txt);
}

Bool LangFindGrammeme(Language * lang, Text txt, GrammaticalCategory * p_category, Grammeme * p_grammeme)
/*
Purpose:
	Find grammeme in the language. Grammeme will be searched in all categories.
	Return true if found, false otherwise.
	Number of category and grammeme is returned in p_category and p_grammeme variables.
*/
{
	GrammaticalCategory category;
	Grammeme i;
	GrammaticalCategoryDef * cat;

	ASSERT(lang != NULL);

	for (category = 0; category<lang->category_count; category++) {
		cat = &lang->categories[category];
		for(i=1; i <= cat->grammeme_count; i++) {
			if (StrEqual(txt, cat->grammemes[i])) {
				*p_category = category;
				*p_grammeme = i;
				return true;
			}
		}
	}
	return false;
}

void ParseKey(Text txt, Text * p_key, Text * p_rest)
{
	Text p, s;
	s = p = SkipSpaces(txt);
	while(*p != 0 && *p != ':') p++;
	*p_key = StrAllocLen(s, p-s);
	*p_rest = SkipSpaces(p+1);
}

WordClass LangAddWordClass(Language * lang, Text txt)
{
	Text p, s;
	GrammaticalCategory cat;
	WordClass idx = lang->word_class_count;
	WordClassDef * wcls = &lang->word_classes[idx];

	ParseKey(txt, &wcls->name, &p);
	wcls->used_categories_count = 0;

	if (*p != 0) {
		while(*p == '<') {
			p++;
			s = p;
			while(*p!='>' && *p!=0) p++;
			*p = 0;
			cat = LangFindGrammaticalCategory(lang, s);
			wcls->used_categories[wcls->used_categories_count] = cat;
			wcls->used_categories_count++;
			p = SkipSpaces(p+1);
		}
	}

	lang->word_class_count++;
	return idx;
}

Bool LangFindWordClass(Language * lang, Text name, WordClass * p_cls)
{
	WordClass cid;
	WordClassDef * def;
	for(cid = 0; cid < lang->word_class_count; cid++) {
		def = &lang->word_classes[cid];
		if (strcmp(name, def->name) == 0) {
			*p_cls = cid;
			return true;
		}
	}
	return false;
}

char * LangEncodeText(Language * lang, Dictionary * dict, char * text);

void LangAddSuffix(Language * lang, Text txt)
{
	Text p;
	WordSuffixDef * suff = &lang->suffixes[lang->suffix_count];

	ParseKey(txt, &suff->name, &p);
	suff->txt = LangEncodeText(lang, NULL, p);

	lang->suffix_count++;
	
}

Bool LangFindSuffix(Language * lang, Text txt, int * p_suffix)
{
	int i;

	ASSERT(lang != NULL);

	for (i = 0; i<lang->suffix_count; i++) {
		if (StrEqual(txt, lang->suffixes[i].name)) {
			*p_suffix = i;
			return true;
		}
	}
	return false;
}

void LangLoad(Language * lang, char * file)
{
	FILE * f;
	unsigned char line[256];
	int  mode = 0;
	GrammaticalCategory category = 0;
	char * c;

	f = FileOpenReadUTF8(file);

	while (!feof(f)) {
		line[0] = 0;
		fgets(line, sizeof(line), f);
		CutEnd(line, '\n');
		if (strcmp(line, "== categories ==") == 0) {
			mode = 1;
		} else if (strcmp(line, "== classes ==") == 0) {
			mode = 2;
		} else if (strcmp(line, "== suffixes ==") == 0) {
			mode = 3;
		} else {
			CutEnd(line, ';');
			if (*line != 0) {
				switch(mode) {
				case 1:
					if (line[0] == '-' && line[1] == '-') {
						c = line+2;
						while(*c == ' ') c++;
						category = LangAddGrammaticalCategory(lang, c);
					} else {
						LangAddGrammeme(lang, category, line);
					}
					break;
				case 2:
					LangAddWordClass(lang, line);
					break;
				case 3:
					LangAddSuffix(lang, line);
					break;
				}
			}
		}
	}
	fclose(f);
}

void LangPrint(Language * lang)
{
	GrammaticalCategory cid;
	Grammeme i;
	GrammaticalCategoryDef * cat;

	for(cid = 0; cid < lang->category_count; cid++) {
		cat = &lang->categories[cid];
		printf("-- %s (%d) --\n", cat->name, cat->grammeme_count);
		for(i=1; i<=cat->grammeme_count; i++) {
			printf("%s\n", cat->grammemes[i]);
		}
		printf("\n");
	}

}
