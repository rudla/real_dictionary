#include "dict.h"

#define WORD_CHR 31

FILE * FileOpenReadUTF8(char * filename)
{
	FILE * f;
	unsigned char buf[3];
	f = fopen(filename, "r");

	if (f != NULL) {
		// Skip BOM
		if (fread(buf, 1, 3, f) == 3) {
			if (buf[0] == 0xEF && buf[1] == 0xBB && buf[2] == 0xBF) {

			} else {
				fseek(f, 0, SEEK_SET);
			}
		}
	}
	return f;
}

char * SkipSpaces(char * s)
{
	while(*s == ' ') s++;
	return s;
}

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
}

void LangPrint(Language * lang)
{
	int cid, i;
	GrammaticalCategory * cat;
	for(cid = 0; cid < lang->category_count; cid++) {
		cat = &lang->categories[cid];
		printf("-- %s (%d) --\n", cat->name, cat->case_count);
		for(i=1; i<=cat->case_count; i++) {
			printf("%s\n", cat->cases[i]);
		}
		printf("\n");
	}
}


int LangAddCaseCategory(Language * lang, char * name)
{
	int category;
	category = lang->category_count;
	lang->categories[category].name = StrAlloc(name);
	lang->categories[category].case_count = 0;
	lang->categories[category].cases[0] = NULL;
	lang->category_count++;
	return category;
}

void LangAddCase(Language * lang, int category, char * txt)
{
	GrammaticalCategory * cat = &lang->categories[category];
	char * c;
	c = StrAlloc(txt);
	cat->case_count++;
	cat->cases[cat->case_count] = c;
}

Bool LangFindCase(Language * lang, char * txt, int * p_category, int * p_case)
{
	int i, category;
	GrammaticalCategory * cat;
	for (category = 0; category<lang->category_count; category++) {
		cat = &lang->categories[category];
		for(i=1; i <= cat->case_count; i++) {
			if (strcmp(txt, cat->cases[i]) == 0) {
				*p_category = category;
				*p_case = i;
				return true;
			}
		}
	}
	return false;
}

void LangLoad(Language * lang, char * file)
{
	FILE * f;
	unsigned char line[256];
	int  mode = 0;
	int category = 0;
	char * c;

	f = FileOpenReadUTF8(file);

	while (!feof(f)) {
		fgets(line, sizeof(line), f);
		CutEnd(line, '\n');
		if (strcmp(line, "== cases ==") == 0) {
			mode = 1;
		} else if (strcmp(line, "== word types ==") == 0) {
			mode = 2;
		} else {
			switch(mode) {
			case 1:
				if (line[0] == '-' && line[1] == '-') {
					c = line+2;
					while(*c == ' ') c++;
					category = LangAddCaseCategory(lang, c);
				} else {
					CutEnd(line, ';');
					if (*line != 0) {
						LangAddCase(lang, category, line);
					}
				}
				break;
			}
		}
	}
	fclose(f);
}

#define ENCODE_BUF_SIZE 1024

int IsAlpha(unsigned char c)
{
	return c>='a' && c<='z' || c>=128;
}


/***********************************************************

Dictionary

***********************************************************/
//$D

void DictInit(Dictionary * dict, Language * lang)
{
	dict->lang = lang;
	dict->words[0] = NULL;
	dict->word_count = 0;
	dict->sentences[0] = NULL;
	dict->sentence_count = 0;

}

Int16 FindWordIndex(char ** list, int count, char * key)
{
	int w;
	char * p, * s;
	for(w=1; w <= count; w++) {
		s = list[w];
		p = key;

		while(*s != ':') {
			if (*s != *p) break;
			s++;
			p++;
		}

		if (*s == ':' && *p == 0) {
			return w;
		}
	}
	return 0;
}

char * DictWordAtIndex(Dictionary * dict, UInt32 idx)
{
	char * s;
	s = dict->words[idx];
	while(*s != ':') s++;
	s = SkipSpaces(s+1);
	return s;
}

char * FindByKey(char ** list, int count, char * key)
{
	int w;
	char * p, * s;
	for(w=1; w <= count; w++) {
		s = list[w];
		p = key;

		while(*s != ':') {
			if (*s != *p) break;
			s++;
			p++;
		}

		if (*s == ':' && *p == 0) {
			s++;
			while(*s == ' ') s++;
			return s;
		}
	}
	return NULL;

}

