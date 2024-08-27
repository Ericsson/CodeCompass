package cc.search.match.matcher;

import cc.search.match.Context;
import cc.service.search.LineMatch;
import java.io.IOException;
import java.util.ArrayList;
import java.util.List;
import java.util.logging.Level;
import java.util.logging.Logger;

/**
 * A ResultMatcherFactory that wraps the current matcher implementations.
 */
public class MasterMatcherFactory implements ResultMatcherFactory {
  /**
   * Logger.
   */
  private static final Logger _log  = Logger.getGlobal();
  /**
   * Matcher wrapper for matching with multiple matchers.
   */
  private static class MasterMatcher implements ResultMatcher {
    /**
     * The wrapped matchers.
     */
    private final List<ResultMatcher> _matchers;
    /**
     * @param matchers_ matchers
     */
    public MasterMatcher(List<ResultMatcher> matchers_) {
      _matchers = matchers_;
    }

    @Override
    public List<LineMatch> match() {
      ArrayList<LineMatch> result = new ArrayList<>(100);
      
      for (ResultMatcher matcher : _matchers) {
        try {
          List<LineMatch> lines = matcher.match();
          result.addAll(lines);
        } catch (IOException ex) {
          _log.log(Level.SEVERE, "Match failed!", ex);
        }
      }
      
      return result;
    }

    @Override
    public void close() {
      for (ResultMatcher matcher : _matchers) {
        try {
          matcher.close();
        } catch (Exception ex) {
          _log.log(Level.SEVERE, "Closing matcher failed!", ex);
        }
      }
    }
    
  }
  /**
   * Wrapped factories.
   */
  private final ResultMatcherFactory[] _factories;
  /**
   * Just a constructor.
   */
  public MasterMatcherFactory() {
    _factories = new ResultMatcherFactory[] {
      new SourceLineMatcherFactory(),
      new TagKindMatcherFactory(),
      new LogQueryMatcherFactory()
    };
  }
  
  @Override
  public ResultMatcher create(Context context_) {
    List<ResultMatcher> matchers = new ArrayList<>(_factories.length);
    
    for (ResultMatcherFactory fac : _factories) {
      try {
        ResultMatcher matcher = fac.create(context_);
        if (matcher != null) {
          matchers.add(matcher);
        }
      } catch (IOException ex) {
        _log.log(Level.SEVERE, "Creating matcher failed!", ex);
      }
    }
    
    if (matchers.isEmpty()) {
      return null;
    } else {
      return new MasterMatcher(matchers);
    }
  }

  @Override
  public void close() {
    for (ResultMatcherFactory fac : _factories) {
      try {
        fac.close();
      } catch (Exception ex) {
        _log.log(Level.SEVERE, "Closing factory failed!", ex);
      }
    }
  }
}
