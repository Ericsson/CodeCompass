define([
  'dojo/dom-construct',
  'dojo/mouse',
  'dojo/on',
  'dojo/topic',
  'dojox/html/entities',
  'codecompass/registration/model',
  'dijit/Tooltip'],
function (dom, mouse, on, topic, entities, model, Tooltip) {
  
  
  function shortId(id)
  {
    if (id.length > 9) {
      return id.substr(0,9) + "…";
    } else {
      return id;
    }
  }  

  
  return {
    
    /**
     * Shows a tooltip with commit information
     * 
     * Note: either
     *  - repoId AND commitId 
     *  - commitDetails
     * must be set. If commitDetails is null, it is fetched using a
     * versionservice.getCommit call that needs the repoId and commitId
     * 
     * @param repoId the id of the repo (may be null if commitDetails is set)
     * @param commitId the id of the commit (may be null if commitDetails is set)
     * @param commitDetails a VersionCommit object (may be null, see above)
     * @param tooltipOverDomNode a domnode for tooltip placement
     */
    registerCommitTooltip : function (repoId, commitId, commitDetails, tooltipOverDomNode) {

      on(tooltipOverDomNode, 'mouseenter', function (evt) {
        
        if (null == commitDetails) {
          commitDetails = model.versionservice.getCommit(repoId, commitId);
        }
        
        var tipmsg
            = entities.encode(commitDetails.message).replace(/\n/g, '<br/>')
            + '<br/>'
            + '<b>Author: </b>' + entities.encode(commitDetails.author)
            + '<br/>'
            + ( commitDetails.author !== commitDetails.committer
              ? '<b>Committer: </b>' + entities.encode(commitDetails.committer)
              + '<br/>'
              : '')
            + '<b>Id: </b>' + entities.encode(commitDetails.oid);      
      
        Tooltip.show(tipmsg, tooltipOverDomNode, ['above']);
        on.once(tooltipOverDomNode, mouse.leave, function(){
          Tooltip.hide(tooltipOverDomNode);
        })
      });
      
      
    },
    
    /**
     * Formats an commit id with according stylesheet
     */
    formatId : function (oid) {
      return '<span class="versioncommon-commitid">' + shortId(oid) + '</span>';
    },
    
    /**
     * Returns a function that can be assigned as an onclick event handler
     * for an object that jumps to the commit on click
     */
    onCommitClick : function (versionrepoid, oid, CommitDom)
    {
      return function (evt) {
          Tooltip.hide(CommitDom);
          topic.publish('codecompass/versionOpenCommit', {
            moduleId        : 'version',
            newTab          : mouse.isMiddle(evt),
            versionrepoid   : versionrepoid,
            versioncommitid : oid,
            resetopentab    : true
          });
      };
    },
  
    placeCommitLink : function (commitDetails, parentDom) {
      var CommitDom = dom.create("span", { innerHTML: this.formatId(commitDetails.oid) }, parentDom);
      this.registerCommitTooltip(null, null, commitDetails, CommitDom);
      on(CommitDom, 'click', this.onCommitClick(commitDetails.repoId, commitDetails.oid, CommitDom));
    }

  };
});
