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

.. function:: get_user_directory()
    
Get the user's directory (read only).    
    
    
------------

.. function:: get_current_directory()
    
Get or set the current directory.    
    
.. code:: phon
   
   system.current_directory = "/home/julien/Documents"
   print(system.current_directory) 


------------

.. function:: get_full_path(relative_path)  
    
    
Turn a relative path into an absolute path.


------------

.. function:: get_path_separator()

Get the native path separator on the current platform. This is ``\\`` on Windows and ``/`` on other platforms.

------------

.. function:: get_temp_directory()
    
Get the name of a directory where temporary files can be written.

------------

.. function:: get_temp_name()  
    
Return the name of a temporary file which is guaranteed to be unique.

------------

.. function:: get_base_name(path as String)  
    
Return the file name part of ``path``, stripping all directories.

.. code:: phon

	print get_base_name("/home/hodor/test.txt") # prints "test.txt"
	

------------

.. function:: create_directory(path as String)  
    
Create a new directory. If the directory could not be created, an error is thrown.

------------

.. function:: remove_directory(path as String)  
    
Remove a directory, which must be empty. If the directory could not be removed, an error is thrown.

------------

.. function:: remove_directory(path as String, recursive as Boolean)  
    
Remove a directory. If ``recursive`` is ``true``, the content is removed resurively; otherwise, the directory is assumed to be empty. If the directory could not be removed, an error is thrown.

------------

.. function:: remove_file(path as String)  

Remove a file. If the file could not be removed, an error is thrown.

------------

.. function:: remove(path as String)  
    
Remove ``path``, which can be either a file or a directory. If ``path`` could not be removed, an error is thrown.

------------

.. function:: list_directory(path as String)

Return a table containing the files in ``path``. Hidden files are not included.

------------

.. function:: list_directory(path as String, include_hidden as Boolean)

Return a table containing the files in ``path``. If ``include_hidden`` is ``true``, hidden files are included. If it is ``false`` or is missing,
hidden files are not included.

------------

.. function:: exists(path as String)

Return ``true`` if the path exists, ``false`` otherwise.


------------

.. function:: is_document(path as String)

Return ``true`` if ``path`` exists and is a file, ``false`` otherwise.

------------

.. function:: is_directory(path as String)


Return ``true`` if ``path`` exists and is a directory, ``false`` otherwise.
    
------------

.. function:: clear_directory(path as String)

Empty the content of a directory.

------------

.. function:: get_extension(path as String)

Get the file's extension, starting with a dot.

------------

.. function:: strip_extension(path as String)

Return ``path`` without extension.

------------

.. function:: split_extension(path as String)

Return a table whose first element is ``path`` with the extension removed, and whose second element is the extension.

------------

.. function:: join_path(s1 as String, s2 as String)

Concatenate ``s1`` and ``s2`` using the native path separator and returns the resulting path name.


------------

.. function:: genericize(path as String)

On Windows, this function converts the native path separator to the generic separator ``/``. On platforms that use the generic separator, it does nothing.

------------

.. function:: nativize(path as String)

On Windows, this function converts the generic path separator to the native separator ``\\``. On platforms that use the generic separator, it does nothing.

 
------------

.. function:: rename(old_name as String, new_name as String)

Renames a file. If the file could not be renamed, an error is raised.



