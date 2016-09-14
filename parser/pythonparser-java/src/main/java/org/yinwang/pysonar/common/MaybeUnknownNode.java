package org.yinwang.pysonar.common;

import org.yinwang.pysonar.ast.Node;
import org.yinwang.pysonar.db.data.PythonUnknown;

/**
 *
 * @author eadaujv
 */
public class MaybeUnknownNode {
    public Node node;
    public Node target;
    public PythonUnknown.Kind kind;

    public MaybeUnknownNode(Node node, Node target, PythonUnknown.Kind kind) {
        this.node = node;
        this.target = target;
        this.kind = kind;
    }
}
