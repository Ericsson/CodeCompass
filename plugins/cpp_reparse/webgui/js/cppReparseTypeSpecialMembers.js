require([
  'dojo/_base/declare',
  'dojo/_base/array',
  'dojo/dom-construct',
  'dojo/dom-style',
  'dojo/topic',
  'dojo/on',
  'dijit/layout/ContentPane',
  'dojox/layout/ResizeHandle',
  'codecompass/view/component/ContextMenu',
  'codecompass/view/component/Text',
  'codecompass/model',
  'codecompass/viewHandler'],
function (declare, array, dom, style, topic, on, ContentPane, ResizeHandle,
          ContextMenu, Text, model, viewHandler) {

  model.addService('cppreparseservice', 'CppReparseService',
                   CppReparseServiceClient);

  if (!model.cppreparseservice.isEnabled())
    // Don't create this functionality if reparse is disabled.
    return;

  var SpecialMemberView = declare(Text, {
    constructor : function () {
      this._mappings = [];
      delete this._buildDialog;
    },

    postCreate : function () {
      this.inherited(arguments);

      // To show this member as a special version of the "Text" code
      // browser, remove a few attributes.
      for (var headerComponent in this._header) {
        if (headerComponent !== 'header' && headerComponent !== 'filename') {
          dom.destroy(this._header[headerComponent]);
          delete this._header[headerComponent];
        }
      }

      this._codeMirror.setOption('lineNumbers', false);
      this._codeMirror.setOption('gutters', []);
    },

    /**
     * Override the topic subscription of Text so the "openFile"
     * topic is not handled by this module.
     */
    _subscribeTopics : function () {
      var that = this;

      topic.subscribe('codecompass/cppReparse/handleSpecialMembers',
        function (astNodeInfo) {
          that.loadSpecialMembers(astNodeInfo.id);

          topic.publish('codecompass/setCenterModule', that.id);
        });
    },

    /**
     * Loads the special members of the class referenced by astNodeId, or
     * the class which contains the member function referenced by astNodeId.
     */
    loadSpecialMembers : function (astNodeId) {
      this._mappings = [];

      var sources = model.cppreparseservice.getSpecialMembersSource(astNodeId);

      // Set the type's name to the header of the text view.
      if (sources.length > 1) {
        var recordHead = sources[0].text;
        var inheritenceColonPos = recordHead.indexOf(':');

        if (inheritenceColonPos > -1)
          recordHead = recordHead.slice(0, inheritenceColonPos);

        dom.place(dom.toDom(recordHead), this._header.filename, 'only');
      }

      var that = this;
      var fullSrcText = "";
      var currentRow = 1, currentColumn = 1;
      sources.forEach(function (fragment) {
        if (!(currentRow in that._mappings))
          that._mappings[currentRow] = [];

        that._mappings[currentRow][currentColumn] = {
          mapped : fragment.mapped
        };

        if (fragment.mapped) {
          that._mappings[currentRow][currentColumn]['fileID'] = fragment.file;
          that._mappings[currentRow][currentColumn]['offset'] =
            fragment.topLeftOffset;
        }

        // Advance the position of the "write head" in the buffer.
        var rows = fragment.text.split('\n');
        var lineBreaks = rows.length - 1;
        var lettersInLastLine = rows[lineBreaks].length;
        if (lineBreaks === 0)
          currentColumn += lettersInLastLine;
        else {
          currentRow += lineBreaks;
          currentColumn = lettersInLastLine !== 0 ? lettersInLastLine : 1;
        }

        fullSrcText += fragment.text;
      });

      this.set('content', fullSrcText);
    },

    _eventHandler : function (event) {


      var pos = this._codeMirror.coordsChar({
        top  : event.clientY,
        left : event.clientX
      });

      pos = [
        pos.line + this._codeMirror.options.firstLineNumber,
        pos.ch + 1];

      // Search for the current clicked character in the mapping descriptor.
      // The mapping structure is first indexed by rows, then by columns. Each
      // record indicates the beginning of a range with a new mapping status,
      // as received by the API. The value contains a hash that describes the
      // mapping itself.
      var lastMappedRow = pos[0];
      var lastMappedColumn = 0;

      if (lastMappedRow in this._mappings) {
        // If the current row contains a mapping, it needs to be searched if
        // the current character falls into a mapping that begun in the current
        // row, as a previous mapping can last until later in the row.
        lastMappedColumn = pos[1];
        while (lastMappedColumn > 0 &&
               !(lastMappedColumn in this._mappings[lastMappedRow]))
          --lastMappedColumn;

        if (lastMappedColumn === 0)
          --lastMappedRow;
      }

      // Seek the line that precedes the clicked line and contains a mapping,
      // if no mapping was found in the current line.
      while (lastMappedRow > 0 && !(lastMappedRow in this._mappings))
        --lastMappedRow;

      if (lastMappedColumn === 0) {
        // If the mapping has not been found yet, the current row's search
        // set column to 0. In this case, the range that includes the click is
        // the last range that begins in the preceding, mapping-containing row.
        lastMappedColumn = this._mappings[lastMappedRow].length - 1;

        while (lastMappedColumn > 0 &&
               !(lastMappedColumn in this._mappings[lastMappedRow]))
          --lastMappedColumn;
      }

      var mapping = this._mappings[lastMappedRow][lastMappedColumn];
      if (!mapping.mapped) {
        // Clear the previously existing context menu because the current
        // click might not result in one.
        this._contextMenu.clear();

        // Do not do anything if the area is not mapped - there is nothing
        // we can do.
        return;
      }

      // Otherwise, "emulate" that the user clicked in the mapped file on the
      // mapped character's position.
      var fileInfo = model.project.getFileInfo(mapping.fileID);
      pos = [
        mapping.offset.line + pos[0] - lastMappedRow,
        mapping.offset.column + pos[1] - lastMappedColumn];

      // Only support Right Click and Ctrl-Click, as marking the usages is
      // not feasible through many different mappings.

      if (event.button === 2)
        this.buildContextMenu(pos, fileInfo, this._contextMenu);
      else if (event.button === 0 && event.ctrlKey) {
        var astNodeInfo = astHelper.getAstNodeInfoByPosition(pos, fileInfo);
        var service = model.getLanguageService(fileInfo.type);
        astHelper.jumpToDef(astNodeInfo.id, service);
      }
    },

    // --- Override the unsupported behaviours of this subtype. --- //

    loadFile : function (file) {},
    _setHeaderAttr : function (fileInfo) {},
    _setHeaderPathAttr : function (path) {}
  });

  var specialMemberView = new SpecialMemberView({
    id : 'cppreparse-special-member-view'
  });

  viewHandler.registerModule(specialMemberView, {
    type: viewHandler.moduleType.Center,
    fileType: 'cpp-reparse'
  });
});
