package model;

import javax.persistence.Column;
import javax.persistence.Entity;
import javax.persistence.Table;

@Entity
@Table(name="\"JavaType\"")
public class JavaType extends JavaEntity{
    @Column(name = "\"isAbstract\"")
    private boolean isAbstract;

    // Getters and setters

    public boolean getIsAbstract() {
        return isAbstract;
    }

    public void setIsAbstract(boolean isAbstract) {
        this.isAbstract = isAbstract;
    }
}
