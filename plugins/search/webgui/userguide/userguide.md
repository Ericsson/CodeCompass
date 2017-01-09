\section userguide_search Search

\subsection userguide_search_module Search
In the [search](#userguide_search) header we can search for text phrases in the
whole source code.

![Search](images/search.png)

\subsection userguide_search_types Search types
CodeCompass supports the following types of search: Text, Definition, File name
and Log.

\subsubsection userguide_search_text_search Text Search
A query is broken up into terms and operators. There are two types of terms:
Single Terms and Phrases. A Single Term is a single word such as "test" or
"hello". A Phrase is a group of words surrounded by double quotes such as
"hello dolly". Multiple terms can be combined together with Boolean operators 
to form a more complex query (see below).

With "Text Search" you can search for terms from indexed documents. If there is
no Boolean operator(s) between two words (terms), the OR operator is used.

#### Text Search syntax
For Text Search you can use wildcards, regex and logical operators:

##### Escaping Special Characters
You can (and should) escape the following characters with a back slash "\"
character:

    +  -  &&  ||  !  (  )
    {  }  [  ]  ^  "  ~  *
    ?  \  /

##### Phrases
A Phrase is a group of words surrounded by double quotes such as "hello dolly".
A Phrase query matches iff the words are next to each other in the particular
order.

##### Wildcards
You can use single and multiple character wildcard searches within single terms
but you cannot use wildcard searches within phrase queries.

Example:

    *test => matches documents that contains word(s) that ends with test.


More examples:

    te?t
    test*
    te*t


In phrases you cannot use wildcards (the wildcard characters are parsed as
simple characters):

    "te*t text" => in this case the * is not a wildcard!

##### Regular Expression
You can use a regular expression pattern between forward slashes.

Example:

    /[mb]o.*at/

##### Boolean operators
Boolean operators allow terms to be combined through logic operators.

###### OR
The OR operator links two terms and finds a matching document if either of the
terms exist in a document. **If there is no Boolean operator between two terms,
the OR operator is used.**

Example:

    "some text" OR other

###### AND
The AND operator matches documents where both terms exist anywhere in the text
of a single document.

Example:

    "some text" AND other

###### Love (+)
The "+" or required operator requires that the term after the "+" symbol exist
somewhere in a the document.

To search for documents that must contain "include" and may contain "main" use
the query:

    +include main

###### Hate (-)
The "-" or prohibit operator excludes documents that contain the term after the
"-" symbol.

To search for documents that contain "print hello" but not "main" use the
query:

    "print hello" -main

\subsubsection userguide_search_definition Definition Search
You can search for CTags tags (definitions) and for C++ qualified names with
"Definition Search". **The query syntax is the same as in "Text Search"**.

You can use the **Type** filter by clicking to the **Settings** button.

\subsubsection userguide_search_filename_search File Name Search
With "File name" search you can use a regular expression (Perl syntax) for
matching  on full (absolute) path of files in the database.

Example:

    /path/.*/te?t\.cpp

\subsubsection userguide_search_log Log Search
With log search you search for the code that probably creates the given log
record. It has no special syntax just copy and paste the log message to the
search field and hit the search button.

Example:

    [DEBUG] Mon Feb 20 13:30:24 2017 Connection from 127.0.0.1:33262 requested URI: <projectname>/ProjectService

\subsection userguide_search_filtering Filtering
For more accurate (and faster) results you can filter by programming language,
file name and by directory.

+ **Note 1:** In file name search you can't use any filter.
+ **Note 2:** The "Type" filter only works with "Definition Search".

### File filtering:
+ To the **File Filter** field you can write regular expression which is
  matched with the name of every relevant file during the query.
+ To the **Directory Filter** field you can also write regular expression which
  is mathced with the full path of the parent directory of a relevant file.
+ A file must match to the search query and with both filters (an empty field
  is like \.\*).

### Programming language filter (mime-type filter):
You can filter to files by its mime-type. The main usage of this feature is
filtering to files that wirtten in a given programming language. By clickig to
the **Settings** button you can set the language filter (the default is *ANY*).