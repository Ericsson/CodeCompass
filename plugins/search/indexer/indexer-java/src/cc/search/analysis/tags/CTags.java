package cc.search.analysis.tags;

import java.io.BufferedReader;
import java.io.IOException;
import java.io.InputStreamReader;
import java.io.OutputStreamWriter;
import java.util.ArrayList;
import java.util.Collection;
import java.util.logging.Level;
import java.util.logging.Logger;

import cc.search.analysis.Location;
import cc.search.indexer.Context;

/**
 * A ctags process instance.
 */
class CTags implements TagGenerator {
  /**
   * Logger.
   */
  private final static Logger _log  = Logger.getLogger(CTags.class
    .getName());
  /**
   * Filter terminator string for ctags.
   */
  private final static String TERMINATOR = "!!!CC_END_OF_TAGS!!!";
  /**
   * CTags process instance.
   */
  private final Process _ctagsProcess;
  /**
   * Input stream for file name input.
   */
  private final OutputStreamWriter _ctagsInput;
  /**
   * Tag output of the process.
   */
  private final BufferedReader _ctagsOutput;
  /**
   * CTags error thread.
   */
  private final Thread _ctagsErrorThread;

  /**
   * Starts a ctags process without any extra parameter.
   *
   * @throws IOException
   */
  CTags() throws IOException {
    this(new ArrayList<String>(0));
  }

  /**
   * Starts a ctags process with the given extra parameters. The given
   * parameters will be appended to the end of the standard command line
   * parameters.
   *
   * @param extraParameters_ additional parameters for ctags (i.e.: kinds).
   * @throws IOException
   */
  CTags(final Collection<String> extraParameters_) throws IOException {
    final ArrayList<String> commandLine = new ArrayList<>(
      5 + extraParameters_.size());

    // Basic parameters
    commandLine.add("ctags");
    commandLine.add("--filter=yes");
    commandLine.add("--filter-terminator=" + TERMINATOR + "\n");
    commandLine.add("--excmd=number");
    commandLine.add("--fields=-f+aKzn");
    commandLine.add("--C-kinds=+px");
    commandLine.add("--C++-kinds=+px");
    commandLine.add("--Java-kinds=+l");
    commandLine.add("--Perl-kinds=+d");
    commandLine.add("--SQL-kinds=+dlr");

    // Extra parameters
    for (final String param : extraParameters_) {
      commandLine.add(param);
    }

    // Start ctags
    final ProcessBuilder builder = new ProcessBuilder(commandLine);
    _ctagsProcess = builder.start();
    _ctagsInput = new OutputStreamWriter(_ctagsProcess.getOutputStream());
    _ctagsOutput = new BufferedReader(new InputStreamReader(
      _ctagsProcess.getInputStream()));

    _ctagsErrorThread = new Thread(new Runnable() {
      @Override
      public void run() {
        StringBuilder errStr = new StringBuilder();

        try (final BufferedReader errStream = new BufferedReader(
          new InputStreamReader(_ctagsProcess.getErrorStream()))) {
          String line = errStream.readLine();
          while (line != null) {
            errStr.append(line);
            errStr.append('\n');
            line = errStream.readLine();
          }
        } catch (IOException e) {
          _log.log(Level.SEVERE, "Error while reading ctags error stream!",e);
        }

        if (errStr.length() > 0) {
          _log.log(Level.SEVERE, "ctags error: {0}", errStr.toString());
        }
      }
    });
    _ctagsErrorThread.setDaemon(true);
    _ctagsErrorThread.start();
  }

  @Override
  public void close() {
    try {
      _ctagsInput.close();
    } catch (IOException ex) {
      _log.log(Level.WARNING, "Closing ctags input failed!", ex);
    }

    try {
      _ctagsOutput.close();
    } catch (IOException ex) {
      _log.log(Level.WARNING, "Closing ctags output failed!", ex);
    }

    _ctagsProcess.destroy();
  }

  /**
   * @return true if ctags is still running, false if not
   */
  public boolean isRunning() {
    try {
      _ctagsProcess.exitValue();
      return false;
    } catch (IllegalThreadStateException e) {
      return true;
    }
  }

  /**
   * Tag the given file with ctags.
   *
   * @param file_ File path.
   * @throws IOException
   */
  private void tagFile(String filePath_) throws IOException {
    _ctagsInput.write(filePath_ + "\n");
    _ctagsInput.flush();
  }

  /**
   * Parses a ctags kind.
   *
   * @param kind_ ctags kind as string.
   * @return a parsed kind.
   * @throws IOException
   */
  private Tag.Kind parseKind(String kind_) throws IOException {
    switch (kind_) {
      case "struct":
      case "class":
      case "interface":
      case "typedef":
      case "union":
      case "enum":
      case "table": // SQL table
      case "type":
      case "record":
      case "Exception":
      case "namelist": // fortran
        return Tag.Kind.Type;
      case "define":
      case "macro":
        return Tag.Kind.Macro;
      case "subroutine":
      case "function":
      case "method":
      case "procedure":
      case "singleton method":
      case "jsfunction":
      case "Constructor":
      case "entry": // fortran
        return Tag.Kind.Function;
      case "prototype":
      case "subroutine declaration":
      case "event":
      case "section":
      case "command":
        //return Tag.Kind.Prototype;
        throw new IOException("Tag type not supported!");
      case "member":
      case "field":
      case "property":
        return Tag.Kind.Field;
      case "externvar":
      case "variable":
      case "local":
      case "external variable declarations":
      case "var":
        return Tag.Kind.Variable;
      case "enumerator":
      case "constant":
      case "enum constant":
      case "null":
        return Tag.Kind.Constant;
      case "label":
        return Tag.Kind.Label;
      case "namespace":
      case "package":
      case "module":
      case "component": // fortran
      case "program": // fortran
      case "block data": // fortran
      case "common": // fortran
        return Tag.Kind.Module;
      default:
        _log.log(Level.WARNING, "Unknown kind: {0}", kind_);
        return Tag.Kind.Other;
    }
  }

