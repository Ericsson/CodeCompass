define([
  'dojo/dom-construct',
  'dojo/mouse',
  'dojo/on',
  'dojo/topic',
  'dojo/_base/Color',
  'dojox/html/entities',
  'dijit/Tooltip',
  'codecompass/registration/model',
  'codecompass/registration/versionCommon'],
function (dom, mouse, on, topic, Color, entities, Tooltip, model, versionCommon) {
  
  /**
   * This function concatenates str string n times.
   * @param {Number} n This many times will str be replicated.
   * @param {String} str This string will be replicated. If not given, then ' '
   * is replicated n times.
   */
  function replicate(n, str) {
    return Array(n + 1).join(str || " ");
  }
  
  /**
   * This function creates a blame info object for a given blame hunk. This info
   * object describes the visual representation of the revision control commit
   * in the blame view at the beginning of the line. The output text will
   * contain the user name, a space and a date in 'YYYY-MM-DD hh:mm' format. The
   * date is always visible even if the given width is less than its length.
   * @param {VersionBlameHunk} blame Thrift object.
   * @param {Number} nameWidth The width (number of characters) of user name
   * column, i.e. the length of the longest name.
   * @param {Number} minTime This is the smallest time value. This is used to
   * calculate the commit color which will be gradient between green and red.
   * @param {Number} maxTime This is the greatest time value.
   */
  function createBlameForLine(blame, nameWidth, minTime, maxTime) {
    var commitLen = 16; // 'YYYY-MM-DD hh:mm'
    var commitTimeUNIX = blame.final_signature.time;
    var nameText, commitText, commitColor, commitId;
    
    if (0 === commitTimeUNIX) {
      //----- name of the committer -----
      nameText = ("Not Committed Yet");

      //----- time -----
      commitText = replicate(nameWidth - 1, '&nbsp;');
      
      //----- color -----
      commitColor = new Color([216, 216, 216]);

      //----- commitid -----
      commitId = "";
    } else {
      //----- name of the committer -----
      var name = blame.final_signature.name;
      nameText = name.length > nameWidth
               ? name.slice(0, nameWidth - 3) + "..."
               : name + replicate(nameWidth - name.length, '&nbsp;');
      
      //----- time -----
      commitText = new Date(commitTimeUNIX * 1000)
                  .toISOString()
                  .slice(0, commitLen)
                  .replace("T", " ");
      
      //----- color -----
      // Get in [0, 1] range
      var commitHeat = (commitTimeUNIX - minTime) / (maxTime - minTime);
      // Convert to rgb. This conversion algorithm is pretty lame.
      commitColor = new Color([
        Math.round(128 + (1 - commitHeat) * 127),
        Math.round(128 +      commitHeat  * 127),
        128]);
      
      //----- commitid -----
      commitId = blame.final_commit_id;
    }
    
    return {
      text     : nameText + " " + commitText,
      commitId : commitId,
      color    : commitColor.toCss()
    };
  }
  
  /**
   * This function queries the blame information from the server and returns
   * and array which contains these information. The array has lineNo+1
   * elements so that we can index it from 1. This means that element 0 is a
   * dummy element.
   * @return The function returns an array of objects which have the following
   * properties: text, commitId, color.
   */
  function caclulateAnnotations(
    versionrepoid, versioncommitid, versionpath, maybeModifiedFileId) {
    
    var blameInfo = model.versionservice.getBlameInfo(
      versionrepoid, versioncommitid, versionpath, maybeModifiedFileId);
    
    var blameForLines = [""];

    var nameMaxLen = 0;
    var minTime    = Infinity;
    var maxTime    = -1;
    
    blameInfo.forEach(function (blame) {
      // Calculate max name length to allow narrower annotations if noone has
      // a long name
      var currLen = blame.final_signature.name.length;
      if (currLen > nameMaxLen) {
        nameMaxLen = currLen;
      }

      // Calculate trac-like coloring based on commit date
      var currTime = blame.final_signature.time;
      if (0 === currTime) return;
      if (currTime > maxTime) maxTime = currTime;
      if (currTime < minTime) minTime = currTime;
    });
    
    if (minTime === maxTime) {
      // This makes one-commit files green
      --minTime;
    }

    blameInfo.forEach(function (blame) {
      var blameForLine
        = createBlameForLine(blame, nameMaxLen, minTime, maxTime);
      
      for (var i = 0; i < blame.lines_in_hunk; ++i)
        blameForLines.push(blameForLine);
    });
    
    return blameForLines;
  }
  
  return {
    
    /**
     * This function returns a line number formatter which is based on the
     * repository. The line numbers are added blame information.
     */
    getBlameFormatter : function (
      versionrepoid, versioncommitid, versionpath, maybeModified) {
      
      var currentAnnotateInfo = caclulateAnnotations(
        versionrepoid, versioncommitid, versionpath, maybeModified);
      
      return function (i) {
        if (i >= currentAnnotateInfo.length) // TODO: What's this?
          i = currentAnnotateInfo.length - 1;
        
        var annotateInfo = currentAnnotateInfo[i];
        
        var lineNumberDomNode = dom.create('span', {
          innerHTML : i + ' ',
          style     : 'display: inline-block'
        });
        
        var annotationDomNode = dom.create('span', {
          innerHTML : annotateInfo.text,
          style     : 'color: #333; display: inline-block;'
                    + (annotateInfo.commitId ? 'cursor: pointer;' : '')
                    + (annotateInfo.color
                        ? 'background-color: ' + annotateInfo.color + ';'
                        : '')
        }, lineNumberDomNode);
        
        versionCommon.registerCommitTooltip(
          versionrepoid, annotateInfo.commitId, null, annotationDomNode);
        
        on(annotationDomNode, 'click', function (evt) {
          Tooltip.hide(annotationDomNode);
          topic.publish('codecompass/versionOpenCommit', {
            moduleId        : 'version',
            newTab          : mouse.isMiddle(evt),
            versionrepoid   : versionrepoid,
            versioncommitid : annotateInfo.commitId,
            resetopentab    : true
          });
        });
        
        return [lineNumberDomNode];
      };
    }

  };
});
