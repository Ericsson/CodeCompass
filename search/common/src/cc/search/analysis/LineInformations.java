package cc.search.analysis;

import java.io.IOException;
import java.io.Reader;
import java.util.ArrayList;
import java.util.List;
import java.util.Map;
import java.util.NavigableMap;
import java.util.TreeMap;

/**
 * Utility class for working with line positions.
 */
public final class LineInformations {
  /**
   * Map from line number to its content.
   */
  private final List<String> _lines;
  /**
   * Map from line number to its start offset.
   */
  private final List<Integer> _lineToStartOffsets;
  /**
   * Map from start offset to line number.
   */
  private final NavigableMap<Integer, Integer> _startOffsetToLine;
  
  /**
   * Helper class for reading lines.
   */
  private class LineReader {
    /**
     * The input.
     */
    private final Reader _in;
    /**
     * Line builder.
     */
    private final StringBuilder _lineBuilder;
    /**
     * Current line start offset.
     */
    private int _currStartOff = 0;
    /**
     * Current line end offset.
     */
    private int _currEndOff = -1;
    /**
     * Current line number.
     */
    private int _currLineNum = 0;
    /**
     * Current line content.
     */
    private String _currLine;
    /**
     * @param in_ input.
     */
    LineReader(Reader in_) {
      _in = in_;
      _lineBuilder = new StringBuilder(2000);
    }
    /**
     * Reads a line from the input.
     * 
     * @return false on EOF, true otherwise.
     * @throws IOException 
     */
    boolean readNextLine() throws IOException {
      _lineBuilder.setLength(0);
      
      _currStartOff = _currEndOff + 1;
      
      boolean endOfFile = false;
      while (true) {
        int ch = _in.read();
        if (ch == -1) {
          endOfFile = true;
          break;
        } else {
          ++_currEndOff;
          if (ch == '\n') {
            break;
          } else {
            _lineBuilder.append((char)ch);
          }
        }
      }
      
      ++_currLineNum;
      _currLine = _lineBuilder.toString();
      
      return !_currLine.isEmpty() || !endOfFile;
    }
    
    /**
     * Reads al the lines from the input and fills the outer class.
     * 
     * @throws IOException 
     */
    void fillFromLines() throws IOException {
      while (readNextLine()) {
        _lines.add(_currLine);
        _lineToStartOffsets.add(_currStartOff);
        _startOffsetToLine.put(_currStartOff, _currLineNum);
      }
    }
  }
  
  /**
   * Construct an empty object (for internal use only).
   */
  private LineInformations() {
    _lines = new ArrayList<>(5000);
    _lineToStartOffsets = new ArrayList<>(5000);
    _startOffsetToLine = new TreeMap<>();
  }
  
  /**
   * Returns a line by line number.
   * 
   * @param lineNum_ line number (starting from 1)
   * @return line content.
   */
  public String getLineContent(int lineNum_) {
    return _lines.get(lineNum_ - 1);
  }
  
  /**
   * Returns the start offset of a line by line number.
   * 
   * @param lineNum_ line number (starting from 1)
   * @return start offset for the line.
   */
  public int getLineStartOffset(int lineNum_) {
    return _lineToStartOffsets.get(lineNum_ - 1);
  }
  
  /**
   * Returns a line number for an offset.
   * 
   * @param offset_ an offset.
   * @return line number.
   */
  public int getLineNumberForOffset(int offset_) {
    Map.Entry<Integer, Integer> entry = _startOffsetToLine.floorEntry(offset_);
    if (entry == null) {
      return _startOffsetToLine.lastEntry().getValue();
    } else {
      return entry.getValue();
    }
  }
  
  /**
   * Converts start offset and length to Location.
   * 
   * @param startOffset_ start offset
   * @param length_ length
   * @return 
   */
  public Location offsetToLocation(int startOffset_, int length_) {
    int line = getLineNumberForOffset(startOffset_);
    int startColumn = startOffset_ - getLineStartOffset(line) + 1;
    int endColumn = startColumn + length_ - 1;
    
    return new Location(line, startColumn, endColumn);
  }
  
  /**
   * Converts a location to its start offset.
   * 
   * @param loc_ a location.
   * @return 
   */
  public int locationToStartOffset(Location loc_) {
    return getLineStartOffset(loc_.line) + loc_.startColumn - 1;
  }
  
  /**
   * Constructs an instance from the given input.
   * 
   * @param content_ input.
   * @return a filled instance.
   * @throws IOException 
   */
  public static LineInformations fromReader(Reader content_) throws IOException {
    final LineInformations result = new LineInformations();
    
    LineReader reader = result.new LineReader(content_);
    reader.fillFromLines();
    
    return result;
  }
}
