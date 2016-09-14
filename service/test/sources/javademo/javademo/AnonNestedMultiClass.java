package javademo;

public class AnonNestedMultiClass {

    public int globalVariableInt;

    public AnonNestedMultiClass() {

        class NestedClassFirst {

            public int nestedClassFirstGlobalVariableInt;
        }

        class NestedClassSecond {

            public int nestedClassSecondGlobalVariableInt;
        }

        InnerClassFirst anonClassFirst = new InnerClassFirst() {
            public int anonClassFirstInt;
        };

        InnerClassSecond anonClassSecond = new InnerClassSecond() {
            public int anonClassSecondInt;
        };
    }
}

class InnerClassFirst {

    public int innerClassFirstGlobalVariableInt;
}

class InnerClassSecond {

    public int innerClassSecondGlobalVariableInt;
}
