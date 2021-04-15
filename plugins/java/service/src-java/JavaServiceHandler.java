import cc.service.java.JavaService;

abstract class JavaServiceHandler implements JavaService.Iface {
  @Override
  public String getJavaString() {
    return "THIS IS A TEST";
  }
}