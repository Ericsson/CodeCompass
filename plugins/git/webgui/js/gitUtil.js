define([
  'codecompass/model',
  'codecompass/util'],
function (model, util) {

  model.addService('gitservice', 'GitService', GitServiceClient);

  return {
    /**
     * This function creates a signature for a git commit author.
     */
    getSignature : function(author) {
      return author.name + ' (' + author.email + ')';
    },

    /**
     * This function creates a html label for a git commit.
     * @param {GitCommit} commit Thrift git commit object.
     * @return HTML Commit label.
     */
    createLabel : function(commit) {
      var avatarLabel = commit.author.name.charAt(0).toUpperCase();
      return '<div class="git-commit">'
        + '<div class="git-avatar" style="background-color:'
        + util.strToColor(commit.author.name) + '">' + avatarLabel + '</div>'
        + '<div class="git-message">' + commit.summary + '</div>'
        + '<div class="git-author">' + this.getSignature(commit.author)
        + '</div></div>';
    },

    /**
     * This functions creates a html tooltip message for a git commit.
     * @param {GitCommit} commit Thrift git commit object.
     * @return HTML Commit tooltip content.
     */
    createTooltip : function(commit) {
      var time = util.timeAgo(new Date(commit.time * 1000));
      return '<div class="git-commit-tooltip">'
        + '<div class="git-sha">#' + commit.oid.substr(0,8) + '</div>'
        + '<div class="git-message">' + commit.message + '</div>'
        + '<div class="commit-meta">'
        +   '<div class="git-author">' + this.getSignature(commit.author)
        +   '</div><div class="git-committed-on"> committed on '
        +     '<span class="git-time">' + time  + '</span></div>'
        + '</div></div>';
    },
  }
});
