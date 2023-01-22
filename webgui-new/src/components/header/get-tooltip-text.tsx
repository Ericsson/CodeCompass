import { SearchMethods } from '../../enums/settings-enum';

export const getTooltipText = (value: string): string | JSX.Element => {
  switch (value) {
    case SearchMethods.FILE_REGEX:
      return (
        <>
          This filter is a regular expression of file names.
          <br />
          Example: .*cpp
        </>
      );
    case SearchMethods.PATH_REGEX:
      return (
        <>
          This filter is a regular expression of directory paths.
          <br />
          Example: /path/to/source/subdir/.*
        </>
      );
    case SearchMethods.EXPRESSION:
      return (
        <>
          <div>
            For <b>&quot;Text search&quot;</b> and <b>&quot;Definition search&quot;</b> you can use wildcards, regex,
            and logical operators:
          </div>
          <ul>
            <li>
              <b>Phrases:</b> a Phrase is a group of words surrounded by double quotes such as &quot;hello dolly&quot;.
              A Phrase query matches if the words are next to each other in the particular order.
            </li>
            <li>
              <b>Wildcards:</b> single and multiple character wildcard searches within single terms.
              <br />
              Examples: *test te?t test* te*t
              <br />
              <b>Note:</b> you cannot use wildcard searches within phrase queries!
            </li>
            <li>
              <b>Regular Expressions:</b> matching a pattern between forward slashes.
              <br />
              Example: /[mb]oat/
            </li>
            <li>
              <b>Boolean Operators:</b> Boolean operators allow terms to be combined through logic operators.
              <br />
              We support AND, &quot;+&quot;, OR, NOT and &quot;-&quot; as Boolean operators. (Note: Boolean operators
              must be ALL CAPS)
              <ul>
                <li>
                  <b>OR:</b> The OR operator links two terms and finds a matching document if either of the terms exist
                  in a document. If there is no Boolean operator between two terms, the OR operator is used.
                  <br />
                  Example: &quot;some text&quot; OR other
                </li>
                <li>
                  <b>AND:</b> The AND operator matches documents where both terms exist anywhere in the text of a single
                  document.
                  <br />
                  Example: &quot;some text&quot; AND other
                </li>
              </ul>
            </li>
            <li>
              <b>Escaping Special Characters:</b> The current list special characters are: + - && || ! ( ) {} [ ] ^
              &quot; ~ * ? : \ /
              <br />
              <b>The escape character is the &quot;\&quot; (back slash).</b>
            </li>
          </ul>
          <div>
            In <b>&quot;File name search&quot;</b> you can use a regular expression for matching on full (absolute) path
            of files in the database.
            <br />
            Example: /path/.*/te?t\.cpp
          </div>
          <div>
            For the documentation of <b>&quot;Log search&quot;</b> please see the user manual.
          </div>
        </>
      );
    case 'Any':
      return (
        <>
          In case of <i>Any</i>, search happens in every file, not only in listed ones.
        </>
      );
    default:
      return <></>;
  }
};
