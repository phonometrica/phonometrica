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
into a project, either individually or by importing a directory
recursively. The logical structure of a project is independent from the
physical organization of the files on the user's computer: once files
have been added to a project, they can be moved around, merged into new
directories or removed without affecting the files on disk. Phonometrica supports
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

Phonometrica lets you import metadata from a CSV file, which is a tabular format according to which each line 
represents a row, and values in a row are separated by a **separator character**. When you click on 
``Import metadata...`` in the ``File`` menu or in the metadata panel, a dialog opens up and asks you to choose 
a file as well as the separator character. We use the semicolon by default, since this is the separator used by
Excel when it exports a spreadsheet to CSV. The file must have the following structure:

* the first line must be a header: the first field is ignored, and the following fields correspond to property categories.
* the first column must contain the name of file to which we want to add metadata, and the following properties are values for the corresponding categories. Cells can be left empty for files files which doesn't have a certain property.


The cells in the first column represent file names, as they are normally displayed by your operating system. For instance,
if you want to tag the file ``C:\Test\loc1f26.wav``, i.e. a file named ``loc1f26.wav`` located in a directory named 
``Test`` on the ``C:`` drive (on Windows), you should only provide ``loc1f26.wav`` as the file name. (Bear in mind that matching
is case sensitive, so ``LOC1F26.WAV`` would not work in this case.) In addition, Phonometrica allows you to use regular expressions
to define a *file name pattern* instead of an exact file name. To use a regular expression, the file name pattern must start with ``^`` 
(the metacharacter that indicates the beginning of a string) and end with ``$`` (the metacharacter that indicates the end of a string). You
can use any valid regular expression supported by Phonometrica, including capturing parentheses. You can refer to the whole matched pattern or to any subgroup in subsequent fields, using either ``%%`` or the placeholders ``%1`` to ``%9``, respectively. 


By default, properties are assigned the type ``text``. You can add the suffixes ``.bool``, ``.num`` or ``.text`` to the category in 
the header line to indicate that the property is a Boolean, a number or a text string, respectively. In addition, there are two special fields for columns: ``%SOUND%`` and ``%DESCRIPTION%``. ``%SOUND%`` allows you
to provide the full path of a sound file for an annotation. If the sound file exists and the file is indeed an 
annotation, Phonometrica will bind the annotation to the sound; otherwise, the value will be dismissed. ``%DESCRIPTION%`` 
allows you to set the description field of a file.
