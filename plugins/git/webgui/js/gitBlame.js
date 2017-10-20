define([
  'dojo/on',
  'dojo/topic',
  'dojo/_base/declare',
  'dojo/_base/Color',
  'dojo/dom-construct',
  'dijit/Tooltip',
  'codecompass/view/component/Text',
  'codecompass/model',
  'codecompass/viewHandler',
  'codecompass/urlHandler',
  'codecompass/util',
  'codecompass/view/gitUtil'],
function (on, topic, declare, Color, dom, Tooltip, Text, model, viewHandler,
  urlHandler, util, gitUtil) {

  model.addService('gitservice', 'GitService', GitServiceClient);

  /**
   * This object caches tooltip information for commits.
   */
  var commitCache = {};

  /**
   * Shows a tooltip with commit information on a domNode.
   * @note If the tooltip has already been shown it will return tooltip
   * information from the cache.
   * @param {GitCommit} Commit Thrift git commit object.
   * @param {DomNode} domNode A domnode for tooltip placement.
   */
  function registerCommitTooltip(repoId, commitId, domNode) {
    if (commitId === '')
      return;

    on(domNode, 'mouseenter', function (evt) {
      var tooltipId = repoId + commitId;
      if (!commitCache[tooltipId]) {
        var commit = model.gitservice.getCommit(repoId, commitId);
        commitCache[tooltipId] = gitUtil.createTooltip(commit);
      }

      Tooltip.show(commitCache[tooltipId], domNode, ['above']);
      on.once(domNode, 'mouseleave', function(){
        Tooltip.hide(domNode);
      })
    });
  }

  /**
   * This function creates a blame info object for a given blame hunk. This info
   * object describes the visual representation of the revision control commit
   * in the blame view at the beginning of the line.
   * @param {VersionBlameHunk} blame Thrift object.
   * @param {Number} nameWidth The width (number of characters) of the commit
   * message column, i.e. the length of the longest name.
   * @param {Number} minTime This is the smallest time value. This is used to
   * calculate the commit color which will be gradient between green and red.
   * @param {Number} maxTime This is the greatest time value.
   */
  function createBlameForLine(blame, nameWidth, minTime, maxTime) {
    var commitLen = 16; // 'YYYY-MM-DD hh:mm'
    var commitTimeUNIX = blame.finalSignature.time;
    var message  = '', /*!< Commit message */
        author   = '', /*!< Commit author */
        time     = '', /*!< Commit time */
        commitId = '', /*!< Commit id */
        commitColor;

    if (0 === commitTimeUNIX) {
      message     = '(Not Committed Yet)';
      commitColor = new Color([216, 216, 216]);
    } else {
      message     = blame.finalCommitMessage;
      commitId    = blame.finalCommitId;
      author      = blame.finalSignature.name;
      time        = util.timeAgo(new Date(blame.finalSignature.time * 1000));

      // Get in [0, 1] range
      var commitHeat = (commitTimeUNIX - minTime) / (maxTime - minTime);
      // Convert to rgb. This conversion algorithm is pretty lame.
      commitColor = new Color([
        Math.round(128 + (1 - commitHeat) * 127),
        Math.round(128 +      commitHeat  * 127),
        128]);
    }

    var avatarLabel = author.charAt(0).toUpperCase();
    return {
      text     : '<div class="git-avatar" style="background-color:'
               + util.strToColor(author) + '">' + avatarLabel + '</div>'
               + '<div class="git-message">' + message
               + '</div><span class="git-time">' + time  + '</span>',
      commitId : commitId,
      color    : commitColor.toCss()
    };
  }
  
  /**
   * This function queries the blame information from the server and returns
   * and array which contains these information.
   * @return The function returns an array of objects which have the following
   * properties: text, commitId, color.
   */
  function caclulateAnnotations(repoId, commitId, path, maybeModifiedFileId) {

    var blameInfo = model.gitservice.getBlameInfo(
      repoId, commitId, path, maybeModifiedFileId);

    var blameForLines = [];

    var nameMaxLen = 0;
    var minTime    = Infinity;
    var maxTime    = -1;

    blameInfo.forEach(function (blame) {
      // Calculate max name length to allow narrower annotations if none has
      // a long name.
      var currLen = blame.finalSignature.name.length;
      if (currLen > nameMaxLen) {
        nameMaxLen = currLen;
      }

      // Calculate trac-like coloring based on commit date
      var currTime = blame.finalSignature.time;
      if (0 === currTime) return;
      if (currTime > maxTime) maxTime = currTime;
      if (currTime < minTime) minTime = currTime;
    });

    if (minTime === maxTime) 
      --minTime; // This makes one-commit files green

    blameInfo.forEach(function (blame) {
      var blameForLine
        = createBlameForLine(blame, nameMaxLen, minTime, maxTime);
      for (var i = 0; i < blame.linesInHunk; ++i)
        blameForLines.push(blameForLine);
    });

    return blameForLines;
  }

  /**
   * This function returns a line number formatter which is based on the
   * repository. The line numbers are added blame information.
   */
  function getBlameFormatter(repoId, branch, i, annotateInfo, showCommitInfo) {

    var blameLine = dom.create('div', {
      class : 'blame-line ' + (showCommitInfo ? 'blame-line-full' : ''),
    });

    if (showCommitInfo) {
      var commitInfo = dom.create('span', {
        innerHTML : annotateInfo.text,
        class     : 'blame-commit-info',
        onclick   : function () {
          Tooltip.hide(this);
          if (annotateInfo.commitId !== '')
            topic.publish('codecompass/gitCommitView', {
              center     : 'gitcommitview',
              gitrepo    : repoId,
              gitcommit  : annotateInfo.commitId,
              gitbranch  : branch
            });
        }
      }, blameLine);
      registerCommitTooltip(repoId, annotateInfo.commitId, commitInfo);
    } else {
      var commitInfo = dom.create('span', {
        innerHTML : '&nbsp;',
        class     : 'blame-commit-info'
      }, blameLine);
    }

    var lineNumber = dom.create('span', {
      innerHTML : i + 1,
      class     : 'blame-line-number',
      style     : 'background-color: ' + annotateInfo.color
    }, blameLine);

    return blameLine;
  }

  var GitBlame = declare(Text, {
    postCreate : function () {
      this.inherited(arguments);
      this._codeMirror.setOption('gutters', ['blame-gutter']);
      this._codeMirror.setOption('lineNumbers', false);
    },

    /**
     * This function sets blame information for each line in the editor.
     * @note We set blame information by editing gutter marker of the
     * CodeMirror editor. We set the line numbers here too.
     * @param {String} path 
     * @param {Integer} fileId
     */
    loadBlameView : function (path, fileId) {
      this.set('selection', [1,1,1,1]);
      var res = model.gitservice.getRepositoryByProjectPath(path);

      if (!res.isInRepository)
        return;

      var annotations = caclulateAnnotations(
        res.repoId, res.commitId, res.repoPath, fileId);

      for (var i = 0; i < this._codeMirror.lineCount(); ++i) {
        if (!annotations[i])
          continue;

        var showCommitInfo =
          (i == 0 || annotations[i].commitId != annotations[i - 1].commitId);
        this._codeMirror.setGutterMarker(i, "blame-gutter",
          getBlameFormatter(res.repoId, res.activeReference, i, annotations[i],
            showCommitInfo));
      }
    },

    setState : function (state) {
      if (state.center !== this.id || !state.fid)
        return;

      var selection
        = state.select
        ? state.select.split('|').map(function (x) { return parseInt(x); })
        : [1, 1, 1, 1];

      var fileInfo = urlHandler.getFileInfo();
      this.loadFile(fileInfo.id);
      this.set('selection', selection);
      this.jumpToPos(selection[0], selection[1]);

      this.loadBlameView(fileInfo.path, fileInfo.id);
    },

    _subscribeTopics : function () {
      var that = this;
      topic.subscribe('codecompass/gitblame', function (message) {
        var fileInfo = message.fileInfo;

        if (!fileInfo)
          return;

        commitCache = {};
        that.loadFile(fileInfo.id);
        that.loadBlameView(fileInfo.path, fileInfo.id);

        topic.publish('codecompass/setCenterModule', that.id);

        urlHandler.setStateValue({
          center : that.id
        });
      });
    }
  });

  var blameView = new GitBlame({id : 'gitblame'});
  viewHandler.registerModule(blameView, {
    type : viewHandler.moduleType.Center
  });
});
