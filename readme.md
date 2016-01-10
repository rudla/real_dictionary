Real Dictionary
===============

This is library that is able to format texts in languages that are inflected.
You may define language and it's word classes and grammatical cases and their possible values (gramemmes).
The defined gramemmes may be specified in the text and in the definitions of words, that may be inserted into text.

Appropriate inflection of the word will be used when used as an argument in text.   


Language definition
===================

Language is defined in `.lang` file, which is UTF-8 encoded text file with BOM character.
; can be used to introduce comments.

```
== categories ==
-- category 1
grammeme 1
grammeme 2
...
-- category 2

== classes ==
class1: <case 1><case 2> 
```

Grammatical categories
----------------------
Grammatical categories are defined in `categories` section.

https://en.wikipedia.org/wiki/Grammatical_category

Word classes
------------
Word classes (like noun, verb, adjective, adverb, pronoun, preposition) are defined in `classes` section.

Extensions
----------
Common word suffix sets may be specified in `suffixes` section.
Name of the set is specified before double colon.

Dictionary
==========

Words
----- 
In words section, words that can be used in texts are specified.

```
key ":" [forced grammemes] chars([grammemes]chars)*   [suffixes]
```

Sentences
---------

Texts
=====
When specifying text, grammemes may be specified using angle braces <>. When 
formatting the text, thay will be removed, but they will be used to find appropriate word forms.

Code     | Meaning
---------|-------------
`<x>`    | Grammeme. Specified grammeme is activated in subsequent text. (Unless other grammeme from same category is specified.)
`%1-%9`  | Numeric arguments.
`%A-%Z`  | Word arguments
`%^A`    | Use grammemes forced by specified word argument.
``word`  | Word referenced by it's key. This makes sure, the word will be properly inflected, typically based on argument.

