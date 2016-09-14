package org.yinwang.pysonar;

import java.util.Map;
import org.yinwang.pysonar.ast.Node;
import org.yinwang.pysonar.common.Extension;

@Extension
public class EParser extends Parser {

    @Override
    public Node convert(Object o) {
        Node node = super.convert(o);
        
        if (!(o instanceof Map)) {
            return null;
        }
        Map<String, Object> map = (Map<String, Object>) o;

        String type = ((String) map.get("type") == null || ((String) map.get("type")).equals(""))
                ? "UNKNOWN" : (String) map.get("type");
        Double startLine = ((Double) map.get("start_lineno") == null)
                ? -1.0 : (Double) map.get("start_lineno");
        Double endLine = ((Double) map.get("end_lineno") == null)
                ? -1.0 : (Double) map.get("end_lineno");
        Double offset = ((Double) map.get("col_offset") == null)
                ? -1.0 : (Double) map.get("col_offset");
        Double end_offset = ((Double) map.get("end_col_offset") == null)
                ? -1.0 : (Double) map.get("end_col_offset");
        
        node.setAstType(type);
        node.setLineStart((int) startLine.doubleValue());
        node.setLineEnd((int) endLine.doubleValue());
        node.setOffset((int) offset.doubleValue());
        node.setEndOffset((int) end_offset.doubleValue());

        return node;
    }
}
