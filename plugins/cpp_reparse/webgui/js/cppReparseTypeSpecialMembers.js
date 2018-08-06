require([
  'dojo/topic',
  'codecompass/model'],
function (topic, model) {

  model.addService('cppreparseservice', 'CppReparseService',
                   CppReparseServiceClient);

  if (!model.cppreparseservice.isEnabled())
    // Don't create this functionality if reparse is disabled.
    return;

  topic.subscribe('codecompass/cppReparse/handleSpecialMembers',
    function (astNodeInfo) {
      var res = model.cppreparseservice.getSpecialMembersSource(
        astNodeInfo.id);

      var sourceText = "";
      res.forEach(function (specialMember) {
        sourceText += specialMember.text;
      });
      console.log(sourceText);
  });
});
