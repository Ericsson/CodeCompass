package cc.search.indexer.util;

import java.io.BufferedInputStream;
import java.io.ByteArrayOutputStream;
import java.io.IOException;
import java.io.InputStream;
import java.io.InputStreamReader;
import java.nio.charset.Charset;

/**
 * Helper method for I/O.
 */
public class IOHelper {
  /**
   * Construct an InputStreamReader with a possibly correct character set. The
   * implementation is based on OpenGrock (TextAnalyzer.java).
   * 
   * @param input_ input stream.
   * @return input reader.
   * @throws java.io.IOException
   */
  public static InputStreamReader getReaderForInput(InputStream input_) throws IOException {
    InputStream in = input_.markSupported() ?
      input_ : new BufferedInputStream(input_);

    String charset = null;
    byte[] head = new byte[3];
    
    in.mark(3);
    
    int br = in.read(head, 0, 3);
    if (br >= 2
      && (head[0] == (byte) 0xFE && head[1] == (byte) 0xFF)
      || (head[0] == (byte) 0xFF && head[1] == (byte) 0xFE)) {
      charset = "UTF-16";
      in.reset();
    } else if (br >= 3
      && head[0] == (byte) 0xEF && head[1] == (byte) 0xBB
      && head[2] == (byte) 0xBF) {
      // InputStreamReader does not properly discard BOM on UTF8 streams,
      // so don't reset the stream.
      charset = "UTF-8";
    }

    if (charset == null) {
      in.reset();
      charset = Charset.defaultCharset().name();
    }

    return new InputStreamReader(in, charset);
  }
  /**
   * Reads the full content of the stream to a String.
   * 
   * @param reader_ input stream.
   * @return content.
   * @throws IOException 
   */
  public static String readFullContent(InputStreamReader reader_) throws IOException {
    ByteArrayOutputStream out = new ByteArrayOutputStream();

    int b = reader_.read();
    while (b != -1) {
      out.write(b);
      b = reader_.read();
    }

    return out.toString(reader_.getEncoding());
  }
}
