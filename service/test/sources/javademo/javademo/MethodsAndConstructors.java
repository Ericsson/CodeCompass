package javademo;

public final class MethodsAndConstructors {

    // overloading constructors
    public MethodsAndConstructors() {
    }

    public MethodsAndConstructors(int firstParamInt) {
    }

    public MethodsAndConstructors(double firstParamDouble) {
    }

    public MethodsAndConstructors(int firstParamInt, int secondParamInt) {
    }

    public MethodsAndConstructors(double firstParamDouble, double secondParamDouble) {
    }

    public MethodsAndConstructors(int firstParamInt, double secondParamDouble) {
    }

    public MethodsAndConstructors(double firstParamDouble, int secondParamInt) {
    }

    public MethodsAndConstructors(MethodsAndConstructors mac){
    }

    public MethodsAndConstructors(MethodsAndConstructors mac1, MethodsAndConstructors mac2){
    }

    static {
        int staticInitializerBlockInt;
    }

    {
        int initializerBlockInt;
    }

    // recursion
    public void recursionTimes30(int iteration) {
        if (iteration < 30) {
            recursionTimes30(iteration + 1);
        }
        // System.out.println(iteration);
    }

    // A -> B -> C -> A ...
    public void callOtherFunctionStopAt30_A(int count) {
        if (count < 30) {
            callOtherFunctionStopAt30_B(count + 1);
        }
        // System.out.println(count);
    }

    public void callOtherFunctionStopAt30_B(int count) {
        if (count < 30) {
            callOtherFunctionStopAt30_C(count + 1);
        }
        // System.out.println(count);
    }

    public void callOtherFunctionStopAt30_C(int count) {
        if (count < 30) {
            callOtherFunctionStopAt30_A(count + 1);
        }
        // System.out.println(count);
    }

    // basic methods
    public void methodReturnsVoid() {
    }

    public int methodReturnsInt() {
        return 0;
    }

    public String methodReturnsString() {
        return "asdf";
    }

    public static void staticMethodReturnsVoid() {
    }

    public static int staticMethodReturnsInt() {
        return 0;
    }

    public static String staticMethodReturnsString() {
        return "asdf";
    }

    // overloading
    public void overloadedMethod() {
    }

    public void overloadedMethod(int onlyParamInt) {
    }

    public void overloadedMethod(int firstParamInt, int secondParamInt) {
    }

    public void overloadedMethod(double onlyParamDouble) {
    }

    public void overloadedMethod(double firstParamDouble, double secondParamDouble) {
    }

    public void overloadedMethod(int firstParamInt, double secondParamDouble) {
    }

    public void overloadedMethod(double firstParamDouble, int secondParamInt) {
    }

}
