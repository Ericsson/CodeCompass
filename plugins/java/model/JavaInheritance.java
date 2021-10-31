package model;

import javax.persistence.*;

@Entity
@Table(name = "\"JavaInheritance\"")
public class JavaInheritance {
    @Id
    @GeneratedValue
    @Column(name = "id")
    private long id;

    @Column(name = "base")
    private long base;

    @Column(name = "derived")
    private long derived;

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
}
