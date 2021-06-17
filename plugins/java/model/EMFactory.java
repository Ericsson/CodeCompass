package model;

import javax.persistence.EntityManager;
import javax.persistence.EntityManagerFactory;
import javax.persistence.Persistence;
import javax.persistence.spi.PersistenceUnitTransactionType;
import java.util.HashMap;

import static org.eclipse.persistence.config.PersistenceUnitProperties.*;

public class EMFactory {
  public static EntityManager createEntityManager() {
    EntityManagerFactory emf =
      Persistence.createEntityManagerFactory(
        "ParserPU", initProperties()
      );
    return emf.createEntityManager();
  }

  private static HashMap<String, Object> initProperties() {
    HashMap<String, Object> properties = new HashMap<>();

    properties.put(
      TRANSACTION_TYPE,
      PersistenceUnitTransactionType.RESOURCE_LOCAL.name()
    );
    properties.put(JDBC_DRIVER, "org.postgresql.Driver");
    properties.put(
      JDBC_URL, "jdbc:postgresql://localhost:5432/mydatabase"
    );
    properties.put(JDBC_USER, "compass");
    properties.put(JDBC_PASSWORD, "password");

    return properties;
  }
}
