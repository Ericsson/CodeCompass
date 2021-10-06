package service.srcjava;

import cc.service.core.FilePosition;
import cc.service.java.JavaService;
import cc.service.language.AstNodeInfo;

import javax.persistence.EntityManager;

import java.util.logging.Level;

import static logger.Logger.LOGGER;
import static model.EMFactory.createEntityManager;

class JavaQueryHandler implements JavaService.Iface {
  // private static final EntityManager em =
  //   createEntityManager(System.getProperty("rawDbContext"));

  @Override
  public AstNodeInfo getAstNodeInfoByPosition(FilePosition fpos) {
    return null;
  }
}