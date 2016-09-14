define([
  'dijit/layout/ContentPane',
  'dojo/_base/declare',
  'dojo/dom-construct',
  'dojo/topic',
  'dojo/mouse',
  'codecompass/registration/model',
  'codecompass/util'],
function (ContentPane, declare, dom, topic, mouse, model, util) {
  
  var CatalogueTreeGrid = declare(ContentPane, {
    /**
     * The constructor creates a table for Catalog. The constructor has to be
     * given an object as parameter which contains at least a "service"
     * attribute which is a LanguageService thrift object.
     */
    constructor : function () {
      this._data = [];
    },

    postCreate : function () {
      var that = this;

      this.set('content', dom.create('span', {
        class : 'diagram-loading'
      }));

      this.service.getCatalogue(function (catalog) {
        catalog.forEach(function (infoNode) {
          that._data.push(that._createItem(infoNode));
        });

        that._buildTable();

        that.set('content', that._table);
      });
    },

    _buildTable : function () {
      var that = this;

      //--- Build table ---//

      this._table = dom.toDom(
         '<table class="TreeDataGrid" width="99%">'
        +  '<colgroup>'
        +    '<col width="20%"></col>'
        +    '<col width="*"></col>'
        +    '<col width="65%"></col>'
        +  '</colgroup>'
        +  '<thead>'
        +    '<tr> <th>Type</th> <th>Name</th> <th>Documentation</th></tr>'
        +  '</thead>'
        +  '<tbody>'
        +    '<tr> <th></th> <th></th> <th></th></tr>'
        +  '</tbody>'
        + '</table>');

      $(this._table).fancytree({
        extensions : ['table'],
        table      : { indentation : 20, checkboxColumnIdx : 0 },
        source     : that._data,

        renderColumns : function (event, data) {
          function setIcon(node, url) {
            node.css({
              backgroundImage    : 'url(' + url + ')',
              backgroundPosition : '0 0'
            });
          }

          var node = data.node;

          var $tdList = $(node.tr).find('>td');

          $tdList.eq(1).html(
            node.data.name[0] !== '(' // (anonymous) or (global types)
            ? $('<div class="fancytree-name">').text(node.data.name)
            : node.data.name);

          $tdList.eq(2).html(node.data.documentation);

          switch (node.data.astValue.astNodeType) {
            case 'Namespace':
              setIcon($(node.span).find('> span.fancytree-icon'), '/images/n_icon.png');
              break;
            case 'Type':
              setIcon($(node.span).find('> span.fancytree-icon'), '/images/t_icon.png');
              break;
            case 'MoreBtn':
              setIcon($(node.span).find('> span.fancytree-icon'), '/images/dots_icon.png');
              break;
          }
        },

        lazyLoad : function (event, data) {
          var node = data.node;

          var result = that.service.getSubCatalogue(
            node.data.infoQuery).map(that._createItem);

          result.forEach(function (node) {
            if (node.astValue.astNodeType === 'Type')
              node.lazy = false;
          });

          data.result = result;
        }
      });

      //--- Add mouse events ---//

      $(this._table).delegate('.fancytree-title, .fancytree-icon', 'click',
      function (e) {
        var node = $.ui.fancytree.getNode(e);

        if (node.data.isMoreBtn) {
          that.service.getSubCatalogue(node.data.infoQuery, function (catalog) {
            catalog.reverse().map(that._createItem).forEach(function (item) {
              node.appendSibling(item);
            });
            
            node.remove();
          });
        } else {
          node.setExpanded(!node.isExpanded());
        }
      });

      $(this._table).delegate('.fancytree-name', 'mouseup', function(e) {
        var node = $.ui.fancytree.getNode(e);

        if (!node.data.astValue.astNodeType)
          return;

        var elems = that.service.getReferences(
           node.data.astValue.astNodeId,
          (node.data.astValue.astNodeType === 'Namespace')
            ? model.RefTypes.GetUsage
            : model.RefTypes.GetDef);

        if (elems.length > 1) {
          util.buildAmbiguousRefPage(elems, that._openFile);
        } else {
          that._openFile(node.data.astValue.range, e);
        }
      });
    },

    _createItem : function (infonode) {
      var isMoreBtn = infonode.astValue.astNodeType === 'MoreBtn';

      var title = '';
      switch (infonode.astValue.astNodeType) {
        case 'Type':
        case 'Namespace': title = infonode.astValue.astNodeType; break;
        case 'MoreBtn': title = 'More...'; break;
      }

      return {
        title: title,
        name: infonode.label,
        documentation: infonode.astValue.documentation,
        folder: !isMoreBtn,
        lazy: !isMoreBtn,
        astValue: infonode.astValue,
        infoQuery: infonode.query,
        isMoreBtn: isMoreBtn,
        children: []
      };
    },

    _openFile : function (fileRange, event) {
      topic.publish('codecompass/openFile', {
        fileId     : fileRange.file,
        line       : fileRange.range.startpos.line,
        moduleId   : 'text',
        newTab     : mouse.isMiddle(event),
        newSession : true,
        selection  : {
          from : fileRange.range.startpos,
          to   : fileRange.range.endpos
        }
      });
    }
  });
  
  return declare(ContentPane, {
    buildRendering : function() {
      this.inherited(arguments);
      dom.place(dom.toDom('<h1>C/C++ Class Catalog</h1>'), this.domNode);
      this.addChild(new CatalogueTreeGrid({ service : model.cppservice }));
    }
  });

});
