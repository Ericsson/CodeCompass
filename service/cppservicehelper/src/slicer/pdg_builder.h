#ifndef SERVICE_CPPSERVICEHELPER_SLICER_PDG_BUILDER_H
#define SERVICE_CPPSERVICEHELPER_SLICER_PDG_BUILDER_H

#include <stack>
#include <map>
#include <set>

#include <clang/Frontend/ASTUnit.h>
#include <clang/AST/RecursiveASTVisitor.h>
#include <clang/AST/Decl.h>
#include <clang/AST/DeclBase.h>
#include <clang/AST/Stmt.h>
#include <clang/AST/Type.h>

#include <iostream>

#include <model/position.h>
#include <model/cxx/cppastnode.h>
#include <cxxparser/internal/filelocutilbase.h>

#include "pdg.h"
#include "multistack.h"
#include "variablevisitor.h"

/* 
 * TODO: handle functoin invocation in expressions
 * TODO: handle break, continue, return --> switch
 */


namespace cc
{ 
namespace service
{  
namespace language
{

/**
 * This class build the PDG by visiting the AST of the function.
 * The control edges and back control edges are computed via traversing the AST, 
 * and the data edges added to the graph immediately after.
 * 
 * The def and use variables also put to the PDG in the first step: while the
 * AST is traversed.
 */
class PDG_builder : public clang::RecursiveASTVisitor<PDG_builder>
{
public:
  typedef std::map<model::HashType, std::set<Node::id_t>> Variables; 
  
  PDG_builder(std::shared_ptr<odb::database> db_, PDG& pdg_, clang::SourceManager* srcMgr_) :
    pdg(pdg_), variableVisitor(db_, *srcMgr_), floc(*srcMgr_)
  {
    pdg.createEntry();
    cs.push(pdg.entry());
    control_s.push(pdg.entry()); 
  }
  
  bool VisitFunctionDecl(clang::FunctionDecl* fun)
  {
    for(clang::FunctionDecl::param_iterator it = fun->param_begin(); it != fun->param_end(); ++it)
    {
      Node::id_t param_node_id = pdg.addNode(Node::Statement, "param_decl", *it, computeRange(*it));
      pdg.linkNode(cs.top(), param_node_id);
      computeDefUse(*it, pdg.node(param_node_id));
    }
    
    return true;
  }
  
  bool TraverseStmt(clang::Stmt* stmt_)
  {
    if(stmt_ && llvm::isa<clang::Expr>(stmt_))
    {
      Node::id_t stmt_node_id = pdg.addNode(Node::Statement, "exp stmt", stmt_, computeRange(stmt_));
      pdg.linkNode(cs.top(), stmt_node_id);
      
      control_s.replace_top(stmt_node_id);
      
      computeDefUse(stmt_, pdg.node(stmt_node_id));
      
      //std::cerr << stmt_node_id << " == " << std::endl;
      //stmt_->dump();
      //std::cerr << " ======= " << std::endl;
      
      return true;
    }
    else
      return clang::RecursiveASTVisitor<PDG_builder>::TraverseStmt(stmt_);
  }
  
  bool TraverseDeclStmt(clang::DeclStmt* declStmt_)
  {
    Node::id_t decl_node_id = pdg.addNode(Node::Statement, "decl stmt", declStmt_, computeRange(declStmt_));
    pdg.linkNode(cs.top(), decl_node_id);
    
    for(clang::DeclStmt::decl_iterator it = declStmt_->decl_begin(); it != declStmt_->decl_end(); ++it)
    {
      if( (*it)->getKind() == clang::Decl::Var || (*it)->getKind() == clang::Decl::ParmVar)
      {
         clang::VarDecl* varDecl = static_cast<clang::VarDecl*>(*it);
         computeDefUse(varDecl, pdg.node(decl_node_id));
      }
    }
    //std::cerr << decl_node_id << " -- " << std::endl;
    //declStmt_->dump();
    //std::cerr << " -------- " << std::endl;
    //TODO: handle variables
    
    control_s.replace_top(decl_node_id);
    
    return true;
  }
  
  bool TraverseIfStmt(clang::IfStmt* if_)
  {
    Node::id_t reg = pdg.addNode(Node::Region, "if_region");
    pdg.linkNode(cs.top(), reg);
    
    Node::id_t pred = pdg.addNode(Node::Predicate, "if_pred", if_->getCond(), computeRange(if_->getCond()));
    pdg.linkNode(reg, pred);
    computeDefUse(if_->getCond(), pdg.node(pred));
    
    Node::id_t regT = pdg.addNode(Node::Region, "if_true_region");
    pdg.linkNode(pred, regT, "true");
    
    Node::id_t regF = pdg.addNode(Node::Region, "if_false_region");
    pdg.linkNode(pred, regF, "false");
    

    control_s.replace_top(regT);
    
    if(if_->getThen())
    {      
      cs.push(regT);                
      clang::RecursiveASTVisitor<PDG_builder>::TraverseStmt(if_->getThen());
      cs.pop();
    }
    
    control_s.push(regF);
    
    if(if_->getElse())
    {      
      cs.push(regF);                
      clang::RecursiveASTVisitor<PDG_builder>::TraverseStmt(if_->getElse());
      cs.pop();
    }
    
    control_s.join_top2();
    
    return true;
  }
  
