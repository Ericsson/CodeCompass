define([
  'dojo/mouse',
  'dojo/on',
  'dojo/_base/declare',
  'dojo/_base/window',
  'dojo/topic',
  'dojo/dom-attr',
  'dojo/dom-construct',
  'dojo/dom-style',
  'dojo/query!css2',
  'dijit/Dialog',
  'dijit/Tooltip',
  'dijit/form/Button',
  'dijit/form/ToggleButton',
  'dijit/layout/BorderContainer',
  'dijit/layout/ContentPane',
  'codecompass/registration/diagramProvider',
  'codecompass/registration/model',
  'dojo/NodeList-traverse'],
function (mouse, on, declare, win, topic, attr, dom, style, query, Dialog, 
  Tooltip, Button, ToggleButton, BorderContainer, ContentPane, diagramProvider, model) {
  
  /**
   * This function sets the onClick() event of the context buttons and makes
   * them visible
   * @param {Diagram} diagram An instance of Diagram module.
   * @param {String} svg The diagram in svg format is necessary to display it in
   * new window when "Export SVG" is clicked.
   */
  function setContextButtons(diagram, svg) {
    var fileInfo = diagram._textModule.getFileInfo();
    var contextButton = diagram.contextButton;
    
    contextButton[0].onClick = function () {
      topic.publish('codecompass/openFile', {
        moduleId : 'text',
        fileId   : fileInfo.file
      });
    };
    
    contextButton[1].onClick = function () {
      window.open().document.body.innerHTML = svg;
    };
    
    contextButton[2].onClick = function () {
      var service = model.getLanguageService(fileInfo.type);

      if (!service)
        return;

      var legend = service.getLegend(diagram._diagType);

      new Dialog({
        title   : 'Diagram Legend',
        content : legend || '<b>No Legend</b>',
        style   : legend ? '' : 'width: 200px'
      }).show();
    };

    if (contextButton[3])
      contextButton[3].onClick = function () {
        this.set(
          'label',
          contextButton[3].checked ? "Reduced view" : "Non-reduced view");
          
        diagram.loadDiagram(
          diagram.diagramNode,
          model.DiagramId.MI_HIERARCHY_DIAGRAM,
          !contextButton[3].checked);
      };

    contextButton.forEach(function (button) {
      style.set(button.domNode, 'display', 'block');
    });
  }
  
  function showTooltip(diagram, svgNode) {
    var diagramData = diagramProvider.diagramTypes[diagram._diagType];

    if (!diagramData.getTooltip)
      return;

    var astNodeId = new model.AstNodeId();
    astNodeId.astNodeId = svgNode.id;
    
    try {
      if (diagram._diagType == model.DiagramId.MI_HIERARCHY_DIAGRAM) {
        astNodeId = svgNode;
      }
      diagramData.getTooltip(astNodeId, function (tooltip) {
        if (tooltip) {
          Tooltip.show(tooltip, svgNode, ['after-centered', 'before-centered']);
            
          on.once(query('.dijitTooltip')[0], mouse.enter, function () {
            diagram._isMouseOverTooltip = true;
          });
          on.once(query('.dijitTooltip')[0], mouse.leave, function () {
            Tooltip.hide(svgNode);
            diagram._isMouseOverTooltip = false;
          });
        }
      });
    } catch (ex) {
    }
  }
  
  function setMouseEvents(diagram) {
    var isMouseOverSVGElement = false;
    var clickPos = { x : 0, y : 0 };
    
    function onMouseOver(svgElement) {
      
      isMouseOverSVGElement = true;
        
      setTimeout(function () {
        if (!isMouseOverSVGElement)
          return;
        
        // In case of MIHierarchy diagrams, always show tooltips.
        if (diagram._diagType != model.DiagramId.MI_HIERARCHY_DIAGRAM)
          if (svgElement.id === diagram._lastMouseoverNodeId)         
            return;
        diagram._lastMouseoverNodeId = svgElement.id;
      
        var diagramData = diagramProvider.diagramTypes[diagram._diagType];
        var info = diagramData.mouseOverInfo(svgElement.id);
      
        diagram._textModule.loadFile(info.fileId);
        diagram._textModule.set('selection', info.selection);
        diagram._textModule.jumpToPos(info.selection.from.line);
      
        showTooltip(diagram, svgElement);
      }, 1000);
    }
    
    function onMouseOut(svgElement) {
      var that = svgElement;
      
      isMouseOverSVGElement = false;
       
      setTimeout(function () {
        if (!diagram._isMouseOverTooltip)
          Tooltip.hide(that);
      }, 300);
    }
    
    var edges = query('.edge', diagram._diagCont.domNode);
    edges.on('mouseover', function() {
    	onMouseOver(this);
    });
    edges.on('mouseout', function(event) {
    	onMouseOut(this);
    });
    
    edges.on('click', function (event) {
      if (clickPos.x !== event.clientX || clickPos.y !== event.clientY)
        return;
    });

    edges.on('mousedown', function (event) {
      clickPos = { x : event.clientX, y : event.clientY };
    });
    
    var nodes = query('.node', diagram._diagCont.domNode);
    nodes.on('mouseover', function() {
    	onMouseOver(this);
    });
    nodes.on('mouseout', function(event) {
    	onMouseOut(this);
    });
    
    nodes.on('click', function (event) {
      if (clickPos.x !== event.clientX || clickPos.y !== event.clientY)
        return;
           
      topic.publish('codecompass/openDiagram', {
        diagramNode : this.id,
        diagramType : diagram._diagType,
        other : !diagram.contextButton[3].checked
      });
    });
    
    nodes.on('mousedown', function (event) {
      clickPos = { x : event.clientX, y : event.clientY };
    });
  }
  
  function setDiagramZoomable(diagram) {
    var svgDom = query('svg')[0];

    attr.set(svgDom, {
      width  : '100%',
      height : '100%',
      style  : 'margin-top: -24px' // TODO: shouldn't be hardcoded
    });

    diagram._svgPanZoom = svgPanZoom(svgDom, {
      fit     : false,
      center  : true,
      maxZoom : Infinity,
      minZoom : 0.1,
      dblClickZoomEnabled: false,
      controlIconsEnabled : true
    });
  }
  
  return declare(BorderContainer, {
    constructor : function (params) {
      var that = this;

      /**
       * The message object has to have two numeric attributes: diagNode and
       * diagType.
       */
      topic.subscribe('codecompass/openDiagram', function (message) {
        that.loadDiagram(message.diagramNode, message.diagramType, message.other);
        that._textModule.jumpToPos(0);
      });

      this._textModule = params.textModule;

      //--- Diagram region ---//

      this._diagCont = new ContentPane({
        region   : 'top',
        splitter : true,
        style    : 'height: 70%; width: 100%'
      });

      //--- Text region ---//

      this._textModuleWrapper = new ContentPane({
        region : 'center',
        style  : 'border-top: 1px solid gray'
      });
      
      //--- Context buttons ---//
      
      this.contextButton = [
        new Button({ label : 'Text View'  }),
        new Button({ label : 'Export SVG' }),
        new Button({ label : 'Legend'     }),
        new ToggleButton({ label : 'Non-reduced view' })
      ];
      
      this.contextButton.forEach(function (button) {
        style.set(button.domNode, 'width', '100%');
        style.set(button.domNode.firstChild, 'display', 'block');
        style.set(button.domNode, 'margin-left', '0px');
        style.set(button.domNode, 'display', 'none');
      });
    },

    postCreate : function () {
      var that = this;
      
      this.addChild(this._diagCont);
      this.addChild(this._textModuleWrapper);

      this.set('style', 'height: 100%');
      this.set('gutters', false);
      
      require(['dojo/aspect'], function (aspect) {
        aspect.after(that._diagCont, 'resize', function () {
          if (that._svgPanZoom)
            that._svgPanZoom.resize();
        }, true);
      });
    },
    
    /**
     * This function loads the diagram in the diagram node.
     * @param {Number|String} diagramNode Id of a diagram node.
     * @param {Number} diagramType Diagram type. The diagram type has to be
     * registered in the diagramProvider.
     * @param {Any} other This parameter is going to be passed to the
     * getDiagram() function which is provided by diagramProvider.
     */
    loadDiagram : function (diagramNode, diagramType, other) {
      var that = this;
      this.diagramNode = diagramNode;
      var diagramData = diagramProvider.diagramTypes[diagramType];
      
      //--- Get diagram from the server ---//

      var fileInfo = this._textModule.getFileInfo();
      
      try {
        this._diagCont.set('content', dom.create('span', {
          class : 'diagram-loading'
        }));

        diagramData.getDiagram(diagramNode, fileInfo, function (svg) {
          if (svg) {
            var svgDom = dom.toDom(svg);
            
            // Remove default browser tooltips.
            // TODO: Sometimes the default browser tooltips are in an <a> tag
            // which contains an xlink:title attribute. This is the case for
            // example in "provides" relation where the provided functions are
            // listed in the tooltip. These should also be placed in a Dojo
            // Tooltip so these can also be removed from the diagram.
            query('.node title', svgDom).forEach(function (node) {
              dom.destroy(node);
            });
            
            query('.edge title', svgDom).forEach(function (node) {
                dom.destroy(node);
              });
            
            query('.graph title', svgDom).forEach(function (node) {
                dom.destroy(node);
              });
            
            that._diagCont.set('content', svgDom);
          } else {
            that._diagCont.set('content', dom.create('div', {
              innerHTML : 'No diagram',
              style     : 'margin: 100px 0px 0px 10px;\
                           text-align: center;\
                           font-weight: bold'
            }));
          }
          
          that._diagType = diagramType;
          
          //--- Display context buttons ---//
          
          setContextButtons(that, svg);
          
          //--- Mouse events on nodes ---//
          
          setMouseEvents(that);
          
          //--- Make SVG zoomable ---//
          
          setDiagramZoomable(that);
        }, other);
      } catch (ex) {
        console.error(ex);
        new Dialog({
          title   : 'Error',
          content : '<img src="/images/error_icon.png" class="erroricon" />\
                    Graph generation seems to be too long.'
        }).show();
      }
    },

    /**
     * This function restores the state of the module based on a given state
     * object. This state object basically comes from the URL.
     * @param {Object} state Key-value pairs of state.
     */
    setState : function (state) {
      if (state.cent !== this.id || !state.diagType || !state.diagNode)
        return;

      this.loadDiagram(state.diagNode, state.diagType);
    },

    onLoad : function () {
      this._textModuleWrapper.addChild(this._textModule);
    },
    
    onUnload : function () {
      this.contextButton.forEach(function (button) {
        style.set(button.domNode, 'display', 'none');
      });
    }
  });
});
