package model.enums;

public enum AstType {
    STATEMENT("Statement"),
    DECLARATION("Declaration"),
    DEFINITION("Definition"),
    USAGE("Usage");

    private final String name;

    AstType(String name) {
        this.name = name;
    }

    public String getName() {
        return name;
    }
}