  bool TraverseForStmt(clang::ForStmt* for_)
  {
    if(for_->getInit())      
    {
      Node::id_t init_node_id = pdg.addNode(Node::Statement, "for_init", 
                                            for_->getInit(), computeRange(for_->getInit()));
      pdg.linkNode(cs.top(), init_node_id);
      computeDefUse(for_->getInit(), pdg.node(init_node_id));
      
    }
    
    Node::id_t reg = pdg.addNode(Node::Region, "for_region");
    pdg.linkNode(cs.top(), reg);
    
    //TODO: if there is no condition put a true contstant there (need to deal with on data depencency
    Node::id_t pred = pdg.addNode(Node::Predicate, "for_pred", for_->getCond(), computeRange(for_->getCond()));
    pdg.linkNode(reg, pred);
    computeDefUse(for_->getCond(), pdg.node(pred));
    
    Node::id_t reg2 = pdg.addNode(Node::Region, "for_region");
    pdg.linkNode(pred, reg2);
    
    control_s.push(reg2);
    
    cs.push(reg2);   
    if(for_->getBody())
      clang::RecursiveASTVisitor<PDG_builder>::TraverseStmt(for_->getBody());
    
    if(for_->getInc())
    {
      Node::id_t inc_node_id = pdg.addNode(Node::Statement, "for_inc", for_->getInc(), computeRange(for_->getInc()));                                 
      pdg.linkNode(cs.top(), inc_node_id);   
      computeDefUse(for_->getInc(), pdg.node(inc_node_id));
      control_s.replace_top(inc_node_id);
    }
    
    cs.pop();
    
    add_back_control_edges(control_s.top(), reg);
    control_s.pop();
    
    return true;
  }
  
  bool TraverseDoStmt(clang::DoStmt* do_)
  {
    Node::id_t do_node_region = pdg.addNode(Node::Region, "do_while_region");
    pdg.linkNode(cs.top(), do_node_region);
    
    cs.push(do_node_region);
    
    if(do_->getBody())
      clang::RecursiveASTVisitor<PDG_builder>::TraverseStmt(do_->getBody());
    
    cs.pop();
    
    Node::id_t do_node_id = pdg.addNode(Node::Predicate, "do_while_pred");
    pdg.linkNode(do_node_region, do_node_id);
    //TODO: do we need to deal with prediace in this case?
    //there is no control depencency on it
    
    pdg.linkBackControlNode(do_node_id, do_node_region);
    return true;
  }
  
  bool TraverseWhileStmt(clang::WhileStmt* while_)
  {
    Node::id_t reg = pdg.addNode(Node::Region, "while_region");
    pdg.linkNode(cs.top(), reg);
    
    Node::id_t pred = pdg.addNode(Node::Predicate, "while_pred" ,while_->getCond(), computeRange(while_->getCond()));
    pdg.linkNode(reg, pred);
    computeDefUse(while_->getCond(), pdg.node(pred));
    
    Node::id_t regT = pdg.addNode(Node::Region,"while_region");
    pdg.linkNode(pred, regT, "true");
    
    control_s.push(regT);
    
    if(while_->getBody())
    {
      cs.push(regT);
      clang::RecursiveASTVisitor<PDG_builder>::TraverseStmt(while_->getBody());
      cs.pop();
    }
    
    add_back_control_edges(control_s.top(), reg);
    control_s.pop();
    
    return true;
  }  
  
  void add_data_edges()
  {    
    Node::id_t workingNode = pdg.last_node();
    std::cerr << "start to add data edges with node: " << workingNode << std::endl;
    Variables vars;
    add_uses(vars, workingNode);
        
    std::vector<Node::id_t> ps = pdg.priorSiblings(workingNode);  
    std::cerr << "sibling ( " << ps.size() << "): " << std::endl;
    for(auto x : ps)
      std::cerr << x << " ";
    std::cerr << std::endl;
    
    add_data_edges_aux(ps, vars);
  }
  
  void add_uses(Variables& vars, Node::id_t workingNode)
  {
    for( model::HashType var : pdg.node(workingNode).uses)
      vars[var].insert(workingNode);
  }
  
  void join_variables(Variables& target, Variables& true_vars, Variables& false_vars)
  {
    for( auto& p : false_vars)
    {
      true_vars[p.first].insert(p.second.begin(), p.second.end());
    }
    target = true_vars;
  }
  