char * DictEncodeText(Dictionary * dict, char * text)
/*
<case>   specify change of grammatical change
'word    specified word must be specified in dictionary and will be changed according to current sentence state
*/
{
	int i, j, wid;
	int category, ccase;
	char * p;
	char id[40];
	char buf[ENCODE_BUF_SIZE];
	unsigned char c;
//	Language * lang = dict->lang;

	i = 0;
	p = text;
	while((c = *p++) != 0) {
		if (c == '<') {
			while((c = *p++) != 0 && c != '>') {
				j = 0;
				if (IsAlpha(c)) {
					do {
						id[j++] = c;
						c = *p++;
					} while(IsAlpha(c));
					p--;
				} else {
					id[0] = c;
					j = 1;
				}
				id[j] = 0;
				if (LangFindCase(dict->lang, id, &category, &ccase)) {
					if (i > ENCODE_BUF_SIZE-2) goto overrun;
					buf[i++] = category+CAT_FIRST_CHAR;		// 1..16 is category character
					buf[i++] = ccase;			// to prevent 0
				} else {
					printf("Unknown case '%s'\n", id);
				}
			}
		} else if (c == '`') {
			j = 0;
			while(IsAlpha(*p)) {
				id[j++] = *p++;
			}
			id[j] = 0;
			wid = FindWordIndex(dict->words, dict->word_count, id);
			buf[i++] = WORD_CHR;
			ASSERT(wid < 128); // TODO: Encode as UTF-8 character
			buf[i++] = wid;
		} else {
			if (i == ENCODE_BUF_SIZE) goto overrun;
			buf[i++] = c;
		}
	}
	buf[i] = 0;
	return StrAlloc(buf);
overrun:
	return NULL;
}

void DictAddWord(Dictionary * dict, char * word)
{
	char * encoded;
	if (*word == 0) return;
	encoded = DictEncodeText(dict, word);
	dict->word_count++;
	dict->words[dict->word_count] = encoded;
}

void DictAddSentence(Dictionary * dict, char * sentence)
{
	char * encoded;
	if (*sentence == 0) return;
	encoded = DictEncodeText(dict, sentence);
	dict->sentence_count++;
	dict->sentences[dict->sentence_count] = encoded;
}

void DictLoad(Dictionary * dict, char * filename)
{
	unsigned char line[256];
	FILE * f;
	int  mode = 0;

	f = FileOpenReadUTF8(filename);
	if (f == NULL) return;

	while (!feof(f)) {
		fgets(line, sizeof(line), f);
		CutEnd(line, '\n');
		if (strcmp(line, "== words ==") == 0) {
			mode = 1;
		} else if (strcmp(line, "== sentences ==") == 0) {
			mode = 2;
		} else {
			switch(mode) {
			case 1:
				if (line[0] == '-' && line[1] == '-') {
				} else {
					DictAddWord(dict, line);
				}
				break;
			case 2:
				DictAddSentence(dict, line);
				break;
			}
		}
	}
	fclose(f);

}

void OutPrint(void * ctx, char * text, int len)
{
	printf("%.*s", len, text);
}

void SentenceStateInit(SentenceState * state)
{
	int i;
	for(i=0; i<CAT_COUNT; i++) state->state[i] = 0;
}

int SentenceStateCmp(SentenceState * state1, SentenceState * state2)
{
	int i;
	for(i=0; i<CAT_COUNT; i++) {
		if (state1->state[i] != 255 && state1->state[i] != state2->state[i]) return 0;
	}
	return 1;
}

int IsGroupChar(char c)
{
	return c >= CAT_FIRST_CHAR && c < CAT_FIRST_CHAR+CAT_COUNT;
}

void FormatWord(char * word, Language * lang, SentenceState * state, void (*out_fn)(void * ctx, char * text, int len), void * ctx)
{
	char * p;
	char buf[128];
	char buf2[128];
	int prefix_len, wlen;
	SentenceState state2;
	int category, gcase;

	SentenceStateInit(&state2);

	p = word;

	// Cases defined at the beginning of the word are set both to state and compared state
	while(IsGroupChar(*p)) {
		category = *p++ - CAT_FIRST_CHAR;
		gcase = *p++ - 1;
		state->state[category] = gcase;
		state2.state[category] = gcase;
	}

	prefix_len = 0;		
	wlen = 0;
	while(!IsGroupChar(*p)) {
		if (*p == '-') {
			prefix_len = wlen;
		} else {
			buf[wlen++] = *p;
		}
		p++;
	}
	buf[wlen] = 0;

	if (prefix_len == 0) prefix_len = wlen;

	strcpy(buf2, buf);
	while(!SentenceStateCmp(&state2, state)) {
		if (*p == 0) break;
		while(IsGroupChar(*p) && *p != 0) {
			category = *p++;
			state2.state[category-CAT_FIRST_CHAR] = *p++ - 1;
		}

		if (*p == '-') {
			memcpy(buf2, buf, prefix_len);
			wlen = prefix_len;
			p++;
		} else {
			wlen = 0;
		}

		while(*p != 0 && !IsGroupChar(*p)) buf2[wlen++] = *p++;

		buf2[wlen] = 0;
	}

	out_fn(ctx, buf2, wlen);
}

