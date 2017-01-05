define([
  'dojo/_base/array',
  'dojo/_base/declare',
  'dojo/dom-class',
  'dojo/dom-construct',
  'dijit/Dialog',
  'codecompass/registration/model'],
function (array, declare, domClass, dom, Dialog, model) {
  
  /**
   * This function appends a header row in the table of the given dialog instance.
   * @param {BuildLogDialog} buildLogDialog A BuildLogDialog instance.
   */
  function addHeader(buildLogDialog) {
    var row = dom.create('tr');
    var col;
    
    col = dom.create('th', { innerHTML : 'Type' });
    dom.place(col, row);
    col = dom.create('th', { innerHTML : 'Message' });
    dom.place(col, row);
    col = dom.create('th', { innerHTML : 'From', colspan : 2 });
    dom.place(col, row);
    col = dom.create('th', { innerHTML : 'To', colspan : 2 });
    dom.place(col, row);
    
    dom.place(row, buildLogDialog._table);
  }
  
  return declare(Dialog, {
    
    buildRendering : function () {
      this.inherited(arguments);
      
      this._table = dom.create('table', { class : 'buildlogtable' });
      
      addHeader(this);
      this.set('content', this._table);
    },
    
    /**
     * This function appends an array of build logs in the table of the dialog.
     * @param {[BuildLog]} buildLogs An array of BuildLog thrift objects.
     */
    addBuildLogs : function (buildLogs) {
      var that = this;
      
      array.forEach(buildLogs, function (buildLog) {
        var row = dom.create('tr', { onclick : function () {
          that.hide();
          
          if (!that.editor)
            console.error("Editor object must be given in constructor");
          
          that.editor.set('selection', {
            from : buildLog.range.startpos,
            to   : buildLog.range.endpos
          });
          
          that.editor.jumpTo(buildLog.range.startpos.line, buildLog.range.startpos.end);
        } });
        
        var type = dom.create('span');
        var col;
        
        switch (buildLog.messageType) {
          case model.MessageType.Unknown:
            domClass.add(type, 'unknown');
            dom.place(dom.toDom('Unknown'), type);
            break;
          case model.MessageType.Error:
            domClass.add(type, 'error');
            dom.place(dom.toDom('Error'), type);
            break;
          case model.MessageType.FatalError:
            domClass.add(type, 'fatalerror');
            dom.place(dom.toDom('Fatal Error'), type);
            break;
          case model.MessageType.Warning:
            domClass.add(type, 'warning');
            dom.place(dom.toDom('Warning'), type);
            break;
          case model.MessageType.Note:
            domClass.add(type, 'note');
            dom.place(dom.toDom('Note'), type);
            break;
          case model.MessageType.CodingRule:
            domClass.add(type, 'codingrule');
            dom.place(dom.toDom('Coding Rule'), type);
            break;
        }
        
        col = dom.create('td');
        dom.place(type, col);
        dom.place(col, row);
        
        col = dom.create('td', { innerHTML : buildLog.message });
        dom.place(col, row);
        col = dom.create('td', { innerHTML : buildLog.range.startpos.line });
        dom.place(col, row);
        col = dom.create('td', { innerHTML : buildLog.range.startpos.column });
        dom.place(col, row);
        col = dom.create('td', { innerHTML : buildLog.range.endpos.line });
        dom.place(col, row);
        col = dom.create('td', { innerHTML : buildLog.range.endpos.column });
        dom.place(col, row);
        
        dom.place(row, this._table);
      }, this);
    },
    
    /**
     * This function clears the build log table in the dialog. Only a header row is left there.
     */
    clearBuildLogs : function () {
      dom.empty(this._table);
      addHeader(this);
    }
  });
});