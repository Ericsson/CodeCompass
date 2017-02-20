require([
  'dijit/layout/ContentPane',
  'codecompass/viewHandler'],
function (ContentPane, viewHandler) {

  var infoPage = new ContentPane({
    id    : 'userguide',
    href  : 'userguide/doc/html/userguide.html',
    title : 'User Guide'
  });

  viewHandler.registerModule(infoPage, {
    type : viewHandler.moduleType.InfoPage
  });
});
