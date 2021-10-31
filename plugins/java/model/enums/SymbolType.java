package model.enums;

public enum SymbolType {
    VARIABLE("Variable"),
    CONSTRUCTOR("Constructor"),
    METHOD("Method"),
    ENUM("Enum"),
    ENUM_CONSTANT("Enum constant"),
    TYPE("Type"),
    FILE("File");

    private final String name;

    SymbolType(String name) {
        this.name = name;
    }

    public String getName() {
        return name;
    }
}
