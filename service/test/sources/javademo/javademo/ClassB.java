package javademo;

public class ClassB extends ClassA {

    public ClassB(int y) {
        super(y);
    }

    @Override
    public void function() {
        // Implementation in ClassB
    }

    public void testParentAndChildFunction() {
        super.function();
        this.function();

        (new ClassA(1)).function();
        (new ClassB(2)).function();
        (new ClassC()).function();
    }

}
