.. _page-plugins:

Plugins
=======


Phonometrica can be extended with plugins, which are written in JSON and the `Lua <http://www.lua.org>`_ scripting language.
When it starts up, Phonometrica loads all plugins which are located in the
system plugin directory or in the user plugin directory. Plugins can be
redistributed as ZIP files (the ``.zip`` extension is compulsory). To
install a plugin, go to ``File > Install plugin...`` and choose the ZIP
file. It will be installed in the current user’s plugin directory.

See :ref:`page-scripting` to learn more about scripting.

Structure of a plugin
---------------------

To be valid, a plugin must adhere to a number of conventions: if they
are not respected, Phonometrica will silently ignore the plugin. The root
directory of the plugin must contain the following:

-  a description file, named description.json (compulsory)
-  a ``Scripts`` sub-directory, which contains all your scripts
   (optional).
-  a ``Grammars`` sub-directory, which contains all your
-  a ``Resources`` sub-directory, which may contain anything (optional).

The description file contains all the information necessary to
initialize the plugin. All declarative aspects of the plugin are written
in the JSON format and must bear the extension ``.json``. Scripts are
written in Lua and must bear the extension ``.lua``.

Here is an example of a basic ``description.json`` file:

.. code:: json

    {
        "PluginInfo": {
            "Name": "My first plugin",
            "Version": "0.1",
        },
        
        "Menu": {"Text": "Custom menu", "Actions":
            [
                {"Type": "Action", "Text": "Test script", "Script": "test.lua", "Shortcut": "Ctrl+T"}
            ]
        }
    }

The header ``PluginInfo`` is the only part that is compulsory. It
contains essential information about the plugin. The ``Menu`` key lets
you create a custom menu: each menu entry (called "action") is
associated with a script which must be located in the ``Scripts``
sub-directory. When you click on an action in the menu, the
corresponding script is executed. It is also possible to assign a
shortcut to a given action.

Defining search grammars
------------------------

If you have devised a coding scheme for your data, Phonometrica lets you
define a "search grammar". A search grammar is a description of your
coding scheme which offers a user-friendly interface for querying your
data; it tells Phonometrica what to look for and how to present the
information to the user. Phonometrica will automatically load all valid search
grammars that are located in the ``Grammars`` sub-directory of your
plugin. It will create a submenu in the ``Conc`` menu, whose name is the
name of your plugins. All search grammars will be be available as tabs
in that submenu.

A search grammar defines a number of fields which can take on a number
of values. The user is presented with a number of checkboxes for each
field, and Phonometrica converts the query to the corresponding regular
expression, as defined by the grammar. Here is a simple yet realistic
example, drawn from the PFC project:

.. code:: json

    {
        "Header" : {
            "Object" : "SearchGrammar",
            "DisplayName": "Schwa",
            "Version": "0.9",
        },

        "Separator": "",
        "FileType": "Annotation",
        "Tier": 2,
        "FieldsPerRow": 2,
        
        "Fields" : [
            
            {"Name": "Schwa", "MatchAll": "[0-2]",
             "Values": [
                {"Match": "0", "Text": "Absent"},
                {"Match": "1", "Text": "Present"},
                {"Match": "2", "Text": "Uncertain"}, 
             ]
            },
            
            {"Name": "Position", "MatchAll": "[1-5]", 
             "Values": [
                {"Match": "1",  "Text": "monosyllable"},
                {"Match": "2",  "Text": "initial syllable"},
                {"Match": "3",  "Text": "median syllable"}
                {"Match": "4",  "Text": "final syllable"}
                {"Match": "5",  "Text": "metathesis"}
             ]
            },
            
            
            {"Name": "Left context", "MatchAll": "[1-5]",
             "Values": [
                {"Match": "1",  "Text": "vowel"},
                {"Match": "2",  "Text": "consonant"}
                {"Match": "3",  "Text": "start of an intonational phrase"}
                {"Match": "4",  "Text": "uncertain vowel"}
                {"Match": "5",  "Text": "simplified cluster"}
             ]      
            },
            
            
            {"Name": "Right context", "MatchAll": "[1-4]",
             "Values": [
                {"Match": "1", "Text": "vowel"},
                {"Match": "2", "Text": "consonant"},
                {"Match": "3", "Text": "weak prosodic boundary"},
                {"Match": "4", "Text": "strong prosodic boundary"}
             ]
            }   
        ]
    }

We first see a ``Header``, which provides information about the file.
The field ``Object`` indicates that the file is a search grammar, the
``Name`` corresponds to the name of the grammar, as it will be seen by
the user, and ``Version`` corresponds to the version of the grammar.

Next, the ``Separator`` attribute indicates the separator to be used
between fields. In this case, it is an empty string, which means that
the fields are concatenated directly (e.g. ``1412``). If the separator
was ``_``, each field should be separated by this symbol (e.g.
``1_4_1_2``). Note that although a field may be only one digit (or one
character), it does not need to be so; a field can be of any length,
provided that it can be described by a regular expression.

Next the ``Tier`` attribute indicates the tier number in which codings
should be searched for.

The following attribute, ``FieldsPerRow``, lets us specify how many
fields should be displayed in a row. In our case, since there are 4
fields, we decide to distribute them across 2 rows containing 2 fields
each.

Finally, the ``Fields`` attribute contains a list of fields, each of
them corresponding to a JSON object. The ``Name`` attribute provides a
descriptive label for the field. The ``MatchAll`` attribute is a regular
expression that should match all possible values for the field. If a
user doesn't check any value for a field, this attribute will be used to
retrieve all possible values. The ``Values`` attribute contains a list
of value. Each of them contains (at least) a ``Match`` attribute, which
is a string corresponding to the value, and a ``Text`` attribute which
is the label that will be displayed in the user interface for the
corresponding value, along with a check box. Note that checking all
values has the same effect as leaving all values unchecked.

TO BE CONTINUED...
