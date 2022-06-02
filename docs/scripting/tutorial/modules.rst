.. _modules:

Modules
=======

Overview
--------


When you start writing scripts that are relatively long and complex and/or that you would like to redistribute, you might want to break them down into smaller, 
reusable components. Modules offer a way to achieve that. A ``Module`` is an object that can be used to store unordered key/value pairs. Each pair represents a *field*. Conceptually, it is similar to a ``Table``, 
except that all its keys must be strings. To create a module, we can call the ``Module`` type's constructor, passing it as an argument the name we want to give to the 
module. (This name will be used in error messages.) We can then create and retrieve values with the index operator, just like for tables:

.. code:: phon

    m = Module("My first module")
    m["version"] = "0.1"
    print m["version"]

Alternatively, we can also use the *dot operator*:

.. code:: phon

    print m.version
    m.greet = function() print "hello" end
    m.greet()     # call module function using the dot operator
    m["greet"]()  # call module function using the index operator


As you can see in the above example, the dot operator and the index operator are equivalent: the dot operator is shorter and more legible, whereas the index operator
is more flexible since it allows you to create keys dynamically:

.. code:: phon

    keys = ["a", "b", "c"]
    foreach key in keys do
        m[key] = to_upper(key)
    end

    print m.a         # prints "A"
    print m[keys[-1]] # prints "C"


Modules are mostly used to create *namespaces*: instead of creating variables and functions in the global scope, you can pack them in a module. This makes your code 
easier to redistribute and reuse, since other users will be able to *import* your module. It also avoids "polluting" the global scope with your own variables.


Creating modules
----------------

Let's create a module called ``utils`` in which we will put some utility functions. We will create a file named ``utils.phon`` for this purpose. Remember that whenever
you create a script, all variables and functions that are at the top level and that are not declared as ``local`` are global. If a user were to import our file that 
contains global variables, some of our variables might conflict with the user's, which might lead to errors (in the best case) or to very subtle bugs (in the worst case). 
To prevent that, we declare all our variables as local, and only *export* a single module. Our users will then be able to safely *import* this module and only use the 
modules they want, without any name conflict. Let's first create the module:

.. code:: phon

    # utils.phon
    local m = Module("utils")
    m.version = "0.1"
    m.author = "John Smith"

    # Calculate the perimeter of a rectangle
    m.perimeter = function(length as Number, width as Number)
        return 2 * length + 2 * width
    end

    # Calculate the area of a rectangle
    m.area = function(length as Number, width as Number)
        return length * width
    end

    # Export the module
    return m

In this example, we first create an empty module and assign it to the *local* variable ``m``, next we populate our module with a couple of variables and functions, 
and finally we return the module ``m``. You might be surprised by this ``return`` statement at the end of the file, since it is outside of any function definition. 
However, when Phonometrica loads a script, it will put all the top-level code in an implicitly defined function, and will then execute it. Therefore, a script *is*
a function, and all scripts return a value (``null`` by default). When a user imports our script, the value that will be returned to them will be our module. 


Importing modules
-----------------

We will now create a script called ``main.phon`` in the same directory as ``utils.phon``, and we will now import and use the module we created earlier. This is done with the function 
``import``, which takes the name of the file to be imported, without the ``.phon`` extension.

.. code:: phon

    # main.phon
    local utils = import("utils")
    local l = 100
    local w = 20
    print "The perimeter of a rectangle with length = ", l, " and width = ", w, " is ", utils.perimeter(l, w)
    print "The area of a rectangle with length = ", l, " and width = ", w, " is ", utils.area(l, w)


The function ``import`` does several things. First, it looks for a Phonometrica script with the name that was passed as its argument in the same directory as the 
current script and, if this fails, it tries to find it in one of its standard *import paths*. If it finds such a file, it checks whether this file has already
been loaded: if so, it simply returns the script's return value, otherwise it loads the script and then returns its value. This ensures that the script is only 
executed once even if it is imported several times from different places. 

In our main script, we assign the result of ``import`` to a global variable named ``utils``, which now stores the module. We then call the functions ``perimeter`` and 
``area`` from this module. There is no obligation give the variable the same name as the module: if we wanted to save some typing, we could have called the variable ``u``,
for instance. In addition, note that we defined ``utils`` as a global variable: this is fine since this is our main script, but we could have also defined it as a 
local variable: this would be good practice if this file was also meant to be imported by other users. 

Reloading modules
-----------------

As mentioned above, ``import`` checks whether a script has already been loaded to avoid re-executing its code every time it is imported. In general, this is what you 
want, but sometimes you might want to ensure that the module *is* reloaded. The most common scenerio is when you make some changes to a module and you want to import it from the 
console: in that case, you want to ensure that Phonometrica uses the latest version of your module, even if it has already been imported. The function ``import`` accepts
a Boolean as a second argument: if the value is ``true``, it will force reloading the module even if it has already been imported:

.. code:: phon

    # ensure that our version of utils is not stale
    local utils = import("utils", true)


Distributing modules as plugins
-------------------------------

When Phonometrica loads a plugin, its ``Scripts`` directory is automatically added to the the search path for modules. This means that you can 
put your own modules in this directory and access them from your own scripts, but it also means that other users will be able to load your module 
using its base name (without the ``.phon`` extension). 

In order to avoid conflicts with other modules, it is recommended to give them a unique name. You could for instance use a prefix which is 
specific to your plugin. As an example, a utility module for a project named PFC could be named ``pfc-utils.phon``, and a user could load as follows:

.. code:: phon

    local utils = import("pfc-utils")



Redistributing scripts
----------------------

If you intend to redistribute a script or module, we strongly recommend that you adhere to the following guidelines:

- unless you really need to define global variables, declarea all top-level variables as local so as not to pollute the global namespace
- if your script is intended to be imported as a module, pack all the symbols you want to export in a ``Module`` object and return it at the end of your script
- for all exported variables and fields, use ``snake_case`` rather than ``camelCase`` or ``PascalCase``; for example, use ``validate_item`` instead of ``validateItem`` or ``ValidateItem``
- provide an explicit type for function parameters
- prefer names that are explicit, even if they are a bit longer, to names that are short but possibly difficult to understand; for example, ``list_directory`` is clearer than ``listdir`` or (worse) ``lsdir``

Following these rules will ensure that your code is easy to understand and works in a consistent and predictable way. 
