Getting started
---------------


Main window
~~~~~~~~~~~

The left panel is the *file manager*: by default, it displays the
hierarchical structure of the project, but it can also display bookmarks
associated with the current project. The right panel is the *information
panel*, which is used to display and edit metadata about the file(s)
currently selected. The bottom panel is the *console*, which can be used
to type commands using Phonometrica's scripting engine. Finally, the central
part of the user interface is the *viewer*, which displays views such as
the result of a query. Each view is displayed as a tab, in a similar
fashion to web pages in a modern browser. The default view, the start
view, displays a few buttons for the most common operations a user may
want to perform.

Corpus management
~~~~~~~~~~~~~~~~~

Several functions from the ``File`` menu let the user important files
into a project, either individually or by importing a folder
recursively. The logical structure of a project is independent from the
physical organization of the files on the user's computer: once files
have been added to a project, they can be moved around, merged into new
folders or removed without affecting the files on disk. Phonometrica supports
several annotation formats, including TextGrid (Praat) and LAB
(WaveSurfer). It also supports a number of audio formats, including WAV,
AIFF and FLAC (the exact number of supported formats depends on the
platform). By default, Phonometrica will try to automatically bind an
annotation and a sound file if they have the same base name but a
different extension. If the names differ, it is possible to bind them
manually, by right-clicking on them and choosing the corresponding
option in the context menu, or semi-automatically using the
``Import metadata...`` feature from the file menu or using the scripting
engine.

The hierarchical organization of a project is a matter of pure
convenience to the user and is irrelevant for Phonometrica. Instead, the
program relies on metadata to keep files organized internally and to
perform queries. File names represent the most basic type of metadata
and for small projects (containing a dozen of files or so) this may be
all that is needed. When one needs to sort and organize a larger
collection of files, Phonometrica offers a flexible mechanism called
*properties*. A field is a typed key/value pair. Each file can be
tagged with an arbitrary number of such properties: the key represents a
category, which is always a text string, and the value may be either
Boolean, textual or numeric. Typical examples of properties would be
*Speaker* (where each unique speaker identifier represents a distinct
value, for example *11ajp1*) and *Gender* (with the values *Male* and
*Female*). Properties can be managed via the field editor, available
from the information panel when a file selection is active.

In addition to properties, each file can be annotated with a
description, a free-form string which can used to store any kind of
information, and which is also exposed to the search engine to filter
files.


Import metadata
~~~~~~~~~~~~~~~

Phonometrica lets you import metadata from a CSV file. The CSV file must have the following structure:

* the first line must be header
* the first column must contain the name of file to which we want to add metadata
* the separator must be the semicolon (``";"``)


The cells in the first column represent file names, as they are normally displayed by your operating system. For instance,
if you want to tag the file ``C:\Test\loc1f26.wav``, i.e. a file named ``loc1f26.wav`` located in a directory named 
``Test`` on the ``C:`` drive (on Windows), you should only provide ``loc1f26.wav`` as the file name. (Bear in mind that matching
is case sensitive, so ``LOC1F26.WAV`` would not work in this case.) In addition, Phonometrica allows you to use regular expressions
to define a *file name pattern* instead of an exact file name. To use a regular expression, the file name pattern must start with ``^`` 
(the metacharacter that indicates the beginning of a string) and end with ``$`` (the metacharacter that indicates the end of a string). You
can use any valid regular expression supported by Phonometrica, including capturing parentheses. You can refer to the whole matched pattern or to any subgroup in subsequent fields, using either ``%%`` or the placeholders ``%1`` to ``%9``, respectively. 


The header of each column (except for the first one) is interpreted as a property category, and the corresponding
cell for each file is interpreted as a category value. (Empty values are ignored.)

In addition, there are two special fields for columns: ``%SOUND%`` and ``%DESCRIPTION%``. ``%SOUND%`` allows you
to provide the full path of a sound file for an annotation. IF the sound file exists and the file is indeed an 
annotation, Phonometrica will bind the annotation to the sound; otherwise, the value will be dismissed. ``%DESCRIPTION%`` 
allows you to set the description of any file. Keep in mind that this field should not contain the character ``";"`` since it
is used as a separator. 

