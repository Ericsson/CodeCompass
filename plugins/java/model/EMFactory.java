package model;

import javax.persistence.EntityManager;
import javax.persistence.EntityManagerFactory;
import javax.persistence.Persistence;
import javax.persistence.spi.PersistenceUnitTransactionType;
import java.util.Arrays;
import java.util.HashMap;

import static org.eclipse.persistence.config.PersistenceUnitProperties.*;

public class EMFactory {
  public static EntityManager createEntityManager(String rawDbContext) {
    EntityManagerFactory emf =
      Persistence.createEntityManagerFactory(
        "ParserPU", initProperties(rawDbContext)
      );
    return emf.createEntityManager();
  }

  private static HashMap<String, Object> initProperties(String rawDbContext) {
    HashMap<String, Object> properties = new HashMap<>();
    String[] splitByColon = rawDbContext.split(":");
    String dbType = splitByColon[0];
    String contextString = splitByColon[1];
    boolean isSqlite = dbType.equals("sqlite");
    String driver =
      isSqlite ? "org.sqlite.JDBC" : "org.postgresql.Driver";
    String[] contextList = contextString.split(";");
    HashMap<String, String> contextMap = new HashMap<>();

    properties.put(
      TRANSACTION_TYPE,
      PersistenceUnitTransactionType.RESOURCE_LOCAL.name()
    );

    Arrays.stream(contextList).forEach(c -> {
      String[] splitContext = c.split("=");
      contextMap.put(splitContext[0], splitContext[1]);
    });

    String connString =
      isSqlite ?
        "jdbc:" + "sqlite" + ":/" +
          contextMap.get("database")
            .replaceFirst("^~", System.getProperty("user.home"))
        :
        "jdbc:" + "postgresql" + "://" +
          contextMap.get("host") + ":" +
          contextMap.get("port") + "/" +
          contextMap.get("database");

    properties.put(JDBC_DRIVER, driver);
    properties.put(JDBC_URL, connString);

    if (!isSqlite) {
      properties.put(JDBC_USER, contextMap.get("user"));
      properties.put(JDBC_PASSWORD, contextMap.get("password"));
    }

    return properties;
  }
}
