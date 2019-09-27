Queries
=======


Phonometrica offers a number of features to find concordances in a corpus. It
also allows user to customize its search interface by creating search
grammars specifically tailored for a project. Concordancing features are
available in the ``Search`` menu.


Simple queries
--------------


To run a new query, click on ``Edit > Edit new query...`` or use
the shortcut ``alt+Q``. This will open a new
search window, which lets you search through all the annotations in your corpus. 

The Files box
~~~~~~~~~~~~~

The ``Files`` box in the top left corner allows you to select the type
of files to search in (documents or annotations). You can either select
files individually if you want to restrict your query to a particular
set of files, or leave all files unchecked, in which case Phonometrica will
try to search in all files.

The Search box
~~~~~~~~~~~~~~

The ``Search`` box in the top right corner allows you to enter some text
or a regular expression to search for. Next to the search field, a spin
box lets you select the tier you want to search in. The default choice
is ``Any tier``, which means that Phonometrica will try to find your pattern
in all tiers of the selected files. You can restrict the search to a
particular tier by selecting the appropriate tier number. Alternatively,
you can specify a *tier name pattern* using a regular expression. If you
do specify a tier name pattern, Phonometrica will ignore the tier number and
will search instead in any tier whose name matches the pattern.

By default, the text in the search field is interpreted as a regular
expression. If you are looking for plain text string instead, you can
select ``plain text`` instead of ``regular expression`` in the selector
located under the ``+`` and ``-`` buttons. Whether you use a plain text
string or a regular expression, the search will be case-insensitive by
default, which means that strings like "foo", "Foo" and "FOO" are
treated as identical. To perform a case sensitive search, simply check
the ``case sensitive`` box.

Concordances in a simple query follow the KWIC model (key word in
context), which means that a match is extracted along with its left and
right context. (The length of the context window can be adjusted in the
preferences.) When the context window is longer than the context of the
match in a given item (time point or interval), Phonometrica will extract
additional text in the preceeding/following items until the left/right
contexts have the expected length. If it cannot find enough items, it
will pad the text with white space. By default, Phonometrica will use one
white space character to join the text from different intervals. You can
specify a different string (including an empty string) in the
``Separator`` field.

Metadata
~~~~~~~~

If you have added properties to your project, a set of field boxes
will be added below the ``Files`` and ``Search`` box. Each field
category is displayed as a group box containing a list of all the labels
of this category. You can check or uncheck any label in any category
(each category also has an ``All labels`` button to check/uncheck all
labels at once). The search engine will filter files based on the
conditions that you specify in the field box. Within a category, it
uses the Boolean ``OR`` operator to find the subset of files that has
either label. Across categories, it uses the ``AND`` operator to find
the intersection of all the subsets defined by each category.

At the bottom of search window, an additional field lets you filter
files based on their description. For example, it is possible to extract
all the files that contain (or do not contain) a specific string.

Viewing results
~~~~~~~~~~~~~~~

Once you hit the ``ok`` button, the result of your query is presented as
a new ``query view`` in the viewer. You can browse the results with the
mouse wheel. The information panel on the right-hand side displays
information about the selected token.

If an annotation is bound to a sound file, you can play a match by
double-clicking on it or by pressing the space bar (you can also
interrupt it by pressing ``Esc``).

Right-clicking on item will display a context menu that allows you to
perform a number of actions:

-  ``Play selection``: this will play the corresponding item if the
   annotation is bound to a sound file.

-  ``Open in annotation``: this will open the annotation in a new view,
   along with its sound file if it is bound to a sound.

-  ``Open selection in Praat``: if Praat is installed and configured to
   work with Phonometrica, this will open the match in Praat. Phonometrica will open
   the TextGrid (and the sound file if the annotation is bound) in Praat
   and will display the current match. (Note that you need to have Praat
   already running for this to work.)

-  ``Edit item text``: this allows you to modify the text of the item
   where the match was found. (Note that the query view is currently not
   updated to reflect this change.)

-  ``Create table view``: this will convert the concordance set, along
   with all its metadata, to a table that can be imported into a
   spreadsheet program.

-  ``Export results to tab-separated file (CSV)...``: this exports the
   concordance set, along with all its metadata, to a CSV file that can
   be imported into a spreadsheet program.

-  ``Bookmark search result``: this allows you to bookmark a matched
   item. Bookmarks are displayed in the bookmark panel, which can
   accessed by clicking on the star in the bottom left corner of the
   main window.
   
Complex queries
---------------

After performing a `simple queries <simple.md>`__ on a set of annotation
files, Phonometrica attempts to find a set of concordances in one item (point
or interval) at a time. While it is possible for an item to match a
given search pattern several times if several substrings match the
pattern, matches are nevertheless limited to a single item.

