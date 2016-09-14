package javademo;

import java.io.IOException;

/**
* The purpose of this class is to test the ability of the Java Parser to
* parse javadoc tags.
*
* @author   Zsíros B. Ödön
* @since    1.0
* @version  1.2
*/
public class JavadocTags {

	/**
	* This is a public int.
	*/
	public int pubInt;

	/**
	* This is a protected String.
	*/
	protected String protString;

	/**
	* This is a constructor without parameters.
	*/
	public JavadocTags() {}

	/**
	* This is a constructor with one parameter.
	*
	* @param paramInt An integer parameter.
	*/
	public JavadocTags(int paramInt) {}

	/**
	* This is a constructor with two parameters.
	*
	* @param paramInt An integer parameter.
	* @param paramString A String parameter.
	*/
	public JavadocTags(int paramInt, String paramString) {}

	/** This is a public function that returns with void. */
	public void publicFunctionReturnsVoid() {}

	/**
	* This is a public function that returns with void.
	*
	* @param param1 This is an integer parameter.
	*/
	public void publicFunctionReturnsVoid(int param1) {}

	/**
	* This is a public function that returns with void.
	*
	* @param param1 This is the first integer parameter.
	* @param param2 This is the second integer parameter.
	*/
	public void publicFunctionReturnsVoid(int param1, int param2) {}

	/**
	* This is a public function that returns with an integer.
	*
	* @return   Just an integer.
	* @throws   IOException Throws an IOException.
	* @see      Integer
	* {@link    Integer}
	* @since    1.0
	* @version  1.2
	*/
	public int publicFunctionReturnsInt() throws IOException {
		return 0;
	}

}