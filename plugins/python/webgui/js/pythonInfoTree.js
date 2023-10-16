require([
    'codecompass/model',
    'codecompass/viewHandler',
    'codecompass/util'],
function (model, viewHandler, util) {  
    model.addService('pythonservice', 'PythonService', LanguageServiceClient);
});
  