import ast


class ParserTreeNode:
    def __init__(self, node, parent: 'ParserTreeNode'):
        self.node = node
        self.parent = parent
        self.children = []
        self.process_children()

    def process_children(self):
        for child in ast.iter_child_nodes(self.node):
            self.children.append(ParserTreeNode(child, self))


class ParserTree:
    def __init__(self, node):
        self.root = ParserTreeNode(node, None)

    def find_node(self, node) -> ParserTreeNode:
        return self.find_node_in_parent(self.root, node)

    def find_node_in_parent(self, parent: ParserTreeNode, node) -> ParserTreeNode:
        if parent.node is node:
            return parent
        for child in parent.children:
            n = self.find_node_in_parent(child, node)
            if n is not None:
                return n
        return None
