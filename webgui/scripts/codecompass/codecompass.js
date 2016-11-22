define([
  'dojo/dom-construct',
  'dojo/dom-style',
  'dojo/topic',
  'dijit/TitlePane',
  'dijit/layout/AccordionContainer',
  'dijit/layout/BorderContainer',
  'dijit/layout/ContentPane',
  'codecompass/viewHandler',
  'codecompass/urlHandler',
  'codecompass/model'],
function (dom, style, topic, TitlePane, AccordionContainer, BorderContainer,
  ContentPane, viewHandler, urlHandler, model) {

  /**
   * ContentPane at the top for search modules and header stuff.
   */
  var header;

  /**
   * ContentPane for center modules.
   */
  var center;

  /**
   * AccordionContainer for the selectable modules at the left side.
   */
  var accordion;

  /**
   * This variable holds the ID of the current center module.
   */
  var currentCenterModuleId;

  /**
   * This is a TitlePane object which contains operational buttons which are
   * provided context sensitively by the modules.
   */
  var contextButtons;

  /**
   * This function places the center module in the center view with given ID.
   * @param {String} id ID of center module to place in the middle.
   */
  function setCenterModule(id) {
    setContextButton(id);

    if (id === currentCenterModuleId)
      return;

    var module = viewHandler.getModule(id);

    if (!module)
      throw 'There is no center module with this id: ' + id;

    if (module.onUnload)
      module.onUnload(center);

    center.getChildren().forEach(function (child) {
      center.removeChild(child);
    });

    center.addChild(module);

    if (module.onLoad)
      module.onLoad(center, urlHandler.getState());

    setTimeout(function () { center.resize(); }, 0);

    currentCenterModuleId = id;

    style.set(
      contextButtons.domNode, 'display', id === 'infopage' ? 'none' : 'block');
  }

  /**
   * This function removes the current children of context button and shows the
   * actual items by center module id.
   * @param id Center module id
   */
  function setContextButton(id) {
    contextButtons.getChildren().forEach(function (child) {
      contextButtons.removeChild(child);
    });

    var state = urlHandler.getState();

    if (!state.fid)
      return;

    var fileInfo = model.project.getFileInfo(state.fid);

    viewHandler.getModules({
      type   : viewHandler.moduleType.ContextButton,
      center : id
    }).forEach(function (module) {
      var item = module.render(fileInfo);
      if (item) {
        style.set(item.domNode, 'width', '100%');
        contextButtons.addChild(item);
      }
    });
  }

  /**
   * This function builds the page. It places a BorderContainer in the page
   * body. This container has a header, an accordion panel and a center module.
   * The accordion panel is filled by the registrated accordion panels.
   * ContextButtons module is also placed in the DOM tree as an invisible
   * module.
   * @param {String} defaultCenter ID of the default center module which appears
   * in the beginning.
   */
  function buildPage(defaultCenter) {

    //--- Create main layout ---//

    var layout = new BorderContainer({
      style : 'height: 100%;'
    }, dom.create('div', null, window.document.body));

    //--- Create header ---//

    header = new ContentPane({
      id     : 'cc-mainheader',
      region : 'top'
    });

    var logo = dom.create("div", {id : 'cc-headerlogo'});
    dom.place(logo, header.domNode);

    viewHandler.getModules({
      type : viewHandler.moduleType.Header
    }).forEach(function (module) {
      header.addChild(module);
    });

    layout.addChild(header);

    //--- Create accordion panel ---//

    accordion = new AccordionContainer({
      region   : 'left',
      style    : 'width: 300px;',
      splitter : true
    });

    viewHandler.getModules({
      type : viewHandler.moduleType.Accordion
    }).forEach(function (module) {
      accordion.addChild(module);
    });

    layout.addChild(accordion);

    //--- Create center panel ---//

    center = new ContentPane({ region : 'center' });    
    center.addChild(viewHandler.getModule(defaultCenter));
    
    layout.addChild(center);

    currentCenterModuleId = defaultCenter;

    //--- Create Context buttons panel ---//

    contextButtons = new TitlePane({
      id    : 'contextbutton',
      title : 'Context Buttons',
      class : 'contextButtons',
      style : 'display: none'
    });

    dom.place(contextButtons.domNode, window.document.body);

    //--- Load layout ---//

    layout.startup();
  }

  /**
   * This function initializes all modules according to the url.
   * @param {Object} url Key-value pairs in the url.
   */
  function initByUrl(url) {
    viewHandler.getModules().forEach(function (module) {
      if (module.setState)
        module.setState(url);
    });

    if (url.center)
      setCenterModule(url.center);
  }

  //--- Message and URL handling ---//

  topic.subscribe('codecompass/setCenterModule', function (moduleId) {
    setCenterModule(moduleId);
  });

  topic.subscribe('codecompass/selectAccordion', function (moduleId) {
    accordion.selectChild(viewHandler.getModule(moduleId), true);
  });

  topic.subscribe('/dojo/hashchange', function (url) {
    if (urlHandler.hashSetProgress) return;
    initByUrl(urlHandler.getState());
  });

  var state = urlHandler.getState();
  if (!state.wsid)
    state.wsid = model.workspace.getWorkspaces()[0].id;

  urlHandler.setState(state);

  //--- CodeCompass starter ---//

  return function () {
    buildPage('infopage');
    initByUrl(urlHandler.getState());
  };
});