Sometimes, however, we might want to match text in several items
*simultaneously*. Such a query is called a *complex query* in Phonometrica.
There are 3 types of relations between items, detailed below: alignment,
precedence and dominance.

Building a complex query
~~~~~~~~~~~~~~~~~~~~~~~~

When you open a search window, two small buttons with a ``+`` and ``-``
sign appear below the main the search field. These buttons allow you to
add and remove search items. Any query which has more than one search
item is a complex query.

When you add one or more search items, you will notice that each of them
(except the last one) is followed by a selector with 3 possible values:
``is aligned with``, ``precedes`` and ``dominates``. They correspond to
the tier item relations ``alignment``, ``precedence`` and ``dominance``,
respectively.

Contrary to simple queries, complex queries do not use the KWIC model to
display results. Instead of displaying a matched string in its context,
it lets the user select a ``display tier``, which appears at the top of
the search box. The text that is displayed is the concatenation of all
the items contain within the time interval defined by simultaneous
satisfaction of the constraints on each search item. Several examples
are given below.

Alignment relation
~~~~~~~~~~~~~~~~~~

Two items are aligned if they are on different tiers and their left and
right boundaries coincide. Suppose that you have a word tier (tier 1),
where each word was segmented, and a part-of-speech (POS) tier (tier 2)
which is aligned with the word tier. To extract all the nouns in the
corpus, you could do the following:

-  set ``NOUN`` as the search pattern for tier 1, and choose the
   ``is aligned with`` value of the relation selector.
-  set ``.+`` as the search pattern for tier 2
-  set the display tier to tier 2

Phonometrica will first look for all items whose text contains "NOUN" on tier
1, and will keep all those items which contain a non-empty label in an
item of tier 2 which is exactly aligned with a NOUN item on tier 1.
Phonometrica will then return a list of the text labels on tier 2 which match
the above criteria.

As another example, suppose you now want to extract all the adverbs that
end with *-ly*. You could do the following:

-  set ``ADV`` as the search pattern for tier 1, and choose the
   ``is aligned with`` value of the relation selector.
-  set ``.+ly$`` as the search pattern for tier 2
-  set the display tier to tier 2

Assuming that tier 2 contains exactly one word per interval, this will
successfully extract all the adverbs on tier 2 that end with *-ly*.

Precedence relation
~~~~~~~~~~~~~~~~~~~

Two items are in a precedence relation if they immediately follow each
other. You can search for arbitrarily long sequences by chaining search
items on the same tier. When you specify a sequence, Phonometrica will
retrieve the text from the display tier that is included within the span
defined by the sequence.

Suppose that you have a word tier (tier 1) and a POS tier (tier 2), as
in the alignment examples. Instead of searching for a single word, you
might be interested in looking for word sequences. To find all the
``DET+NOUN`` sequences, you could do the following:

-  set ``DET`` as the search pattern for the first tier item in tier 1,
   and choose the ``precedes`` value of the relation selector.
-  set ``NOUN`` as the search pattern for the second tier item, setting
   the tier number to 1 to ensure you are looking in the same tier
-  set the display tier to tier 2

Phonometrica will first look for all ``DET`` items on tier one, and will keep
only those that are followed by a ``NOUN`` item on the same tier. It
will then display the text that results from the concatenation of all
the items on tier 2 within the span determined by the beginning of the
``DET`` item and by the end of ``NOUN`` item on tier 1.

Dominance relation
~~~~~~~~~~~~~~~~~~

An item ``a`` dominates an item ``b`` if ``a`` and ``b`` are on
different tier, the left boundary of ``b`` is greater or equal to that
of ``a``, and the right boundary of ``b`` is lesser or equal to that of
``a``. Dominance relations typically encode hierarchical structures, for
instance ``word > syllable > segment``.

Suppose you have 3 tiers in your file: the first one contains spans
which denote syllables, the second one contains syllabic constituents
("syll") ("Onset", "Nucleus", "Coda") and the last one individual
segments ("p", "a", "t"...). In order to retrieve all syllables that end
in a coda, you could do the following:

-  set ``syll`` as the search pattern for tier 1, and choose the
   ``dominates`` value of the relation selector.
-  set ``Coda`` as the search pattern for tier 2
-  set the display tier to tier 3

This query will first get all the items that have a ``syll`` label on
the first tier; then, for each of those, it will look for a label
``Coda`` on tier 2 within the limits of the span on tier 1; for each
item which matches both conditions, it will display the concatenated
text of the items on tier 3 that are dominated by the matching item on
tier 1.
   
