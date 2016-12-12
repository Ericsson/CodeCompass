require([
  'codecompass/viewHandler',
  'codecompass/view/component/Text'],
function (viewHandler, Text) {

  viewHandler.registerModule(new Text({ id : 'text' }), {
    type : viewHandler.moduleType.Center
  });
});
