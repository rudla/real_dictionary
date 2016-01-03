﻿Real Dictionary
===============

This is library that is able to format texts in languages with rich morphology and complex compounding.
You may define language and it's word classes and grammatical cases and their possible values (gramemmes).
The defined gramemmes may be specified in the text and in the definitions of words, that may be inserted into text.

Appropriate variant of the word will be used when used as an argument in text.   




Cases are stored as special codes in text. They are stripped off when generating the text.
 
Count
=====
one     (1)
several (#)

Gender
======
man    (he)  ^
woman  (she) +
thing  (it) @

Article
=======
Defines, if we should use 'a' or 'an' before the word.


Gramatical cases
================

Gramatical case is defined using <>.

In Czech
--------
nominativ (kdo, co?)
genitiv (koho, čeho?)
dativ (komu, čemu?)
akuzativ (koho, co?)
vokativ (volání, oslovení)
lokál neboli lokativ (o kom, o čem?)
instrumentál (kým, čím?)

Words implying a case
=====================
:::::
<en>: are<#>, many<#>, some<#>
<cz>: jsou<#>, kolik<čeho#>, nějaké<co#>, on<^>
::::::

Adjectives & Nouns
==================

It is possible to define, that a word is adjective and should be modified based on following noun.
In general, word should be modified based on some characteristics, defined by following word.


Words
=====

Words are set of word cases.
At the beginning is the common part of a word.
In word definition, we may use # directly.

:::::
<cs>Tiskárn<co>a#y<čeho>y#en<koho>u#y 
<en>Printer#s
:::::

Categories
==========

Words may be categorized. At least nouns, verbs etc. may be specified.


Examples
========

obj: tiskárna

"<cs>Kolik <čeho#>[obj] je k dispozici?"  => "<cs>Kolik [obj] je k dispozici?"
"<cs>Prosím, zapněte<koho> [obj]!"

"Kolik tiskáren je k dispozici?"
"Prosím, zapněte tiskárnu."

obj: printer
"<en>How many <what#>[obj] are available?"

"How many printers are available?"

"<en>How many <what#>[obj] are available?"