define([
  'dojo/dom-construct',
  'dojo/dom-style',
  'dojo/_base/array',
  'dojo/_base/window',
  'dojo/topic',
  'dojox/layout/ToggleSplitter',
  'dijit/DropDownMenu',
  'dijit/MenuItem',
  'dijit/TitlePane',
  'dijit/form/DropDownButton',
  'dijit/form/Select',
  'dijit/layout/AccordionContainer',
  'dijit/layout/BorderContainer',
  'dijit/layout/ContentPane',
  'codecompass/viewHandler',
  'codecompass/urlHandler',
  'codecompass/registration/model',
  'codecompass/registration/moduleRegistration',
  'codecompass/view/component/Breadcrumb',
  'codecompass/view/component/SearchFields',
  'codecompass/util'],
function (dom, style, array, win, topic, ToggleSplitter, DropDownMenu, MenuItem,
  TitlePane, DropDownButton, Select, AccordionContainer, BorderContainer,
  ContentPane, viewHandler, urlHandler, model, moduleRegistration, Breadcrumb,
  SearchFields, util) {
  
  /**
   * ContentPane for center modules.
   */
  var center;
  
  /**
   * AccordionContainer.
   */
  var accordion;
  
  /**
   * This variable holds the id of the current center module. This is used
   * because of performance reasons.
   */
  var currentCenterModuleId;
  
  /**
   * This is a TitlePane object which contains operational buttons which are
   * provided context sensitively by the modules.
   */
  var contextButtons;
  
  /**
   * This function populates the Accordion panel with registrated accordion
   * modules.
   * @param {AccordionContainer} accordion The accordion in which the
   * appropriate modules will be placed.
   */
  function populateAccordion(accordion) {
    array.forEach(viewHandler.getModules(viewHandler.moduleType.Accordion),
    function (module) {
      accordion.addChild(module);
    });
  }
  
  function createWorkspaceCombo() {
    var workspaces = [];
    
    array.forEach(model.workspaceservice.getWorkspaces(),
    function (workspaceInfo) {
      workspaces.push({
        label : workspaceInfo.description,
        value : workspaceInfo.id
      });
    });
    
    return new Select({
      options  : workspaces,
      value    : urlHandler.getStateValue('wsid'),
      onChange : function (value) {
        urlHandler.setState({'wsid': value});
        location.reload(true);
      }
    });
  }
  
  /**
   * This function populates the header with logo and search text fields.
   * @param {ContentPane} header Header panel.
   */
  function populateHeader(header) {
    
    //--- Search fields ---//
    
    var searchFields = new SearchFields();
    header.addChild(searchFields);
    
    //--- Breadcrumb ---//
    
    var breadcrumb = new Breadcrumb();
    header.addChild(breadcrumb);
    
    //--- Menu ---//
    
    var menu = new DropDownMenu();
    
    menu.addChild(new MenuItem({
      label     : 'About',
      iconClass : 'menuicon workspaceicon',
      onClick   : function () {
        topic.publish('codecompass/infopage', { page : 'startpage' });
      }
    }));
    
    menu.addChild(new MenuItem({
      label     : 'Report a bug here',
      iconClass : 'menuicon bugreporticon',
      onClick   : function () {
        window.open(
          'https://eforge.ericsson.se/sf/tracker/do/listTrackers/projects.codecompass/tracker');
      }
    }));
    
    menu.addChild(new MenuItem({
      label     : 'User guide',
      iconClass : 'menuicon userguideicon',
      onClick   : function () {
        topic.publish('codecompass/infopage', { page : 'userguide' });
      }
    }));
    
    menu.addChild(new MenuItem({
      label     : 'Credits',
      iconClass : 'menuicon creditsicon',
      onClick   : function () {
        topic.publish('codecompass/infopage', { page : 'credits' });
      }
    }));
    
    var menuButton = new DropDownButton({
      dropDown : menu,
      label    : 'Menu',
      style    : 'position: absolute; right: 10px; top: 10px'
    });
    
    var workspaceCombo = createWorkspaceCombo();
    workspaceCombo.set('style', 'position: absolute; right: 10px; top: 38px');
    document.title = workspaceCombo.get('value') + ' - CodeCompass';
    
    header.addChild(menuButton);
    header.addChild(workspaceCombo);
  }
  
  /**
   * This function places the center module in the center view with given id.
   * @param {String | Number} id Id of center module to place in the middle.
   */
  function setCenterModule(id) {
    if (id === currentCenterModuleId)
      return;
    
    var module = viewHandler.getModule(id);
    
    if (module === undefined)
      throw 'There is no center module with this id: ' + id;
    
    var prevModule = viewHandler.getModule(currentCenterModuleId);
    if (prevModule && prevModule.onUnload)
      prevModule.onUnload(center);
    
    center.getChildren().forEach(function (child) {
      center.removeChild(child);
    });
    
    center.addChild(module);
    setTimeout(function () { center.resize(); }, 0);
    
    if (module.onLoad)
      module.onLoad(center, urlHandler.getState());
    
    currentCenterModuleId = id;
    
    style.set(
      contextButtons.domNode, 'display', id === 'infopage' ? 'none' : 'block');
  }
  
  function setTopicChecker()
  {
    require(['dojo/aspect', 'codecompass/registration/topics'],
    function (aspect, topics) {
      aspect.before(topic, 'publish', function (label, message) {
        if (label.indexOf('codecompass') !== 0)
          return;
        
        var undocumentedAttrs = topics.getUndocumentedAttrs(label, message);
        if (undocumentedAttrs.length !== 0)
          console.error(
            'The attributes of a topic message has to be documented',
            label,
            undocumentedAttrs);
      });
    });
  }
  
  /**
   * This function builds the page. It places a BorderContainer in the page
   * body. This container has a header, an accordion panel and a center module.
   * The accordion panel contains the registrated accordion panels.
   * @param {String | Number} defaultCenter Id of the default center module.
   */
  function buildPage(defaultCenter) {

    //--- Create main layout ---//

    var layout = new BorderContainer({
      style : 'height: 100%;'
    }, dom.create('div', null, win.body()));

    //--- Create header ---//

    var header = new ContentPane({
      region : 'top',
      style  : 'height: 90px;'
    }, dom.create('div', { class : 'mainheader' }));

    populateHeader(header);
    layout.addChild(header);

    //--- Create accordion panel ---//

    accordion = new AccordionContainer({
      region   : 'left',
      style    : 'width: 300px;',
      splitter : true
    });
    
    populateAccordion(accordion);
    layout.addChild(accordion);
    
    accordion.watch('selectedChildWidget', function () {
      var prevSelectedId = '';
      
      return function (attr, from, to) {
        if (to.id !== prevSelectedId) {
          to.onShow();
          prevSelectedId = to.id;
        }
      };
    });

    //--- Create center panel ---//

    center = new ContentPane({ region : 'center' });

    var defaultModule = viewHandler.getModule(defaultCenter);

    currentCenterModuleId = defaultCenter;
    center.addChild(defaultModule);
    defaultModule.onLoad(center, urlHandler.getState());
    layout.addChild(center);
    
    //--- Create Context buttons panel ---//
    
    contextButtons = new TitlePane({
      title : 'Context Buttons',
      class : 'contextButtons titlePaneBottomFix titlePaneRightFix',
      style : 'display: none'
    });
    
    viewHandler.getModules().forEach(function (module) {
      if (module.contextButton) {
        if (module.contextButton instanceof Array)
          module.contextButton.forEach(function (button) {
            if (button)
              contextButtons.addChild(button);
          });
        else
          contextButtons.addChild(module.contextButton);
      }
    });
    
    dom.place(contextButtons.domNode, win.body());
    
    //--- Load layout ---//

    setTimeout(function () { layout.startup(); }, 0);
    
    // This is a big hack so the user can middle click on a tree node and the
    // event be delegated to the tree instead of scroll icon shown by the
    // browser.
    document.addEventListener('mousedown', function (event) {
      if (event.button === 1)
        require(['dojo/query'], function (query) {
          var node = query(event.target).closest('.dijitTreeNode')[0];
          if (!node) return;
          
          var clicked = dijit.byNode(node);
          if (!clicked) return;
    
          clicked.tree.onClick(clicked.item, clicked, event);
          event.preventDefault();
        });
    }, true);
  }

  /**
   * This function initializes all modules according to the url.
   * @param {Object} url Key-value pairs in the url.
   */
  function initByUrl(url) {
    array.forEach(viewHandler.getModules(), function (module) {
      if (module.setState)
        module.setState(url);
    });
    
    if (url.cent)
      setCenterModule(url.cent);
  }

  //--- Message and URL handling ---//

  topic.subscribe('codecompass/openFile', function (message) {
    var selection = '1|1|1|1';
    
    if (message.selection)
      selection = util.rangeToString(message.selection);

    if (message.newTab) {
      window.open(
        window.location.protocol + '//' + window.location.host +
        '/#wsid=' + urlHandler.getStateValue('wsid') +
        '&fid=' + message.fileId.fid +
        '&select=' + selection +
        (message.moduleId ? '&cent=' + message.moduleId : ''));
      return;
    }

    if (message.moduleId)
      setCenterModule(message.moduleId);

    if (message.jumpFrom) {
      var selectionFrom = util.rangeToString(
        message.jumpFrom.range.startpos,
        message.jumpFrom.range.endpos);
      
      urlHandler.setStateValue('select', selectionFrom);
    }
    
    urlHandler.setStateValue({
      fid    : message.fileId.fid,
      select : selection,
      cent   : 'text'
    });
  });

  topic.subscribe('codecompass/openDiagram', function (message) {
    setCenterModule('diagram');
    
    var state = {
      diagNode : message.diagramNode,
      diagType : message.diagramType,
      cent     : 'diagram'
    };
    
    if (message.fromNodeId)
      state.diagFrom = message.fromNodeId.astNodeId;
    
    urlHandler.setStateValue(state);
  });
  
  topic.subscribe('codecompass/codebites', function (message) {
    setCenterModule('codebites');
    
    urlHandler.setStateValue({
      diagNode : message.astNodeInfo.astNodeId.astNodeId,
      cent     : 'codebites'
    });
  });

  topic.subscribe('codecompass/references', function (message) {
    accordion.selectChild(viewHandler.getModule('queryresult'), true);
  });

  topic.subscribe('codecompass/search', function (message) {
    accordion.selectChild(viewHandler.getModule('queryresult'), true);
  });

  topic.subscribe('codecompass/infotree', function (message) {
    var infoTree = viewHandler.getModule('infotree');
    infoTree.loadInfoTree(message.elementInfo, message.fileType);
    accordion.selectChild(infoTree, true);
  });
  
  topic.subscribe('codecompass/openPath', function (path) {
    accordion.selectChild(viewHandler.getModule('filemanager'), true);
  });

  topic.subscribe('codecompass/infopage', function (message) {
    setCenterModule('infopage');
    viewHandler.getModule('infopage').selectPage(message.page);
  });
  
  topic.subscribe('codecompass/setstate', function (message) {
    urlHandler.setState(message);
    initByUrl(message);
  });


  topic.subscribe('codecompass/versionOpenText', function (message) {
    setCenterModule('versiontext');

    urlHandler.setStateValue({
      cent            : 'versiontext',
      versionrepoid   : message.versionrepoid,
      versioncommitid : message.versioncommitid,
      versionpath     : message.versionpath
    });
  });
      
  topic.subscribe('codecompass/versionOpenCommit', function (message) {
    setCenterModule('versioncommit');

    urlHandler.setStateValue({
      cent            : 'versioncommit',
      versionrepoid   : message.versionrepoid,
      versioncommitid : message.versioncommitid
    });
  });

  topic.subscribe('codecompass/versionOpenHistory', function (message) {
    setCenterModule('versionhistory');

    urlHandler.setStateValue({
      cent            : 'versionhistory',
      versionrepoid   : message.versionrepoid,
      versionbranch   : message.versionbranch,
      versionpath     : message.versionpath
    });
  });
      
  topic.subscribe('codecompass/versionOpenHistoryDiff', function (message) {
    setCenterModule('versionhistorydiff');

    urlHandler.setStateValue({
      cent                : 'versionhistorydiff',
      versionrepoid       : message.versionrepoid,
      versioncommitfromid : message.versioncommitfromid,
      versioncommitid     : message.versioncommitid,
      versionpath         : message.versionpath
    });
  });

  topic.subscribe('codecompass/metrics', function (message) {
    setCenterModule('metrics');
    
    urlHandler.setStateValue({
      cent : 'metrics'
    });
  });
      
  topic.subscribe('codecompass/doxygen', function (message) {
    setCenterModule('doxygen');
    urlHandler.setStateValue({ cent : 'doxygen' });
  });
  
  topic.subscribe('codecompass/statistics', function () {
    setCenterModule('statistics');
  });
  
  topic.subscribe('codecompass/catalog', function () {
    setCenterModule('catalog');
    urlHandler.setStateValue({ cent : 'catalog' });
  });
  
  var state = urlHandler.getState();
  if (!state.wsid)
    state.wsid = model.workspaceservice.getWorkspaces()[0].id;
  
  urlHandler.setState(state);

  topic.subscribe('/dojo/hashchange', function (url) {
    if (urlHandler.hashSetProgress) return;
    initByUrl(urlHandler.getState());
  });

  //--- CodeCompass starter ---//

  return function () {
    moduleRegistration.registration();
    setTopicChecker();
    buildPage('infopage');
    initByUrl(urlHandler.getState());      
  };
});
