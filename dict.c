#include "dict.h"

#define ENCODE_BUF_SIZE 1024
#define WORD_CHR 31
#define SUFFIX_CHR 127


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

char * LangEncodeText(Language * lang, Dictionary * dict, char * text)
/*
<case>   specify change of grammatical category (grammeme).
'word    specified word must be specified in dictionary and will be changed according to current sentence state
*/
{
	int i, j, wid, suffix;
	GrammaticalCategory category;
	Grammeme grammeme;
	char * p;
	char id[40];
	char buf[ENCODE_BUF_SIZE];
	unsigned char c;

	i = 0;
	p = text;
	while((c = *p++) != 0) {
		if (c == '<') {
			j = 0;
			while((c = *p++) && c != '>') id[j++] = c;			
			id[j] = 0;
			if (LangFindGrammeme(lang, id, &category, &grammeme)) {
				if (i > ENCODE_BUF_SIZE-2) goto overrun;
				buf[i++] = category+CAT_FIRST_CHAR;		// 1..16 is category character
				buf[i++] = grammeme;
			} else if (LangFindSuffix(lang, id, &suffix)) {
				if (i > ENCODE_BUF_SIZE-2) goto overrun;
				buf[i++] = SUFFIX_CHR;
				ASSERT(suffix < 128); // TODO: Encode as UTF-8 character
				buf[i++] = suffix+1;
			} else {
				printf("Unknown case '%s'\n", id);
			}
		} else if (dict != NULL && c == '`') {
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
	encoded = LangEncodeText(dict->lang, dict, word);
	dict->word_count++;
	dict->words[dict->word_count] = encoded;
}

void DictAddSentence(Dictionary * dict, char * sentence)
{
	char * encoded;
	if (*sentence == 0) return;
	encoded = LangEncodeText(dict->lang, dict, sentence);
	dict->sentence_count++;
	dict->sentences[dict->sentence_count] = encoded;
}

void DictLoad(Dictionary * dict, char * filename)
{
	unsigned char line[256];
	FILE * f;
	int  mode = 0;
	WordClass word_class;
	Text s;

	f = FileOpenReadUTF8(filename);
	if (f == NULL) return;

	while (!feof(f)) {
		fgets(line, sizeof(line), f);
		CutEnd(line, ';');
		if (*line == 0) {
		} else if (strcmp(line, "== words ==") == 0) {
			mode = 1;
		} else if (strcmp(line, "== sentences ==") == 0) {
			mode = 2;
		} else {
			switch(mode) {
			case 1:
				if (line[0] == '-' && line[1] == '-') {
					s = SkipSpaces(line + 2);
					if (!LangFindWordClass(dict->lang, s, &word_class)) {
						printf("Error: Unknown Word class '%s'", s);
						return;
					}
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
	for(i=0; i<LANG_MAX_CATEGORY_COUNT; i++) state->state[i] = 0;
}

int SentenceStateCmp(SentenceState * state1, SentenceState * state2)
{
	int i;
	for(i=0; i<LANG_MAX_CATEGORY_COUNT; i++) {
		if (state1->state[i] != 255 && state1->state[i] != state2->state[i]) return 0;
	}
	return 1;
}

int IsGroupChar(char c)
{
	return c >= CAT_FIRST_CHAR && c < CAT_FIRST_CHAR+LANG_MAX_CATEGORY_COUNT;
}

int DictFindWord(Dictionary * dict, char * word, UInt16 word_len, int * p_word_cat, int * p_word_idx, SentenceState * state)
{
	char * p;
	char key[128];
	char buf[128];
	int key_len;
	UInt32 i;
	char c;
	int prefix_len, wlen;
	int category, gcase;
	
	SentenceStateInit(state);

	for(i = 1; i <= dict->word_count; i++) {
		p = dict->words[i];
		
		// copy key to separate buffer and skip it
		key_len = 0;
		while(*p != ':') {
			key[key_len] = *p++;
			key_len++;
		}
		key[key_len] = 0;
		p++;

		while(*p == 32) p++;

		prefix_len = 0; wlen = 0;

		do {
			while (IsGroupChar(*p)) {
				category = *p++ - CAT_FIRST_CHAR;
				gcase = *p++ - 1;
				state->state[category] = gcase;
			}
			wlen = 0;

			while(*p != 0 && !IsGroupChar(*p)) {
				c = *p++;
				if (c == '-') {
//					if (prefix_len == 0) {
//						prefix_len = wlen;
//					} else {
						if (wlen != 0) {
							prefix_len = wlen;		// if we are in the middle of the word, this is new prefix
						} else {
							wlen = prefix_len;
						}
//					}
				} else {
					buf[wlen] = c;
					wlen++;
				}
			}
			buf[wlen] = 0;

			if (prefix_len == 0) prefix_len = wlen;

			Print(buf);
			PrintEOL();

		} while(*p != 0);

	}
	return false;
}

void FormatWord(char * word, Language * lang, SentenceState * state,  DictOutFn out_fn, void * ctx)
{
	char * p;
	char buf[128];
	char buf2[128];
	int prefix_len, wlen;
	SentenceState state2;
	int category, gcase;

	SentenceStateInit(&state2);

	p = word;

	// grammemes defined at the beginning of the word are set both to state and compared state
	while(IsGroupChar(*p)) {
		category = *p++ - CAT_FIRST_CHAR;
		gcase = *p++ - 1;
		state->state[category] = gcase;
		state2.state[category] = gcase;
	}

	prefix_len = 0;		
	wlen = 0;
	while(*p != 0 && !IsGroupChar(*p)) {
		if (*p == SUFFIX_CHR) {
			p = lang->suffixes[p[1]-1].txt;
		}
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
		if (*p == SUFFIX_CHR) {
			p = lang->suffixes[p[1]-1].txt;
		}
		if (*p == 0) break;		//TODO: This word is not right
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

		while(*p != 0 && !IsGroupChar(*p)) {
			if (*p == SUFFIX_CHR) {
				p = lang->suffixes[p[1]-1].txt;
			}
			buf2[wlen++] = *p++;
		}

		buf2[wlen] = 0;
	}

	out_fn(ctx, buf2, wlen);
}

void FormatText(char * text_key, Dictionary * dict, char * text_arguments[], int * num_args,  DictOutFn out_fn, void * ctx)
/*
   <>     grammatical grammemes required for a text argument (as defined by the language)
   %A-Z   insert specified word argument
   %^A-Z  insert grammatical grammemes implied by specified argument
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
		for(p2 = p; *p2 != 0 && (*p2 < CAT_FIRST_CHAR || *p2 >= CAT_FIRST_CHAR+LANG_MAX_CATEGORY_COUNT) && *p2 != '%' && *p2 != WORD_CHR && *p2 != SUFFIX_CHR; p2++);
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
				if (p2 != NULL) {
					if (act == 0) {
						FormatWord(p2, dict->lang, &state, out_fn, ctx);
					} else if (act == 1) {
						while(IsGroupChar(*p2)) {
							category = *p2++;
							gcase = *p2++ - 1;
							state.state[category-CAT_FIRST_CHAR] = gcase;
						}
					}
				} else {
					PrintFmt("Error: Word '%s' not found.", targ);
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

int OutMem(void * ctx, char * text, int len)
{
	char ** p_p = ctx;
	char * p = *p_p;
	while(len-->0) *p++ = *text++;
	*p_p = p;
	return 0;
}

void DictDescribeWord(Dictionary * dict, WordClass cls, Text word_name)
{
	Text txt;
//	Grammeme g[LANG_MAX_CATEGORY_COUNT];
	GrammaticalCategory i, cat_no;
	GrammaticalCategoryDef * cat;
	SentenceState state;
	Grammeme j;
	char buf[128];
	int word_no;
	char * s;

	WordClassDef * def = &dict->lang->word_classes[cls];
	word_no = FindWordIndex(dict->words, dict->word_count, word_name);
	txt = DictWordAtIndex(dict, word_no);

	SentenceStateInit(&state);
//	for(i = 0; i<def->used_categories_count; i++) g[i] = 0;

//	for(i=def->used_categories_count+1; i>0; i--) {
	i = 2;
	cat_no = def->used_categories[i-1];
	cat = &dict->lang->categories[cat_no];
	for(j=1; j<=cat->grammeme_count; j++) {
		state.state[cat_no] = j-1;
		s = buf;
		FormatWord(txt, dict->lang, &state, OutMem, &s);
		*s = 0;
		PrintFmt("%s: %s\n", cat->grammemes[j], buf);
	}
//	}
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

