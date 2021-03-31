import cc.service.java.JavaService;

abstract class JavaHandler implements JavaService.Iface {
  @Override
  public String getJavaString() {
    return "THIS IS A TEST";
  }
}