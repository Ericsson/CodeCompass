define([
  'dojo/_base/declare',
  'dojo/dom-construct',
  'dojo/mouse',
  'dojo/on',
  'dojo/topic',
  'dojox/html/entities',
  'dijit/layout/ContentPane',
  'dijit/layout/LayoutContainer',
  'dijit/layout/TabContainer',
  'dijit/form/Button',
  'dijit/form/Form',
  'dijit/form/RadioButton',
  'codecompass/registration/model',
  'codecompass/registration/versionCommon',
  'dojo/date/locale',
  'dijit/Tooltip'
],
function (declare, dom, mouse, on, topic, entities, ContentPane, LayoutContainer,
  TabContainer, Button, Form, RadioButton, model, versionCommon, locale, Tooltip) {
  
  
  function onCompareSelectedClick(container, versionrepoid, path)
  {
    return function (evt) {
        var versioncommitfromid = container._radioBtnCurrStateFrom;
        var versioncommitid     = container._radioBtnCurrStateTo;
        
        topic.publish('codecompass/versionOpenHistoryDiff', {
          moduleId            : 'version',
          newTab              : mouse.isMiddle(evt),
          versionrepoid       : versionrepoid,
          versioncommitfromid : versioncommitfromid,
          versioncommitid     : versioncommitid,
          versionpath         : path,
        });
    };
  }
  
  function handleFromToRadioBtnClick(evt, container, fromOrTo)
  {
    if ("from" == fromOrTo) {
      container._radioBtnCurrStateFrom = evt.target.value;
    } else {
      container._radioBtnCurrStateTo = evt.target.value;
    }
  }
  
  function shortCommitMessage(message)
  {
    if (message.length > 70) {
      /* 50 should be the maximum but everybody will write longer messages anyway */
      return message.substr(0,69) + "...";
    } else {
      return message;
    }
  }
  
  function drawLogGraphToCanvas(canvas)
  {
    var ctx = canvas.getContext("2d");
    var drawInfo = JSON.parse(canvas.getAttribute("data-draw"));
    
    //commit lines
    ctx.lineWidth=2;
    for (var j = 0; j < drawInfo.l.length; ++j) {
      var currentDraw = drawInfo.l[j];
      
      ctx.beginPath();
      if (currentDraw.color) {
        ctx.strokeStyle = '#' + currentDraw.color;
      } else {
        ctx.strokeStyle = '#303030';
      }
      if (currentDraw.from != currentDraw.to) {
        ctx.moveTo(10 * currentDraw.from, 0);
        ctx.lineTo(10 * currentDraw.from, 10);
        ctx.lineTo(10 * currentDraw.to, 19);
        ctx.lineTo(10 * currentDraw.to, 20);
      } else {
        ctx.moveTo(10 * currentDraw.to, 0);
        ctx.lineTo(10 * currentDraw.to, 20);
      }
      ctx.stroke();
      
      //console.log(currentDraw);
    }
    
    //dot
    ctx.lineWidth=2;
    ctx.beginPath();
    ctx.arc(10*drawInfo.dot, 5, 4, 2*Math.PI, false);
    ctx.fillStyle = drawInfo.dotStyle == 1 ? 'white' : 'black';
    ctx.fill();
    ctx.strokeStyle = '#000000';
    ctx.stroke();
  }
  
  function appendReceivedCommitsToTable(
    container,
    commitTableDom,
    hasPath,
    showDiffRadioButtons,
    historyCommits)
  {
    //calculate canvas width
    var maxDotPos = 0;
    for (var i = 0; i < historyCommits.logList.length; ++i) {
      var drawinfo = historyCommits.logList[i].drawinfo;
      for (var j = 0; j < drawinfo.l.length; ++j) {
        maxDotPos = Math.max(maxDotPos, drawinfo.l[j].from);
        maxDotPos = Math.max(maxDotPos, drawinfo.l[j].to);
      }
    }
    var canvasWidth = 20 + maxDotPos * 10; 
    
    //append to table
    for (var i = 0; i < historyCommits.logList.length; ++i)
    {
      var currentCommit = historyCommits.logList[i].commit;
      
      var datetime = new Date(currentCommit.time * 1000);
      
      var commitDom = dom.create("tr", {}, commitTableDom);
      
      var commitGraphCanvasTdDom = dom.create("td", { }, commitDom);
      if (maxDotPos) {
        var commitGraphCanvasDom = dom.create("canvas", {
          class: "history-graphcanvas",
          width: canvasWidth, //canvas width/height does not affect coordinate system if set from css
          height: 20
        }, commitGraphCanvasTdDom);
      }
      if (showDiffRadioButtons) {
        var commitDiffFromDom = dom.create("td", { }, commitDom);
        var commitDiffFromBtn = new RadioButton({ type: "radio", name: "history-from", value: currentCommit.oid, onClick: function(evt){handleFromToRadioBtnClick(evt, container, "from")} });
        dom.place(commitDiffFromBtn.domNode, commitDiffFromDom);
        var commitDiffToDom   = dom.create("td", { }, commitDom);
        var commitDiffToBtn   = new RadioButton({ type: "radio", name: "history-to", value: currentCommit.oid, onClick: function(evt){handleFromToRadioBtnClick(evt, container, "to")} });
        dom.place(commitDiffToBtn.domNode, commitDiffToDom);
      }
      var commitIdDom       = dom.create("td", { }, commitDom);
      versionCommon.placeCommitLink(currentCommit, commitIdDom);
      var commitSummaryDom  = dom.create("td", { innerHTML: entities.encode(shortCommitMessage(currentCommit.summary)) }, commitDom);
      versionCommon.registerCommitTooltip(null, null, currentCommit, commitSummaryDom);
      var commitAuthorDom   = dom.create("td", { innerHTML: entities.encode(currentCommit.author) }, commitDom);
      var commitDateDom     = dom.create("td", {
        innerHTML: locale.format(datetime, "yyyy-MM-dd HH:mm"),
        style: "white-space: nowrap;"
      }, commitDom);
      
      //draw on the canvas
      if (maxDotPos) {
        var canvas = commitGraphCanvasDom;
        //canvas will be able to redraw itself if needed
        canvas.setAttribute("data-draw", JSON.stringify(historyCommits.logList[i].drawinfo));
        drawLogGraphToCanvas(canvas);
      }
    }
  }
  
  function onLoadMoreClick(
      container,
      loadMore,
      commitTableDom,
      hasPath,
      showDiffRadioButtons,
      versionrepoid,
      versionbranch,
      versionpath,
      nPageLength)
  {
    return function(){
      
      var historyCommits = model.versionservice.getFileRevisionLog(
        versionrepoid,
        versionbranch,
        versionpath,
        loadMore.continueatcommit,
        nPageLength,
        loadMore.continueatdrawstate
      );
      
      appendReceivedCommitsToTable(
        container,
        commitTableDom,
        hasPath,
        showDiffRadioButtons,
        historyCommits
      );

      //update button
      if (historyCommits.logList.length == nPageLength) {
        loadMore.continueatcommit = historyCommits.logList[historyCommits.logList.length-1].commit.oid;
        loadMore.continueatdrawstate = historyCommits.drawState;
      } else {
        //all loaded, remove button
        dom.destroy(loadMore.domNode);
      }
    }
  }
  
  function initializeVersionHistoryComponent(
    container,
    versionrepoid,
    versionbranch,
    versionpath)
  {
    var nPageLength = 100;
    
    var historyCommits = model.versionservice.getFileRevisionLog(
      versionrepoid,
      versionbranch,
      versionpath,
      "",
      nPageLength
    )
        
    var hasPath = versionpath ? true : false;
    var showDiffRadioButtons = true;
    
    dom.empty(container._hist.domNode);
    container._radioBtnCurrStateFrom = "";
    container._radioBtnCurrStateTo = "";
    
    dom.create("p", { innerHTML:
      "History " +
      (hasPath ? "of file <b>" + entities.encode(versionpath) + "</b> " : "") +
      "in branch <b>" + entities.encode(versionbranch) +
      "</b>" }, container._hist.domNode);

    if (showDiffRadioButtons) {
      var compareSelected = new Button({
        label   : 'Compare selected',
        onClick : onCompareSelectedClick(container, versionrepoid, versionpath)
      });
      dom.place(compareSelected.domNode, container._hist.domNode);
    }
      
    var commitTableDom = dom.create("table", { class : "history-table" }, container._hist.domNode);
    var commitTableHeadDom = dom.create("thead", { innerHTML:
      "<th></th>" +
      (showDiffRadioButtons ? '<th colspan="2"><span title="Compare">Cmp</span></th>' : "") +
      "<th>Id</th>" +
      "<th>Message</th>" + 
      "<th>Author</th>" + 
      "<th>Date</th>" }, commitTableDom);
    
    appendReceivedCommitsToTable(container, commitTableDom, hasPath, showDiffRadioButtons, historyCommits);
    
    if (historyCommits.logList.length == nPageLength) {
      var loadMore = new Button({
        label   : 'Load more...',
        continueatcommit: historyCommits.logList[historyCommits.logList.length-1].commit.oid,
        continueatdrawstate: historyCommits.drawState
      });
      on(loadMore, 'click', onLoadMoreClick(
          container,
          loadMore,
          commitTableDom,
          hasPath,
          showDiffRadioButtons,
          versionrepoid,
          versionbranch,
          versionpath,
          nPageLength)
      );
      dom.place(loadMore.domNode, container._hist.domNode);
      
    }

  }

  return declare(TabContainer, {
    constructor : function () {
      var that = this;

      this._hist = new ContentPane({
        id    : 'hist',
        title : 'History',
        style : 'padding: 10px'
      });
      
      topic.subscribe('codecompass/versionOpenHistory', function (message) {
        if (message.newTab)
          return;

        initializeVersionHistoryComponent(
          that,
          message.versionrepoid,
          message.versionbranch,
          message.versionpath
        );
      });
      
    },

    postCreate : function () {
      this.addChild(this._hist);
      
      // This is needed for the active tab to initially appear
      this.startup();
    },
    
    /**
     * This function restores the state of the module based on a given state
     * object. This state object basically comes from the URL.
     * @param {Object} state Key-value pairs of state.
     */
    setState : function (state) {
      if (state.cent !== this.id)
        return;
      
      var versionrepoid = state.versionrepoid || "";
      var versionbranch = state.versionbranch || "";
      var versionpath   = state.versionpath   || "";

      initializeVersionHistoryComponent(
        this,
        versionrepoid,
        versionbranch,
        versionpath);
    },
  });
});