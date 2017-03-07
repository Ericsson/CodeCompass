define([
  'dijit/Dialog',
  'dojo/topic',
  'dojo/dom-style',
  'dojo/data/ItemFileWriteStore',
  'dojox/grid/DataGrid',
  'codecompass/model'],
function (Dialog, topic, style, ItemFileWriteStore, DataGrid, model) {
  /**
   * This function opens the file and jumps to the location of the given AST
   * node.
   * @param {AstNodeInfo} astNodeInfo Thrift object.
   */
  function jump(astNodeInfo) {
    var range = astNodeInfo.range.range;

    topic.publish('codecompass/openFile', {
      fileId    : astNodeInfo.range.file,
      line      : range.startpos.line,
      selection : [
        range.startpos.line,
        range.startpos.column,
        range.endpos.line,
        range.endpos.column]
    });
  }

  /**
   * This function builds and displays a modal dialog which contains the given
   * elements. These elements are clickable so the user can jump to the location
   * of the elements.
   * @param {Array}     elems     Array of AstNodeInfo Thrift objects.
   * @param {Function}  decorator A callback function which can specify a class
   * to be used on the filename of the reference.
   */
  function buildAmbiguousRefPage(elems, decorator) {
    function formatter(content) {
        return '<div style="cursor: pointer">' + content + '</div>';
    }

    function filenameFormatter(item) {
      // Call the decorator function (if exists) to give us decoration
      // for the filename.

      var decoration;
      if (decorator)
        decoration = decorator(item);

      return '<div style="cursor: pointer">' +
        (decoration ? '<span class="' + decoration.class + '">' : '') +
        item.filename +
        (decoration ? '</span>' : '') +
        '</div>';
    }

    var layout = [{
      name      : 'Id',
      field     : 'id',
      hidden    : true
    }, {
      name      : 'File name',
      field     : '_item',
      width     : 40,
      formatter : filenameFormatter
    }, {
      name      : 'Position',
      field     : 'position',
      formatter : formatter
    }];

    var store = new ItemFileWriteStore({
      data : {
        identifier : 'id',
        items      : []
      }
    });

    var infos = [];

    elems.forEach(function (nodeInfo) {
      var fileInfo = model.project.getFileInfo(nodeInfo.range.file);
      store.newItem({
        id       : Math.random(),
        filename : fileInfo.path,
        position : 'line: ' + nodeInfo.range.range.startpos.line,
        tags     : nodeInfo.tags
      });
      infos.push(nodeInfo);
    });

    var datagrid = new DataGrid({
      store : store,
      structure : layout,
      autoWidth : true,
      autoHeight : true,
      style : 'margin: 0 auto',
      onRowClick : function (event) {
        dialog.hide();
        jump(infos[event.rowIndex]);
      }
    });

    var dialog = new Dialog({
      title   : 'Possible definitions',
      content : datagrid
    });

    style.set(dialog.get('containerNode'), 'width', '600px');
    style.set(dialog.get('containerNode'), 'max-height', '300px');
    style.set(dialog.get('containerNode'), 'overflow-y', 'auto');

    dialog.show();
  }

  return {
    jumpToDef : function (astNodeId, service, ambiguousPageDecorator) {
      var refTypes = service.getReferenceTypes(astNodeId);
      var astNodeInfos = service.getReferences(
        astNodeId, refTypes['Definition']);

      if (astNodeInfos.length === 0) {
        var astNodeInfo = service.getAstNodeInfo(astNodeId);

        // If the symbolType is File, the astNodeValue contains the path of it.
        if (astNodeInfo.symbolType === 'File') {
          var fileInfo = model.project.getFileInfoByPath(
            astNodeInfo.astNodeValue);
          topic.publish('codecompass/openFile', {
            fileId : fileInfo.id
          });
        } else {
          console.warn("Can't find definition to this node");
        }
      } else if (astNodeInfos.length === 1)
        jump(astNodeInfos[0]);
      else
        buildAmbiguousRefPage(astNodeInfos, ambiguousPageDecorator);
    },

    /**
     * This function returns the AST node info object which belongs to the given
     * position in the given file.
     * @param {Array} position An array with two elements: line and column
     * respectively.
     * @param {FileInfo} fileInfo A Thrift object which contains the information
     * of the file in which the click happens.
     * @return {AstNodeInfo} Thrift object which describes the AST node at the
     * clicked position.
     */
    getAstNodeInfoByPosition : function (position, fileInfo) {

      //--- File position ---//

      var fpos = new FilePosition();
      var  pos = new Position();

      pos.line = position[0];
      pos.column = position[1];
      fpos.file = fileInfo.id;
      fpos.pos = pos;

      //--- Get AST node info ---//

      var service = model.getLanguageService(fileInfo.type);
      if (service)
        return service.getAstNodeInfoByPosition(fpos);
    }
  };
});
