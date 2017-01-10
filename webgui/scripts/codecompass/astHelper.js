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
   * @param {Array} elems Array of AstNodeInfo Thrift objects.
   */
  function buildAmbiguousRefPage(elems) {
    function formatter(content) {
      return '<div style="cursor: pointer">' + content + '</div>';
    }

    var layout = [{
      name      : 'Id',
      field     : 'id',
      hidden    : true
    }, {
      name      : 'File name',
      field     : 'filename',
      width     : 30,
      formatter : formatter
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
        position : 'line: ' + nodeInfo.range.range.startpos.line
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

    style.set(dialog.get('containerNode'), 'width', '500px');
    style.set(dialog.get('containerNode'), 'max-height', '300px');
    style.set(dialog.get('containerNode'), 'overflow-y', 'auto');

    dialog.show();
  }

  return {
    jumpToDef : function (astNodeId, service) {
      var refTypes = service.getReferenceTypes(astNodeId);
      var astNodeInfos = service.getReferences(
        astNodeId, refTypes['Definition']);

      if (astNodeInfos.length === 0) {
        var astNodeInfo = service.getAstNodeInfo(astNodeId);

        // If the symbolType is File, the astNodeValue contain the path of it.
        if (astNodeInfo.symbolType == 'File') {
          var fileInfo = model.project.getFileInfoByPath(
            astNodeInfo.astNodeValue);
          topic.publish('codecompass/openFile', {
            fileId    : fileInfo.id
          });
        } else {
          console.warn("Can't find definition to this node");
        }
      } else if (astNodeInfos.length === 1)
        jump(astNodeInfos[0]);
      else
        buildAmbiguousRefPage(astNodeInfos);
    }
  };
});