  /**
   * Parses a ctags tag.
   *
   * @param context_ indexing context.
   * @param tagText_ tag as it is in the tags file.
   * @param tagAttributes_ the attribute part of th line.
   * @return parsed tag or null on non-fatal error.
   */
  private Tag parseTag(Context context_, String tagText_, String tagAttributes_) {
    if (tagText_.startsWith("operator ")) {
      // Workaround for C++ operators
      return null;
    }

    int lineNumber = -1;
    String lineNumberStr = null;
    String kind = null;

    int attrStart = 0;
    while (attrStart < tagAttributes_.length()) {
      int attrNameEnd = tagAttributes_.indexOf(':', attrStart);
      if (attrNameEnd < 0) {
        _log.log(Level.WARNING, "Malformed attribute: {0}",
          tagAttributes_.substring(attrStart));
        return null;
      }

      int attrValueEnd = tagAttributes_.indexOf('\t', attrNameEnd + 1);
      if (attrValueEnd < 0) {
        // last value
        attrValueEnd = tagAttributes_.length();
      }

      final String attrValue = tagAttributes_.substring(
        attrNameEnd + 1, attrValueEnd);
      switch (tagAttributes_.substring(attrStart, attrNameEnd)) {
        case "line":
          lineNumberStr = attrValue;
          lineNumber = Integer.parseInt(lineNumberStr);
          break;
        case "kind":
          kind = attrValue;
          break;
      }

      attrStart = attrValueEnd + 1;
    }

    // Check values
    if (lineNumberStr == null || lineNumberStr.isEmpty()) {
      _log.log(Level.WARNING, "Missing line number!");
      return null;
    } else if (lineNumber < 1) {
      _log.log(Level.WARNING, "Bad line number {0} (value: {1}, file: {2})!",
        new Object[]{lineNumberStr, lineNumber,
          context_.getFileFullPath()});
      return null;
    } else if (kind == null || kind.isEmpty()) {
      _log.log(Level.WARNING, "Missing tag kind!");
      return null;
    }

    int startColumn;
    try {
      // Calculate location
      // FIXME: what if there is more than one match in this line???
      startColumn = context_.lineInfos.getLineContent(lineNumber).
        indexOf(tagText_);
      if (startColumn < 0) {
        // it could be an something special.
        _log.log(Level.FINER, "'{0}' not found in file '{2}' at line {1}!",
          new Object[]{tagText_, lineNumber,
            context_.getFileFullPath()});
        return null;
      }
    } catch (IndexOutOfBoundsException ex) {
      _log.log(Level.SEVERE, "Possibly bad line number: ''{0}'' for file " +
        "''{1}''", new Object[]{lineNumberStr,
          context_.getFileFullPath()});
      return null;
    }

    startColumn += 1;
    final Location loc = new Location(lineNumber, startColumn, startColumn +
      tagText_.length() - 1);
    try {
      return new Tag(loc, tagText_, kind, parseKind(kind));
    } catch (IOException ex) {
      // Parsing tag (kind) failed / not supported tag!
      return null;
    }
  }

  /**
   * Processes a file (previously parsed by tagFile) with ctags.
   *
   * @param tags_ container for tags.
   * @param context_ indexing context.
   * @throws IOException
   */
  private void processTags(Tags tags_, Context context_) throws IOException {
    while (true) {
      String outLine = _ctagsOutput.readLine();

      if (outLine == null) {
        // WTF: EOF
        _log.log(Level.WARNING, "Unexpected EOF!");
        return;
      }

      if (outLine.equals(TERMINATOR)) {
        // yey, terminator found :-)
        break;
      } else if (outLine.endsWith(TERMINATOR)) {
        _log.log(Level.WARNING, "Incomplete tag file!");
        break;
      }

      if (outLine.startsWith("ctags:")) {
        // This is an error/warning message from ctags.
        _log.log(Level.WARNING, "ctags message: {0}", outLine);
        continue;
      }

      int pos = outLine.indexOf('\t');
      if (pos <= 0) {
        _log.fine("Skipping a line.");
        continue;
      }

      final String tag = outLine.substring(0, pos);
      pos = outLine.indexOf(";\"\t", pos + 1);
      if (pos <= 0) {
        _log.warning("Skipping a line: has no attributes.");
        continue;
      }

      Tag parsedTag = parseTag(context_, tag, outLine.substring(pos + 3));
      if (parsedTag != null) {
        tags_.add(parsedTag, context_.lineInfos.getLineStartOffset(
          parsedTag.location.line) + parsedTag.location.startColumn - 1);
      }
    }
  }

  @Override
  public void generate(Tags tags_, Context context_) throws IOException {
    tagFile(context_.getFileFullPath());
    processTags(tags_, context_);
  }
}
