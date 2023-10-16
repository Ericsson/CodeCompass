require([
  'dojo/topic',
  'dijit/Menu',
  'dijit/MenuItem',
  'dijit/PopupMenuItem',
  'codecompass/astHelper',
  'codecompass/model',
  'codecompass/urlHandler',
  'codecompass/viewHandler'],
function (topic, Menu, MenuItem, PopupMenuItem, astHelper, model, urlHandler, viewHandler) {
  model.addService('pythonservice', 'PythonService', LanguageServiceClient);
});