package javademo;

import java.util.ArrayList;

public class Usage {

    public static void test() {
        testABC();
        testAbstract();
        testDeepCallChain();
        testMethodsAndConstructors();
        testVariables();
        testAnonNestedMultiClass();
        testGenericClass();

        // TODO: more test calls should be placed here
    }

    public static void testABC() {
        ClassA classA = new ClassA(2);
        ClassB classB = new ClassB(3);
        ClassC classC = new ClassC();
        ClassA classX = new ClassB(4);
        ClassA classY = new ClassC();

        classA.function();
        classB.function();
        classB.testParentAndChildFunction();
        classC.function();
        classX.function();
        classY.function();
    }

    public static void testAbstract() {
        AbstractClass abs = new ClassExtendsAndImplements();
        abs.abstractClassMethod();

        ClassExtendsAndImplements ceai = new ClassExtendsAndImplements();
        ceai.abstractClassMethod();
        ceai.interfaceAMethod();
        ceai.interfaceBMethod();
        int x1 = ceai.interfaceAInt;
        int x2 = ceai.interfaceBInt;

        InterfaceA ifA = new ClassExtendsAndImplements();
        ifA.interfaceAMethod();
        int x3 = ifA.interfaceAInt;

        InterfaceB ifB = new ClassExtendsAndImplements();
        ifB.interfaceBMethod();
        int x4 = ifB.interfaceBInt;
    }

    public static void testDeepCallChain() {
        FunctionCallInOtherClass_1.step1();
    }

    public static void testMethodsAndConstructors() {
        MethodsAndConstructors mac1 = new MethodsAndConstructors();
        MethodsAndConstructors mac2 = new MethodsAndConstructors(1);
        MethodsAndConstructors mac3 = new MethodsAndConstructors(1.0);
        MethodsAndConstructors mac4 = new MethodsAndConstructors(1,2);
        MethodsAndConstructors mac5 = new MethodsAndConstructors(1.0,2.0);
        MethodsAndConstructors mac6 = new MethodsAndConstructors(1,2.0);
        MethodsAndConstructors mac7 = new MethodsAndConstructors(1.0,2);

        mac1.recursionTimes30(0);
        mac1.callOtherFunctionStopAt30_A(0);

        int smri    = mac1.staticMethodReturnsInt();
        String smrs = mac1.staticMethodReturnsString();
        String mrs  = mac1.methodReturnsString();
        int mri     = mac1.methodReturnsInt();
        mac1.staticMethodReturnsVoid();
        mac1.methodReturnsVoid();

        mac1.overloadedMethod();
        mac1.overloadedMethod(1);
        mac1.overloadedMethod(1,2);
        mac1.overloadedMethod(1.0);
        mac1.overloadedMethod(1.0,2.0);
        mac1.overloadedMethod(1,2.0);
        mac1.overloadedMethod(1.0,2);
    }

    public static void testVariables() {
        Variables variables1 = new Variables();

        variables1.serializablePublicInt    = 1;
        variables1.publicInt                = 1;
        variables1.publicStaticInt          = 1;
        variables1.publicString             = "iAmAString";
        variables1.publicStaticString       = "iAmAString";
        variables1.publicObject             = new Object();
        variables1.publicStaticObject       = new Object();
    }

    public static void testAnonNestedMultiClass() {
        AnonNestedMultiClass anmc = new AnonNestedMultiClass();

        anmc.globalVariableInt = 1;
    }

    public static void testGenericClass() {
        GenericClass<String> gcString = new GenericClass<String>();

        gcString.set("iAmAString");
        String instanceString = gcString.get();

        gcString.simpleGenericFunction(1);
        gcString.genericFunction(new ArrayList<String>(), new ArrayList<Object>(), new ArrayList<Integer>());
    }

    public static void testLocalVariables() {
        ClassExtendsAndImplements ceai = new ClassExtendsAndImplements();
        ceai.abstractClassMethod();

        AnonNestedMultiClass anmc = new AnonNestedMultiClass();
        anmc.globalVariableInt = 1;

        MethodsAndConstructors mac = new MethodsAndConstructors(10,2);
        int localInt1       = 100;
        int localInt2       = 200;
        double localDouble1 = 100.0;
        double localDouble2 = 200.0;

        mac.overloadedMethod(localInt1);
        mac.overloadedMethod(localDouble1);
        mac.overloadedMethod(localInt1,localInt2);
        mac.overloadedMethod(localDouble1,localDouble2);
        mac.overloadedMethod(localDouble1,localInt1);
        mac.overloadedMethod(localInt1,localDouble1);

        mac = null;
        mac = new MethodsAndConstructors(1.0);
        mac = new MethodsAndConstructors(mac);
        mac = new MethodsAndConstructors(mac,mac);
    }

    public static int simpleFunctionTest(int intParam, double doubleParam, Usage usageParam) {
        int localVariableInt = intParam;
        double localVariableDouble = doubleParam;
        Usage localVariableUsage = usageParam;

        return localVariableInt;
    }

    public static void iCallSimpleFunctionTest() {
        Usage usage = new Usage();
        simpleFunctionTest(1, 1.0, usage);
    }

}
