require([
  'codecompass/model',
  'codecompass/viewHandler',
  'codecompass/util'],
function (model, viewHandler, util){
  model.addService('pythonservice', 'PythonService', LanguageServiceClient);

  var pythonInfoTree = {};

  viewHandler.registerModule(pythonInfoTree, {
    type : viewHandler.moduleType.InfoTree,
    service : model.pythonservice
  });
});