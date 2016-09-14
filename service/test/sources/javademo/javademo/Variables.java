package javademo;

/**
 * Documentation for public class Variables
 *
 * @author Toth Boldizsar
 * @deprecated Sample deprecated description.
 * @throws java.lang.Exception Sample throws description.
 * @see functionWithLocalVariables
 * @since 0.1
 * @version 0.2
 */
public class Variables {

    /**
     * Documentation for public int serializablePublicInt
     *
     * @serial
     */
    public int serializablePublicInt;

    public int publicInt;
    protected int protectedInt;
    int defaultInt;
    private int privateInt;

    public static int publicStaticInt;
    protected static int protectedStaticInt;
    static int defaultStaticInt;
    private static int privateStaticInt;

    public final int publicFinalInt = 0;
    protected final int protectedFinalInt = 1;
    final int defaultFinalInt = 2;
    private final int privateFinalInt = 3;

    public static final int publicStaticFinalInt = 0;
    protected static final int protectedStaticFinalInt = 1;
    static final int defaultStaticFinalInt = 2;
    private static final int privateStaticFinalInt = 3;

    public String publicString;
    protected String protectedString;
    String defaultString;
    private String privateString;

    public static String publicStaticString;
    protected static String protectedStaticString;
    static String defaultStaticString;
    private static String privateStaticString;

    public final String publicFinalString = "one";
    protected final String protectedFinalString = "two";
    final String defaultFinalString = "three";
    private final String privateFinalString = "four";

    public static final String publicStaticFinalString = "one";
    protected static final String protectedStaticFinalString = "two";
    static final String defaultStaticFinalString = "three";
    private static final String privateStaticFinalString = "four";

    public Object publicObject;
    protected Object protectedObject;
    Object defaultObject;
    private Object privateObject;

    public static Object publicStaticObject;
    protected static Object protectedStaticObject;
    static Object defaultStaticObject;
    private static Object privateStaticObject;

    public final Object publicFinalObject = null;
    protected final Object protectedFinalObject = null;
    final Object defaultFinalObject = null;
    private final Object privateFinalObject = null;

    public static final Object publicStaticFinalObject = null;
    protected static final Object protectedStaticFinalObject = null;
    static final Object defaultStaticFinalObject = null;
    private static final Object privateStaticFinalObject = null;

    /**
     * public void functionWithLocalVariables(int paramInt)
     *
     * @param paramInt This is an integer parameter
     * @return Returns the input parameter
     */
    public int functionWithLocalVariables(int paramInt) {
        int localInt;
        float localFloat;
        String localString;
        Variables localVariables;

        publicString = "publicString";
        publicInt = 1;
        int localIntSetToGlobalInt = publicInt;

        return paramInt;
    }

}
