require([
  'dojo/on',
  'dojo/topic',
  'dojo/_base/declare',
  'dojo/_base/Color',
  'dojo/Deferred',
  'dojo/dom-construct',
  'dijit/layout/ContentPane',
  'dijit/Tooltip',
  'codecompass/model',
  'codecompass/viewHandler',
  'codecompass/urlHandler'],
function (on, topic, declare, Color, Deferred, dom, ContentPane, Tooltip,
          model, viewHandler, urlHandler) {

  model.addService('cppreparseservice', 'CppReparseService',
                   CppReparseServiceClient);

  var ASTText = declare(ContentPane, {
    constructor : function () {
      this._subscribeTopics();
    },

    postCreate : function () {
      this.inherited(arguments);
    },

    setLoading : function() {
      // TODO: Figure out how to use a StandBy for this.
      // Problem is that if a standby is done, the domNode of it when loading
      // the view (via a call to setState from the framework) will simply not
      // see this domNode and thus the whole load will run into an error.

      this.set('content',
        "<span style=\"text-align: center; font-size: 18px; color: #d00;\">" +
        "Parsing and generating syntax tree, please wait a moment...</span>");
    },

    setState : function (state) {
      if (state.center !== this.id || !(state.fid))
        return;

      if (state.fid)
        this.loadASTForFile(urlHandler.getFileInfo());
    },

    loadASTForFile : function (fileInfo) {
      var that = this;
      this.setLoading();

      model.cppreparseservice.getAsHTML(
        fileInfo.id,
        function(astHtml) {
          /* TODO: {Long-term} Allow the user to click on the SourceLocation in
           * TODO: the AST which jumps back to the source code view,
           * TODO: highlighting that area.
           */
          that.set('content', astHtml);
        }
      );
    },

    _subscribeTopics : function () {
      var that = this;
      topic.subscribe('codecompass/cppreparsefile', function (message) {
        var fileInfo = message.fileInfo;
        if (!fileInfo)
          return;

        that.loadASTForFile(fileInfo, true);
        topic.publish('codecompass/setCenterModule', that.id);

        urlHandler.setStateValue({
          center : that.id,
          fid : fileInfo.id
        });
        urlHandler.unsetStateValue("select");
      });
    }
  });

  var ASTView = new ASTText({id : 'cpp-ast'});
  viewHandler.registerModule(ASTView, {
    type : viewHandler.moduleType.Center
  });
});
