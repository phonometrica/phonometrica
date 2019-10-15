.. _page-plugins:

Plugins
=======


Phonometrica can be extended with plugins, which are written in `JSON <https://www.json.org/>`_ and Phonometrica's scripting language.
When it starts up, Phonometrica loads all plugins which are located in the system plugin directory or in the user plugin directory. Plugins 
can add functionality to Phonometrica in a number of ways, but most commonly they will create a submenu in the `Tools` menu which provides 
additional commands offered by the plugin and/or custom coding protocols.

Plugins can be shared and redistributed as ZIP files. To install a plugin, go to ``Tools > Install plugin...`` and choose the ZIP
file corresponding to the plugin you wish to install. It will be installed in the current user’s plugin directory.

See :ref:`page-scripting` to learn more about scripting.

Structure of a plugin
---------------------

To be valid, a plugin must adhere to a number of conventions. A plugin usually contains the following items:

-  a description file, named ``description.json`` (compulsory)
-  a script named ``initialize.phon`` (optional). It exists, it will be loaded after reading the description file.
-  a script named ``finalize.phon`` (optional). If it exists, it will be run when the program exits.
-  a ``Scripts`` sub-directory, which contains all your scripts
   (optional).
-  a ``Protocols`` sub-directory, which contains coding protocols (optional)
-  a ``Resources`` sub-directory, which may contain anything (optional).

The description file (``description.json``) contains all the information necessary to initialize the plugin. It must **at least** contain a name. 

.. code:: json

    {
        "name": "My first plugin"
    }

Here is a more realistic (and useful) example:

.. code:: json

    {
        "name" : "PFC",
        "version": "0.1",
        "description":  [
            "Plugin for the PFC project.\n",
            "See http://www.projet-pfc.net"
        ],

        "actions" : [
            { "name": "Add metadata", "target": "add_metadata.phon" , "shortcut": "alt+M" }
        ]
    }

The version field can be used to distinguish different versions of the plugin, for instance if a later update breaks backward compatibility. The 
description field is displayed in the ``About`` entry in the plugin's menu (if it exists). This field can be a simple string or, as in the example
above, an array of strings.

The optional field named ``actions`` maps to an array of objects: Each action describes a menu entry, and must have at least two fields: a ``name`` key, 
which is the entry's label (as it is displayed in the menu), and a ``target`` field, which must give the name of a script (located in the plugin's 
``Scripts`` sub-directory) which will be executed when the menu entry is clicked on. Additionally, an action may contain a ``shortcut``, which provides
a shortcut to execute the script the action is bound to.


Defining coding protocols
------------------------

If you have devised a coding scheme for your data, Phonometrica lets you define a **coding protocol**. A coding protocol is a description of your
coding scheme which offers a user-friendly interface for querying your data; it tells Phonometrica what to look for and how to present the
information to the user in the query editor. Phonometrica will automatically load all valid coding protocols in your plugin's submenu.

A coding protocol defines a number of **fields** which can take on a number of values. The user is presented with a number of checkboxes for each
field, and Phonometrica converts the query to the corresponding regular expression, as defined by the coding protocol. Coding protocols are written
in JSON.  Here is a simple yet realistic example, drawn from the `PFC project <http://www.projet-pfc.net>`_:

.. code:: json

    {
        "type": "coding_protocol",
        "name": "Schwa coding",
        "version": "0.1",
        
        "field_separator": "",
        "layer_index": 2,
        "fields_per_row": 3,

        "fields": [
            {"name": "Spelling", "match_all": ".",
                "values": [
                {"match": "e", "text": "graphical e"},
                {"match": "[^e]", "text": "no e"}
                ]
            },

            {"name": "Schwa", "match_all": "[0-2]",
                "values": [
                {"match": "0", "text": "Absent"},
                {"match": "1", "text": "Present"},
                {"match": "2", "text": "Uncertain"}
                ]
            },
            
            {"name": "Position", "match_all": "[1-5]", 
                "values": [
                    {"match": "1", 	"text": "monosyllable"},
                    {"match": "2", 	"text": "initial syllable"},
                    {"match": "3", 	"text": "median syllable"},
                    {"match": "4", 	"text": "final syllable"},
                    {"match": "5", 	"text": "metathesis"}
                ]
            },
            
            
            {"name": "Left context", "match_all": "[1-5]",
                "values": [
                    {"match": "1", 	"text": "vowel"},
                    {"match": "2", 	"text": "consonant"},
                    {"match": "3", 	"text": "start of an intonational phrase"},
                    {"match": "4", 	"text": "uncertain vowel"},
                    {"match": "5", 	"text": "simplified cluster"}
                ]		
            },
            
            {"name": "Right context", "match_all": "[1-4]",
                "values": [
                    {"match": "1", "text": "vowel"},
                    {"match": "2", "text": "consonant"},
                    {"match": "3", "text": "weak prosodic boundary"},
                    {"match": "4", "text": "strong prosodic boundary"}
                ]
            }   
        ]
    }

The ``type`` field is required and indicates that this file is a coding protocol. The ``name`` field corresponds to the name of the grammar, as it 
will be seen by the user, and ``version`` is an optional field which corresponds to the version of the protocol.

Next, the ``field_separator`` is an optional attribute which indicates the separator to be used between fields. In this case, it is an empty string, 
which means that the fields are concatenated directly (e.g. ``1412``). If the separator
was ``_``, for instance, each field should be separated by this symbol (e.g. ``1_4_1_2``). 

Next the ``layer_index`` attribute indicates the index of the layer in which codings
should be searched for. The value is used to search in all annotation layers.

The following attribute, ``fields_per_row``, lets us specify how many fields should be displayed in a row. In our case, since there are 4
fields, we decide to distribute them across 2 rows containing 2 fields each. The default value is 3.

Finally, the ``fields`` attribute contains an array of fields, each of them corresponding to a JSON object. The ``name`` attribute provides a
descriptive label for the field. The ``match_all`` attribute is a partial regular expression that should match all possible values for the field. If a
user doesn't check any value for a field, this attribute will be used to retrieve all possible values. The ``values`` attribute contains an array
of values. Each of them contains (at least) a ``match`` attribute, which is a string corresponding to the value, and a ``text`` attribute which
is the label that will be displayed in the user interface for the corresponding value, along with a check box. Note that leaving all values unchecked
has the same effect as checking them all.


