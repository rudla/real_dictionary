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

void LangSetCount(SentenceState * state, Int32 n)
{
	if (n == 1) {
		state->state[0] = 0;
	} else {
		state->state[0] = 1;
	}
}

void LangInit(Language * lang)
{
	lang->category_count = 0;
	lang->set_count_fn = LangSetCount;
	lang->word_class_count = 0;
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

GrammaticalCategory LangAddGrammaticalCategory(Language * lang, Text name)
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

	ASSERT(cat->grammeme_count < MAX_GRAMMEME_COUNT);
	cat->grammeme_count++;
	cat->grammemes[cat->grammeme_count] = StrAlloc(txt);
}

Bool LangFindGrammeme(Language * lang, Text txt, GrammaticalCategory * p_category, Grammeme * p_grammeme)
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

WordClass LangAddWordClass(Language * lang, Text txt)
{
	Text p, s;
	GrammaticalCategory cat;
	WordClass idx = lang->word_class_count;
	WordClassDef * wcls = &lang->word_classes[idx];

	s = p = SkipSpaces(txt);
	while(*p != 0 && *p != ':') p++;
	wcls->name = StrAllocLen(s, p-s);
	wcls->used_categories_count = 0;

	if (*p != 0) {
		p = SkipSpaces(p+1);
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

	return idx;
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
		fgets(line, sizeof(line), f);
		CutEnd(line, '\n');
		if (strcmp(line, "== cases ==") == 0) {
			mode = 1;
		} else if (strcmp(line, "== classes ==") == 0) {
			mode = 2;
		} else {
			switch(mode) {
			case 1:
				if (line[0] == '-' && line[1] == '-') {
					c = line+2;
					while(*c == ' ') c++;
					category = LangAddGrammaticalCategory(lang, c);
				} else {
					CutEnd(line, ';');
					if (*line != 0) {
						LangAddGrammeme(lang, category, line);
					}
				}
				break;
			case 2:
				CutEnd(line, ';');
				if (*line != 0) {
					LangAddWordClass(lang, line);
				}					
				break;
			}
		}
	}
	fclose(f);
}

