package model;

import javax.persistence.EntityManager;
import javax.persistence.EntityManagerFactory;
import javax.persistence.Persistence;

public class EMFactory {
  public static EntityManager createEntityManager() {
    EntityManagerFactory emf =
      Persistence.createEntityManagerFactory("ParserPU");
    return emf.createEntityManager();
  }
}