void FormatText(char * text_key, Dictionary * dict, char * text_arguments[], int * num_args, void (*out_fn)(void * ctx, char * text, int len), void * ctx)
/*
   <>     grammatical cases required for a text argument (as defined by the language)
   %A-Z   insert specified word argument
   %^A-Z  insert grammatical cases implied by specified argument
*/
{
	char * p, * p2;
	char c;
	SentenceState state;
	char * targ;
	int category, gcase;
	int act, len;
	long n;
	char buf[30];
	UInt32 wid;

	SentenceStateInit(&state);

	p = FindByKey(dict->sentences, dict->sentence_count, text_key);

//	encoded = LangEncodeText(lang, text);

//	p = encoded;
	for(; ;) {
		for(p2 = p; *p2 != 0 && (*p2 < CAT_FIRST_CHAR || *p2 >= CAT_FIRST_CHAR+CAT_COUNT) && *p2 != '%' && *p2 != WORD_CHR; p2++);
		if (p2 != p) out_fn(ctx, p, p2 - p);
		p = p2;
		c = *p++;
		if (c == 0) {
			break;
		} else if (c == '%') {
			c = *p++;
			act = 0;
			if (c == '^') { act = 1; c = *p++; }

			if (c>='1' && c<='9') {
				n = num_args[c-'1'];
				len = sprintf(buf, "%d", n);
				out_fn(ctx, buf, len);
				dict->lang->set_count_fn(&state, n);

			} else if (c>='A' && c<='Z') {
				targ = text_arguments[c-'A'];
				p2 = FindByKey(dict->words, dict->word_count, targ);
				if (act == 0) {
					FormatWord(p2, dict->lang, &state, out_fn, ctx);
				} else if (act == 1) {
					while(IsGroupChar(*p2)) {
						category = *p2++;
						gcase = *p2++ - 1;
						state.state[category-CAT_FIRST_CHAR] = gcase;
					}
				}
			}
		} else if (c == WORD_CHR) {
			wid = *p++;		// TODO: decode UTF-8 char
			p2 = DictWordAtIndex(dict, wid);
			FormatWord(p2, dict->lang, &state, out_fn, ctx);
		} else {
			category = c;
			gcase = *p++ - 1;
			state.state[category-CAT_FIRST_CHAR] = gcase;
		}
	}

}

/*********************************************************

  Tests

*********************************************************/
//$T

void OutMem(void * ctx, char * text, int len)
{
	char ** p_p = ctx;
	char * p = *p_p;
	while(len-->0) *p++ = *text++;
	*p_p = p;
}

void DictTest(Dictionary * dict, char * filename)
{
	FILE * f;
	unsigned char line[256];
	int  mode = 0;
	int category = 0;
	char * s, * start, * end, c;
	char * targs[64];
	int  nargs[64];
	int targ_cnt = 0;
	int narg_cnt = 0;
	char buf[1024];


	f = FileOpenReadUTF8(filename);

	while (!feof(f)) {
		*line = 0;
		fgets(line, sizeof(line), f);
		CutEnd(line, '\n');

		if (*line == 0) continue;

		targ_cnt = 0;
		narg_cnt = 0;

		s = SkipSpaces(line);
		while (*s != 0) {
			start = s;
			while((c = *s) != 0 && c != ',' && c != ':') {
				s++;
				if (c != ' ') end = s;
			}
			if (c != 0) {
				*end = 0;
				s = SkipSpaces(s+1);

				if (*start>='0' && *start<='9') {
					nargs[narg_cnt++] = strtol(start, &end, 10);
				} else {
					targs[targ_cnt++] = start;
				}

				if (c == ':') {
					break;
				}
			}
		}

		s = buf;
		FormatText(targs[0], dict, targs+1, nargs, &OutMem, &s);
		*s = 0;
		Print(buf);
		PrintEOL();
	}
	fclose(f);
}

