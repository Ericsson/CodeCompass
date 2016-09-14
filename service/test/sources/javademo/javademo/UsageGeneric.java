package javademo;

public class UsageGeneric {

    public static void test() {
        test1();
        test2();
        test3();
    }

    public static void test1() {
        GenericClass<ClassA> gcClassA = new GenericClass<ClassA>();

        ClassA value = gcClassA.get();
        gcClassA.simpleGenericFunction(1);
        gcClassA.simpleGenericFunction(2);
    }

    public static void test2() {
        GenericClass<ClassB> gcClassB = new GenericClass<ClassB>();

        gcClassB.set(new ClassB(1000));
        ClassB value = gcClassB.get();
        gcClassB.simpleGenericFunction(1);
    }

    public static void test3() {
        GenericClass<ClassC> gcClassC = new GenericClass<ClassC>();

        ClassC value = gcClassC.get();
        gcClassC.simpleGenericFunction(1);
        gcClassC.simpleGenericFunction(2);
        gcClassC.simpleGenericFunction(3);
    }
}
