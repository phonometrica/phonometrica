static std::vector<std::pair<const char*, std::vector<wxString>>> function_declarations = {
	{ "clear",  {
		"clear(ref table as Table)\nRemoves all the elements in the table.\002",
		"clear(ref list as List)\nEmpty the content of the list.\001"
	}},
	{ "contains",  {
		"contains(table as Table, key as Object)\nReturns `true` if there is an element in the table whose key is equal to `key`, and `false` otherwise.\002",
		"contains(list as List, item as Object)\nReturns `true` if `item` is in the list and `false` otherwise.\001\002",
		"contains(string as String, substring as String)\nReturns `true` if `string` contains `substring`, and `false`\notherwise.\001"
	}},
	{ "get",  {
		"get(table as Table, key as Object)\nReturns the value associated with `key` if there is one, or `null` if there is no such value.\002",
		"get(table as Table, key as Object, default as Object)\nReturns the value associated with `key` if there is one, or `default` if there is no such value.\001"
	}},
	{ "is_empty",  {
		"is_empty(table as Table)\nReturns `true` if the table contains no element, and `false` otherwise.\002",
		"is_empty(list as List)\nReturns `true` if the list doesn't contain any element, and `false` if it does.\001"
	}},
	{ "remove",  {
		"remove(ref table as Table, key as Object)\nRemoves the element whose key is equal to `key`.\002",
		"remove(ref list as List, item as Object)\nRemoves all the elements in the list that are equal to `item`.\001\002",
		"remove(ref string as String, sub as String)\nRemoves all (non-overlapping) instances of the substring `sub`.\001"
	}},
	{ "read_file",  {
		"read_file(path)\nReturn the content of the file named `path` as a string."
	}},
	{ "close",  {
		"close(file as File)\nCloses the file."
	}},
	{ "eof",  {
		"eof(file as File)\nReturns `true` if the cursor is positionned at the end of the file, `false` otherwise."
	}},
	{ "open",  {
		"open(path as String)\n\nOpens the file named `path` and returns a handle to it.\002",
		"open(path as String, mode as String)\n\nOpens the file named `path` and returns a handle to it.\001\002",
		"open(path as String, mode as String, encoding as String)\n\nOpens the file named `path` and returns a handle to it.\001"
	}},
	{ "rewind",  {
		"rewind(file as File)\nRewinds the cursor to the beginning of the file."
	}},
	{ "len",  {
		"len(file as File)\nReturns the number of bytes in the file.\002",
		"len(regex as Regex)\nReturns the number of captures in the last match.\001\002",
		"len(list as List)\nReturns the number of elements in the list.\001\002",
		"len(string as String)\nReturns the number of characters in the string.\001"
	}},
	{ "write",  {
		"write(file as File, text as String)\nWrites `text` to `file`."
	}},
	{ "write_line",  {
		"write_line(file as File, text as String)\nWrites `text` to `file`, and appends a new line separator."
	}},
	{ "write_lines",  {
		"write_lines(file as File, lines as List)\nWrites each string in `lines` to `file`, and appends a new line separator after each of them."
	}},
	{ "read_line",  {
		"read_line(file as File)\nReads a line from `file`."
	}},
	{ "read_lines",  {
		"read_lines(file as File)\nReturns the content of the file as a list whose elements are the lines of the file."
	}},
	{ "seek",  {
		"seek(file as File, pos as Integer)\nSets the position of the cursor in the file to `pos`."
	}},
	{ "tell",  {
		"tell(file as File)\nReturns the current position of the cursor in the file."
	}},
	{ "count",  {
		"count(regex as Regex)\nReturns the number of captures in the last match.\002",
		"count(string as String, substring as String)\nReturns the number of times `substring` appears in `string`.\001"
	}},
	{ "match",  {
		"match(regex as Regex, subject as String)\nMatch `regex` against the string `subject`.\002",
		"match(regex as Regex, subject as String, pos as Integer)\nMatch `regex` against the string `subject`, starting at position `pos`.\001"
	}},
	{ "has_match",  {
		"has_match(regex as Regex)\nReturns `true` if the last call to `match` was sucessful, and\n`false` if it was unsuccessful or if `match` was not called."
	}},
	{ "get_end",  {
		"get_end(regex as Regex, nth as Intger)\nReturns the index of the last character of the `nth` capture in `regex`."
	}},
	{ "get_start",  {
		"get_start(regex as Regex, nth as Integer)\nReturns the index of the first character of the `nth` capture in `regex`."
	}},
	{ "group",  {
		"group(regex as Regex, nth as Integer)\nReturns the `nth` captured sub-expression in the last successful call\nto `match`."
	}},
	{ "stringify",  {
		"stringify(object)\nConvert `object` to a string according to the JSON specification."
	}},
	{ "parse",  {
		"parse(str)\nConvert the string `str` to an object according to the JSON specification."
	}},
	{ "abs",  {
		"abs(x as Number)\nReturns the absolute value of `x`.\002",
		"abs(x as Array)\nReturns a copy of the array in which `abs` has been applied to each element.\001"
	}},
	{ "acos",  {
		"acos(x as Number)\nReturns the arccosine of `x`.\002",
		"acos(x as Array)\nReturns a copy of the array in which `acos` has been applied to each element.\001"
	}},
	{ "asin",  {
		"asin(x as Number)\nReturns the arcsine of `x`.\002",
		"asin(x as Array)\nReturns a copy of the array in which `asin` has been applied to each element.\001"
	}},
	{ "atan",  {
		"atan(x as Number)\nReturns the arctangent of `x`.\002",
		"atan(x as Array)\nReturns a copy of the array in which `atn` has been applied to each element.\001"
	}},
	{ "atan2",  {
		"atan2(y as Number, x as Number)\nReturns the four-quadrant inverse tangent of `y` and `x`."
	}},
	{ "ceil",  {
		"ceil(x as Number)\nReturns the smallest integer no smaller than `x`.\002",
		"ceil(x as Array)\nReturns a copy of the array in which `ceil` has been applied to each element.\001"
	}},
	{ "cos",  {
		"cos(x as Number)\nReturns the cosine of `x`.\002",
		"cos(x as Array)\nReturns a copy of the array in which `cos` has been applied to each element.\001"
	}},
	{ "exp",  {
		"exp(x as Number)\nReturns the exponential of `x`.\002",
		"exp(x as Array)\nReturns a copy of the array in which `exp` has been applied to each element.\001"
	}},
	{ "floor",  {
		"floor(x as Nubmer)\nReturns the largest integer that is no larger than `x`.\002",
		"floor(x as Array)\nReturns a copy of the array in which `floor` has been applied to each element.\001"
	}},
	{ "log",  {
		"log(x as Number)\nReturns the natural logarithm of `x`.\002",
		"log(x as Array)\nReturns a copy of the array in which `log` has been applied to each element.\001"
	}},
	{ "log2",  {
		"log2(x as Number)\nReturns the logarithm of `x` in base 2.\002",
		"log2(x as Array)\nReturns a copy of the array in which `log2` has been applied to each element.\001"
	}},
	{ "log10",  {
		"log10(x as Number)\nReturns the logarithm of `x` in base 10.\002",
		"log10(x as Array)\nReturns a copy of the array in which `log10` has been applied to each element.\001"
	}},
	{ "max",  {
		"max(x as Number, y as Number)\nReturns the larger value between `x` and `y`.\002",
		"max(x as Integer, y as Integer)\nReturns the larger value between `x` and `y`.\001"
	}},
	{ "min",  {
		"min(x as Number, y as Number)\nReturns the smaller value between `x` and `y`.\002",
		"min(x as Integer, y as Integer)\nReturns the smaller value between `x` and `y`.\001"
	}},
	{ "random",  {
		"random()\nReturns a pseudo-random value in the interval [0, 1[ according to a uniform distribution."
	}},
	{ "round",  {
		"round(x as Number)\nRounds `x` to the nearest integer.\002",
		"round(x as Array)\nReturns a copy of the array in which `round` has been applied to each element.\001"
	}},
	{ "sin",  {
		"sin(x as Number)\nReturns the sine of `x`.\002",
		"sin(x as Array)\nReturns a copy of the array in which `sin` has been applied to each element.\001"
	}},
	{ "sqrt",  {
		"sqrt(x as Number)\nReturns the square root of `x`.\002",
		"sqrt(x as Array)\nReturns a copy of the array in which `sqrt` has been applied to each element.\001"
	}},
	{ "tan",  {
		"tan(x as Number)\nReturns the tangent of `x`.\002",
		"tan(x as Array)\nReturns a copy of the array in which `tan` has been applied to each element.\001"
	}},
	{ "chi2_test",  {
		"chi2_test(X)\nComputes Pearson's chi-squared test on `X`, which must be a two-dimensional array."
	}},
	{ "corr",  {
		"corr(x, y)\nCalculates Pearson's correlation coefficient between samples `x` and `y`, which must be one-dimensional arrays with the same size."
	}},
	{ "cov",  {
		"cov(x, y)\nCalculates the covariance between samples `x` and `y`, which must be one-dimensional arrays with the same size."
	}},
	{ "f_test",  {
		"f_test(x, y [, alternative])\nComputes the F-test on `x` and `y` which must be one-dimensional arrays."
	}},
	{ "lm",  {
		"lm(y, X)\nFits a linear regression model."
	}},
	{ "logit",  {
		"logit(y, X [, max_iter])\nFits a logistic regression model."
	}},
	{ "mean",  {
		"mean(x [, dim])\nReturns the mean of the array `x`."
	}},
	{ "poisson",  {
		"poisson(y, X [, robust [, max_iter]])\nFits a Poisson regression model."
	}},
	{ "report_chi2",  {
		"report_chi2(X)\nComputes and reports Pearson's chi-squared test on `X`, which must be a two-dimensional array."
	}},
	{ "std",  {
		"std(x [, dim])\nReturns the standard deviation of the array `x`."
	}},
	{ "sum",  {
		"sum(x [, dim])\nReturns the sum of the elements in the array `x`."
	}},
	{ "t_test",  {
		"t_test(x, y [, equal_variance, [, alternative]])\nComputes a two-sample independent t-test for the mean between the samples `x` and `y`, which must be one-dimensional\narrays."
	}},
	{ "t_test1",  {
		"t_test1(x, mu [, alternative])\nComputes a one-sample t-test for the sample `x`, which must be a one-dimensional array."
	}},
	{ "vrc",  {
		"vrc(x [, dim])\nReturns the sample variance of the array `x`."
	}},
	{ "append",  {
		"append(ref list as List, item as Object)\nInserts `item` at the end of `list`.\002",
		"append(ref string as String, suffix as String)\nInserts `suffix` as the end of `string`.\001"
	}},
	{ "find",  {
		"find(list as List, item as Object)\n\nReturns the index of `item` in the list.\002",
		"find(list as List, item as Object, pos as Integer)\n\nReturns the index of `item` in the list, starting the search at index `pos`.\001\002",
		"find(string as String, substring as String)\nReturns the start position of `substring` in `string`, or 0 if it is not found.\001\002",
		"find(string as String, substring as String, pos as Integer)\nReturns the start position of `substring` in `string`, or 0 if it is not found.\001"
	}},
	{ "find_back",  {
		"find_back(list as List, item as Object)\n\nReturns the index of `item` in the list, starting the search from the end.\002",
		"find_back(list as List, item as Object, pos as Integer)\n\nReturns the index of `item` in the list, starting the search from the end at index `pos`.\001\002",
		"find_back(string as String, substring as String)\nReturns the start position of `substring` in `string`, or 0 if it is not found.\001\002",
		"find_back(string as String, substring as String, pos as Integer)\nReturns the start position of `substring` in `string`, or 0 if it is not found.\001"
	}},
	{ "insert",  {
		"insert(ref list as List, pos as Integer, item as Object)\nInserts the element `item` at index `pos`."
	}},
	{ "intersect",  {
		"intersect(list1 as List, list2 as List)\nReturns a new list which contains all the elements that are in both `list1` and `list2`."
	}},
	{ "is_sorted",  {
		"is_sorted(list as List)\nReturns true if all the elements are sorted in ascending order."
	}},
	{ "join",  {
		"join(items as List, delim as String)\nReturns a string in which all the elements in `items` have been joined with the separator `delim`."
	}},
	{ "pop",  {
		"pop(ref list as List)\nRemoves the last element from the list and returns it."
	}},
	{ "prepend",  {
		"prepend(ref list as List, item as Object)\nInserts `item` at the beginning of the list.\002",
		"prepend(ref string as String, suffix as String)\nInserts `suffix` as the beginning of `string`.\001"
	}},
	{ "remove_first",  {
		"remove_first(ref list as List, item as Object)\nRemoves the first element in the list that is equal to `item`.\002",
		"remove_first(ref string as String, sub as String)\nRemoves the first instance of the substring `sub`.\001"
	}},
	{ "remove_last",  {
		"remove_last(ref list as List, item as Object)\nRemoves the last element in the list that is equal to `item`.\002",
		"remove_last(ref string as String, sub as String)\nRemoves the last instance of the substring `sub`.\001"
	}},
	{ "remove_at",  {
		"remove_at(ref list as List, pos as Integer)\nRemove the element at index `pos`.\002",
		"remove_at(ref string as String, at as Integer, count as Integer)\nRemoves `count` characters, starting at position `at`.\001"
	}},
	{ "reverse",  {
		"reverse(ref list as List)\nReverses the order of the elements in the list.\002",
		"reverse(ref string as String)\n\nReverse all characters in the string.\001"
	}},
	{ "sample",  {
		"sample(list as List, n as Integer)\nReturns a list containing `n` elements from the list drawn at random."
	}},
	{ "shift",  {
		"shift(ref list as List)\nRemoves the first element from the list and returns it."
	}},
	{ "shuffle",  {
		"shuffle(ref list as List)\nRandomizes the order of the elements in the list."
	}},
	{ "slice",  {
		"slice(list as List, from as Integer, to as Integer)\nReturns a new list which contains the elements of the original list starting at index `from` and ending at index `to` (inclusive).\002",
		"slice(string as String, from as Integer)\nReturns the substring starting at index `from` until the end of the string.\001\002",
		"slice(string as String, from as Integer, to as Integer)\nReturns the substring starting at index `from` and ending\nat index `to` (inclusive).\001"
	}},
	{ "sort",  {
		"sort(ref list as List)\nSorts the elements in the list in increasing order."
	}},
	{ "sorted_find",  {
		"sorted_find(list as List, item as Object)\nFinds the index of `item` in a sorted list."
	}},
	{ "sorted_insert",  {
		"sorted_insert(ref list as List, item as Object)\nInserts `item` after the first element that is not less than elem."
	}},
	{ "str",  {
		"str(list as List)\nReturns a string representation of the list."
	}},
	{ "subtract",  {
		"subtract(list1 as List, list2 as List)\nReturns a new list which contains all the elements that are in `list1` but not in `list2`."
	}},
	{ "unite",  {
		"unite(list1 as List, list2 as List)\nReturns a new list which contains all the elements that are in `list1` and/or in `list2`."
	}},
	{ "char",  {
		"char(pos as Integer)\nGet character at position `pos`."
	}},
	{ "ends_with",  {
		"ends_with(string as String, suffix as String)\nReturns true if the string ends with `suffix`, and `false` otherwise."
	}},
	{ "left",  {
		"left(string as String, n as Integer)\nGet the substring corresponding to the `n` first characters of the\nstring."
	}},
	{ "ltrim",  {
		"ltrim(ref string as String)\nRemoves whitespace characters at the left end of the string."
	}},
	{ "replace",  {
		"replace(ref string as String, old as String, new as String)\nReplaces all (non-overlapping) instances of the substring `old` by `new`."
	}},
	{ "replace_at",  {
		"replace_at(ref string as String, at as Integer, count as Integer, new as String)\nReplaces `count` characters starting at position `at` with substring `new`."
	}},
	{ "replace_first",  {
		"replace_first(ref string as String, old as String, new as String)\nReplaces the first instance of the substring `old` with `new`."
	}},
	{ "replace_last",  {
		"replace_last(ref string as String, old as String, new as String)\nReplaces the last instance of the substring `old` with `new`."
	}},
	{ "right",  {
		"right(string as String, n as Integer)\nGet the substring corresponding to the `n` last characters of the\nstring."
	}},
	{ "rtrim",  {
		"rtrim(ref string as String)\nRemoves whitespace characters removed at the right end of the string."
	}},
	{ "split",  {
		"split(string as String, delim as String)\nReturns a List of strings which have been split at each occurrence of\nthe substring `delim`."
	}},
	{ "starts_with",  {
		"starts_with(string as String, prefix as String)\nReturns true if the string starts with `prefix`, and `false` otherwise."
	}},
	{ "to_lower",  {
		"to_lower(string as String)\nReturns a copy of the string where each character has been converted to\nlower case."
	}},
	{ "to_upper",  {
		"to_upper(string as String)\nReturns a copy of the string where each character has been converted to\nupper case."
	}},
	{ "trim",  {
		"trim(ref string as String)\nRemoves whitespace characters removed at both ends of the string."
	}},
	{ "read_matrix",  {
		"read_matrix(path [, separator [, drop_header]])\nReads a two-dimensional numeric array from a CSV file, in which values are separated by `separator` (by default, a comma)."
	}},
	{ "write_matrix",  {
		"write_matrix(M, path [, separator])\nWrites a two-dimensional numeric array `M` to a CSV file, in which values are separated by `separator` (by default, a comma)."
	}},
	{ "warning",  {
		"warning(message [, title])\nDisplays a warning dialog."
	}},
	{ "alert",  {
		"alert(message [, title])\nDisplays an error dialog."
	}},
	{ "info",  {
		"info(message [, title])\nDisplays an information dialog."
	}},
	{ "ask",  {
		"ask(message [, title])\nAsks a Yes/No question to the user."
	}},
	{ "get_input",  {
		"get_input(label, title, text)\nDisplays an input dialog whose title is `title` and whose informative text is `label`."
	}},
	{ "open_file_dialog",  {
		"open_file_dialog(message)\nDisplays a dialog that lets the user select a file."
	}},
	{ "save_file_dialog",  {
		"save_file_dialog(message)\nDisplays a dialog that lets the user choose a path to save a file."
	}},
	{ "open_directory_dialog",  {
		"open_directory_dialog(message)\nDisplays a dialog that lets the user select a directory."
	}},
	{ "set_status",  {
		"set_status(message, timeout)\nDisplays `message` in the status bar for `timeout` seconds."
	}},
	{ "view_text",  {
		"view_text(path [, title [, width]])\nOpens the plain text file `path` in a new dialog."
	}},
	{ "create",  {
		"create()\nCreate and return a new event identifier (id)."
	}},
	{ "connect",  {
		"connect(id, callback)\nConnect event `id` to function `callback`."
	}},
	{ "disconnect",  {
		"disconnect(id, callback)\nDisconnect event `id` from function `callback`."
	}},
	{ "emit",  {
		"emit(id, ...)\nEmit event `id`, followed by an optional argument."
	}},
	{ "get_application_directory",  {
		"get_application_directory()\nReturns the path of the directory (inside the user's directory) where application settings are usually written to."
	}},
	{ "get_path_separator",  {
		"get_path_separator()\nGet the native path separator on the current platform."
	}},
	{ "list_directory",  {
		"list_directory(path as String)\nReturn a table containing the files in `path`.\002",
		"list_directory(path as String, include_hidden as Boolean)\nReturn a table containing the files in `path`.\001"
	}},
	{ "exists",  {
		"exists(path as String)\nReturn `true` if the path exists, `false` otherwise."
	}},
	{ "is_file",  {
		"is_file(path as String)\nReturn `true` if `path` exists and is a file, `false` otherwise."
	}},
	{ "is_directory",  {
		"is_directory(path as String)\n\nReturn `true` if `path` exists and is a directory, `false` otherwise."
	}},
	{ "clear_directory",  {
		"clear_directory(path as String)\nEmpty the content of a directory."
	}},
	{ "get_extension",  {
		"get_extension(path as String)\nGet the file's extension, starting with a dot."
	}},
	{ "strip_extension",  {
		"strip_extension(path as String)\nReturn `path` without extension."
	}},
	{ "split_extension",  {
		"split_extension(path as String)\nReturn a table whose first element is `path` with the extension removed, and whose second element is the extension."
	}},
	{ "join_path",  {
		"join_path(s1 as String, s2 as String)\nConcatenate `s1` and `s2` using the native path separator and returns the resulting path name."
	}},
	{ "genericize",  {
		"genericize(path as String)\nOn Windows, this function converts the native path separator to the generic separator `/`."
	}},
	{ "nativize",  {
		"nativize(path as String)\nOn Windows, this function converts the generic path separator to the native separator `\\`."
	}},
	{ "rename",  {
		"rename(old_name as String, new_name as String)\nRenames a file."
	}},
	{ "get_annotations",  {
		"get_annotations()\nReturn a list of all the annotations in the current project."
	}},
	{ "get_selected_annotations",  {
		"get_selected_annotations()\nReturn a list of all the selected annotations in the current project."
	}},
	{ "get_annotation",  {
		"get_annotation(path)\nReturn the `Annotation` object from the current project whose path is `path`, or `null` if there is no such\nannotation."
	}},
	{ "get_current_annotation",  {
		"get_current_annotation()\nReturn the `Annotation` object loaded in the current view, or `null` if the current view is not an annotation view."
	}},
	{ "get_sounds",  {
		"get_sounds()\nReturn a list of all the sounds in the current project."
	}},
	{ "get_selected_sounds",  {
		"get_selected_sounds()\nReturn a list of all the selected sounds in the current project."
	}},
	{ "get_sound",  {
		"get_sound(path)\nReturn the `Sound` object from the current project whose path is `path`, or `null` if there is no such\nsound."
	}},
	{ "get_current_sound",  {
		"get_current_sound()\nReturn the `Sound` object loaded in the current view, or `null` if the current view is neither an annotation view\nnor a sound view."
	}},
	{ "get_window_duration",  {
		"get_window_duration()\n\nReturn the duration of the visible window in the current annotation or sound view."
	}},
	{ "get_selection_duration",  {
		"get_selection_duration()\nReturn the duration of the selection in the current annotation or sound view, or 0 if there is no selection."
	}},
	{ "report_intensity",  {
		"report_intensity(time)\nThis is a convenience function that displays the intensity at the given time in the current view."
	}},
	{ "report_pitch",  {
		"report_pitch(time)\nThis is a convenience function that displays the pitch at the given time in the current view."
	}},
	{ "report_formants",  {
		"report_formants(time)\nThis is a convenience function that displays the values of the visible formants at the given time in the current view."
	}},
	{ "hertz_to_bark",  {
		"hertz_to_bark(f)\nConverts frequency `f` (in Hertz) to bark."
	}},
	{ "bark_to_hertz",  {
		"bark_to_hertz(z)\nConverts frequency `z` (in bark) to Hertz."
	}},
	{ "hertz_to_erb",  {
		"hertz_to_erb(f)\nConverts frequency `f` (in Hertz) to ERB units."
	}},
	{ "erb_to_hertz",  {
		"erb_to_hertz(e)\nConverts frequency `e` (in ERB units) to Hertz."
	}},
	{ "hertz_to_mel",  {
		"hertz_to_mel(f)\nConverts frequency `f` (in Hertz) to mel."
	}},
	{ "mel_to_hertz",  {
		"mel_to_hertz(mel)\nConverts frequency `mel` (in mel) to Hertz."
	}},
	{ "hertz_to_semitones",  {
		"hertz_to_semitones(f0 [, ref])\nConverts frequency `f0` (in Hertz) to semitones, using `ref` as a reference frequency (in Hertz)."
	}},
	{ "semitones_to_hertz",  {
		"semitones_to_hertz(st)\nConverts the number of semitones `st` to Hertz, using `ref` as a reference frequency (in Hertz)."
	}}
};