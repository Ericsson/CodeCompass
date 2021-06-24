package model;

import javax.persistence.EntityManager;
import javax.persistence.EntityManagerFactory;
import javax.persistence.Persistence;
import javax.persistence.spi.PersistenceUnitTransactionType;
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
    DbContext dbContext = new DbContext(rawDbContext);

    properties.put(
      TRANSACTION_TYPE,
      PersistenceUnitTransactionType.RESOURCE_LOCAL.name()
    );
    properties.put(JDBC_DRIVER, dbContext.getDriver());
    properties.put(
      JDBC_URL, dbContext.getConnString()
    );
    properties.put(JDBC_USER, dbContext.getUser());
    properties.put(JDBC_PASSWORD, dbContext.getPassword());

    return properties;
  }
}
