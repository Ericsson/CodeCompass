package model;

import javax.persistence.Column;
import javax.persistence.Entity;
import javax.persistence.Table;

@Entity
@Table(name="\"JavaEnumConstant\"")
public class JavaEnumConstant extends JavaEntity{

    @Column(name = "value")
    private int value;

    // Getters and setters

    public int getValue() {
        return value;
    }

    public void setValue(int value) {
        this.value = value;
    }
}
