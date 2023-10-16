require([
    'codecompass/model',
    'codecompass/viewHandler',
    'codecompass/util'],
function (model, viewHandler, util) {  
    model.addService('cppservice', 'CppService', LanguageServiceClient);
});
  