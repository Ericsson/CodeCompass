package model;

import javax.persistence.Column;
import javax.persistence.Entity;
import javax.persistence.GeneratedValue;
import javax.persistence.Id;

@Entity
public class JavaInheritance {
    @Id
    @GeneratedValue
    @Column(name = "id")
    private long id;

    @Column(name = "derived")
    private long derived;

    @Column(name = "base")
    private long base;

    @Column(name = "\"isAbstract\"")
    private boolean isAbstract;

    @Column(name = "visibility")
    private int visibility;


    // Getters and setters

    public long getId() {
        return id;
    }

    public void setId(long id) {
        this.id = id;
    }

    public long getDerived() {
        return derived;
    }

    public void setDerived(long derived) {
        this.derived = derived;
    }

    public long getBase() {
        return base;
    }

    public void setBase(long base) {
        this.base = base;
    }

    public boolean isAbstract() {
        return isAbstract;
    }

    public void setAbstract(boolean anAbstract) {
        isAbstract = anAbstract;
    }

    public int getVisibility() {
        return visibility;
    }

    public void setVisibility(int visibility) {
        this.visibility = visibility;
    }
}