  void add_data_edges_aux(const std::vector<Node::id_t>& siblings, Variables& vars)
  {        
    for(int i=siblings.size()-1; i>=0; --i)
    {
      //std::cerr << "visit node_id " << i << std::endl;
      //visiting node
      const Node& n = pdg.node(siblings[i]);
      for(model::HashType var : n.defs)
      {
        for(Node::id_t uses_node : vars[var])
          pdg.linkDataNode(n, uses_node);
        vars[var].clear();                           
      }      
      
      if(n.label == "if_pred")
      {
        const std::vector<Node::id_t>& children = pdg.children(n.id);
        std::vector<Node::id_t> true_child {children[0]};
        std::vector<Node::id_t> false_child {children[1]};
        
        Variables true_vars = vars;
        Variables false_vars = vars;
        
        add_data_edges_aux(true_child, true_vars);
        add_data_edges_aux(false_child, false_vars);
        
        join_variables(vars, true_vars, false_vars);        
        
        //add_data_edges_to_predicate_variables(n);
      }
      else if(n.label == "while_pred" || n.label == "for_pred")
      {
        const std::vector<Node::id_t>& children = pdg.children(n.id);
        std::vector<Node::id_t> block {children[0]};
        
        Variables true_vars = vars;
        Variables false_vars = vars;
        
        add_data_edges_aux(block, true_vars);
        
        join_variables(vars, true_vars, false_vars);        

        add_data_edges_to_predicate_variables(n);
      }
      else
      {                    
        const std::vector<Node::id_t>& children = pdg.children(n.id);
        if(!children.empty())
        {
          add_data_edges_aux(children, vars);
        }      
      }
      
      add_uses(vars, n.id);
    }
  }
  
  void add_data_edges_to_predicate_variables(Node::id_t from)
  {
    Variables vars_in_predicate;
    const Node& fromNode = pdg.node(from);
    for(model::HashType var : fromNode.uses)
    {
      vars_in_predicate[var];
    }
    
    add_data_edges_to_predicate_variables_aux(from, vars_in_predicate);

    for( const std::pair<model::HashType, std::set<Node::id_t>>& p : vars_in_predicate)
    {
      for( Node::id_t defined : p.second )
         pdg.linkDataNode(defined, from);
    }
    
    
  }
  
  void add_data_edges_to_predicate_variables_aux(Node::id_t from, Variables& vars_in_pred)
  {
    const std::vector<Node::id_t>& children = pdg.children(from);
    
    for(int i=children.size()-1; i>=0; --i)
    {
      const Node& child = pdg.node(children[i]);
      
      for( auto def : child.defs )
      {
        auto it = vars_in_pred.find(def);
        if( it != vars_in_pred.end() )
        {
          it->second = std::set<Node::id_t>{child.id};
        }
      }
      
      if(child.label == "if_pred")
      {
        const std::vector<Node::id_t>& children = pdg.children(child);
        const Node& trueReg = pdg.node(children[0]);
        const Node& falseReg = pdg.node(children[1]);
        
        Variables true_vars = vars_in_pred;
        Variables false_vars = vars_in_pred;
        
        add_data_edges_to_predicate_variables_aux(trueReg, true_vars);
        add_data_edges_to_predicate_variables_aux(falseReg, false_vars);
        
        join_variables(vars_in_pred, true_vars, false_vars);                
      }
      else
      {
        add_data_edges_to_predicate_variables_aux(child, vars_in_pred);
      }
      
    }
  }
  
private:  
  void add_back_control_edges(const std::vector<Node::id_t>& from, Node::id_t to)
  {
    for(Node::id_t f : from)
      pdg.linkBackControlNode(f, to);
  }

  void computeDefUse(clang::VarDecl* varDecl, Node& workingNode)
  {
    variableVisitor.setWorkingPDGNode(&workingNode);
    variableVisitor.TraverseDecl(varDecl);
  }
  
  void computeDefUse(clang::Stmt* stmt, Node& workingNode)
  {
    variableVisitor.setWorkingPDGNode(&workingNode);
    variableVisitor.TraverseStmt(stmt);
  }
  
  template<typename ClangAstNode>
  core::Range computeRange(ClangAstNode* astNode)
  {
    core::Range range;
    model::Range mrange;
    
    floc.setPosition(astNode->getLocStart(), astNode->getLocEnd(), mrange);
    
    range.startpos.line = mrange.start.line;
    range.startpos.column = mrange.start.column;
    
    range.endpos.line = mrange.end.line;
    range.endpos.column = mrange.end.column;
    
    return range;
  }
  
  PDG& pdg;
  std::stack<Node::id_t> cs;
  multistack<Node::id_t> control_s;  
  VariableVisitor variableVisitor;
  parser::FileLocUtilBase floc;
};

} // language
} // service
} // cc



#endif