Operating System
================

This page documents the ``system`` module.

General concepts
----------------

This module lets you interact with the operating system, and perform operations such as checking whether a file
or directory exists, extracting parts of a file name, etc.


Functions
---------


.. module:: system

------------

.. function:: get_application_directory()

Returns the path of the directory (inside the user's directory) where application settings are usually written to.

------------

.. attribute:: user_directory
    
Get the user's directory (read only).    
    
    
------------

.. attribute:: current_directory
    
Get or set the current directory.    
    
.. code:: phon
   
   system.current_directory = "/home/julien/Documents"
   print(system.current_directory) 


------------

.. function:: get_full_path(relative_path)  
    
    
Turns a relative path into an absolute path.


------------

.. attribute:: separator

Get the native path separator on the current platform. This is ``\\`` on Windows and ``/`` on other platforms.

------------

.. attribute:: temp_directory  
    
Get the name of a directory where temporary files can be written.

------------

.. function:: get_temp_name()  
    
Returns the name of a temporary file which is guaranteed to be unique.

------------

.. function:: get_base_name(path)  
    
Returns the file name part of ``path``, stripping all directories.

.. code:: lua

	print(os.get_base_name("/home/hodor/test.txt")) -- prints "test.txt"
	

------------

.. function:: create_directory(path)  
    
Create a new directory. If the directory could not be created, an error is thrown.

------------

.. function:: remove_directory(path)  
    
Recursively remove a directory. If the directory could not be removed, an error is thrown.

------------

.. function:: remove_file(path)  

Remove a file. If the file could not be removed, an error is thrown.

------------

.. function:: remove(path)  
    
Remove ``path``, which can be either a file or a directory. If ``path`` could not be removed, an error is thrown.


.. function:: list_directory(path [, include_hidden])

Returns a table containing the files in ``path``. If ``include_hidden`` is ``true``, hidden files are included. If it is ``false`` or is missing,
hidden files are not included.

------------

.. function:: exists(path)

Returns ``true`` if the path exists, ``false`` otherwise.


------------

.. function:: is_file(path)

Returns ``true`` if ``path`` exists and is a file, ``false`` otherwise.

------------

.. function:: is_directory(path)


Returns ``true`` if ``path`` exists and is a directory, ``false`` otherwise.
    
------------

.. function:: clear_directory(path)

Empty the content of a directory.

------------

.. function:: get_extension(path)

Get the file's extension, starting with a dot.

------------

.. function:: strip_extension(path)

Returns ``path`` without extension.

------------

.. function:: split_extension(path)

Returns a table whose first element is ``path`` with the extension removed, and whose second element is the extension.

------------

.. function:: build_path(...)

Concatenate the arguments using the native path separator and returns the resulting path name.

------------

.. function:: genericize(path)

On Windows, this function converts the native path separator to the generic separator``"/". On platforms that use the generic separator, it does nothing.

------------

.. function:: nativize(path)

On Windows, this function converts the generic path separator to the native separator``"\\". On platforms that use the generic separator, it does nothing.

 
------------

.. function:: rename(old_name, new_name)

Renames a file. If the file could not be renamed, an error is raised.



