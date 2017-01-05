package javademo;

import java.util.ArrayList;
import java.util.Collections;

public class GenericClass<T> {

    private T t;

    public void set(T t) {
        this.t = t;
    }

    public T get() {
        return t;
    }

    public <S> S simpleGenericFunction(S s) {
        return s;
    }

    public <S> void genericFunction(ArrayList<? extends S> listOfQuestionmark,
                                    ArrayList<S> listOfS,
                                    ArrayList<Integer> listOfInt) {
        Collections.sort(listOfInt);
    }

}
