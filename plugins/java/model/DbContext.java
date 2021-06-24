package model;

import java.util.Arrays;
import java.util.HashMap;
import java.util.Map;

public class DbContext {
  private final String type;
  private final Map<String, String> context;
  private final String connString;

  public DbContext(String rawDbContext) {
    String[] splitByColon = rawDbContext.split(":");
    String[] splitBySemicolon = splitByColon[1].split(";");

    this.type = splitByColon[0];
    this.context = new HashMap<>();

    Arrays.stream(splitBySemicolon).forEach(c -> {
      String[] splitContext = c.split("=");
      context.put(splitContext[0], splitContext[1]);
    });

    this.connString =
      "jdbc:" +
      (type.equals("pgsql") ? "postgresql" : "sqlite") + "://" +
      this.getHost() + ":" +
      this.getPort() + "/" +
      this.getDatabase();
  }

  public String getDriver() {
    if (type.equals("pgsql")) {
      return "org.postgresql.Driver";
    }
    return "org.sqlite.JDBC";
  }

  public String getHost() {
    return context.get("host");
  }

  public String getPort() {
    return context.get("port");
  }

  public String getUser() {
    return context.get("user");
  }

  public String getPassword() {
    return context.get("password");
  }

  public String getDatabase() {
    return context.get("database");
  }

  public String getConnString() {
    return connString;
  }
}
