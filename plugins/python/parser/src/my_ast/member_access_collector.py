import ast
from typing import Optional, List, Any, Union


class MemberAccessCollector(ast.NodeVisitor):
    class MemberData:
        def __init__(self, name: Optional[str]):
            self.name: str = name

    class ConstantData(MemberData):
        pass

    class AttributeData(MemberData):
        pass

    class MethodData(MemberData):
        def __init__(self, name: Optional[str], args):
            super().__init__(name)
            self.arguments = args

    class ReturnValueData(MemberData):
        def __init__(self, name: Optional[str], member_data, node: ast.AST):
            super().__init__(name)
            self.return_of: MemberAccessCollector.MemberData = member_data
            self.node = node

    class SubscriptData(MemberData):
        class Index:
            def __init__(self, idx):
                self.idx = idx

        class Slice:
            def __init__(self, lower, upper, step):
                self.lower = lower
                self.upper = upper
                self.step = step

        def __init__(self, name: Optional[str], sub_slice: List[Union[Index, Slice]]):
            super().__init__(name)
            self.slice: List[Union[MemberAccessCollector.SubscriptData.Index,
                                   MemberAccessCollector.SubscriptData.Slice]] = sub_slice

    class OperatorData(MemberData):
        def __init__(self, name: Optional[str], node: Union[ast.BoolOp, ast.BinOp, ast.UnaryOp, ast.Compare]):
            super().__init__(name)

    class LambdaData(MemberData):
        def __init__(self, name: Optional[str], node: ast.Lambda):
            super().__init__(name)

    def __init__(self, member: Union[ast.Call, ast.Attribute]):
        self.call_list: List[MemberAccessCollector.MemberData] = []
        self.arguments = []
        self.last = False
        self.visit(member)

    def generic_visit(self, node: ast.AST):
        if self.last:
            return
        if not isinstance(node, ast.Await):     # await must be skipped, and process the callable
            print('Unhandled node type: ' + str(type(node)))
        ast.NodeVisitor.generic_visit(self, node)

    def visit_Call(self, node: ast.Call) -> Any:
        if self.last:
            return

        def process_call(value: ast.AST):
            if isinstance(value, (ast.Name, ast.Attribute, ast.Lambda)):
                self.call_list.append(self.MethodData(None, self.collect_arguments(node)))
            elif isinstance(value, (ast.Call, ast.Subscript)) or self.is_callable_after_override(value):
                self.call_list.append(
                    self.ReturnValueData(None, self.MethodData(None, self.collect_arguments(node)), value))
            elif isinstance(value, ast.Await):
                process_call(value.value)
            else:
                assert False
        process_call(node.func)
        ast.NodeVisitor.generic_visit(self, node)

    def visit_Subscript(self, node: ast.Subscript) -> Any:
        if self.last:
            return

        def process_subscript(value: ast.AST):
            if isinstance(value, (ast.Name, ast.Attribute)):
                self.call_list.append(self.SubscriptData(None, self.collect_slice(node)))
            elif isinstance(value, (ast.Call, ast.Subscript)) or self.is_callable_after_override(value):
                self.call_list.append(
                    self.ReturnValueData(None, self.SubscriptData(None, self.collect_slice(node)), value))
            elif isinstance(value, ast.Await):
                process_subscript(value.value)
            else:
                assert False, "Not subscriptable type: " + str(type(value))
        process_subscript(node.value)
        ast.NodeVisitor.generic_visit(self, node)

    def visit_Attribute(self, node: ast.Attribute) -> Any:
        if self.last:
            return
        if len(self.call_list) > 0 and self.call_list[-1].name is None:
            self.call_list[-1].name = node.attr
        else:
            self.call_list.append(self.AttributeData(node.attr))
        ast.NodeVisitor.generic_visit(self, node)

    def visit_Name(self, node: ast.Name) -> Any:
        if self.last:
            return
        if self.call_list[-1].name is None:
            self.call_list[-1].name = node.id
        else:
            self.call_list.append(self.AttributeData(node.id))
        self.last = True

    def visit_Constant(self, node: ast.Constant) -> Any:
        if self.last:
            return
        if self.call_list[-1].name is None:
            self.call_list[-1].name = node.value
        else:
            value = node.value
            if isinstance(value, str):
                value = '"' + value + '"'
            self.call_list.append(self.ConstantData(value))
        self.last = True

    def visit_Lambda(self, node: ast.Lambda) -> Any:
        if self.last:
            return
        if self.call_list[-1].name is None:
            self.call_list[-1].name = 'lambda'
        else:
            self.call_list.append(self.LambdaData('lambda', node))
        self.last = True

    def visit_BinOp(self, node: ast.BinOp) -> Any:
        self.last = True

    def visit_UnaryOp(self, node: ast.UnaryOp) -> Any:
        self.last = True

    def visit_BoolOp(self, node: ast.BoolOp) -> Any:
        self.last = True

    def visit_Compare(self, node: ast.Compare) -> Any:
        self.last = True

    # TODO: is it necessary?
    @staticmethod
    def collect_arguments(call: ast.Call) -> List:
        arguments = []
        for arg in call.args:
            arguments.append(arg)
        return arguments

    @staticmethod
    def collect_slice(subscript: ast.Subscript) -> List[Union[SubscriptData.Index, SubscriptData.Slice]]:
        # TODO: in python 3.9 this must be changed (ast.Index, ast.ExtSlice are deprecated)
        sub_slice = []

        def process_slice(node: (ast.Index, ast.Slice, ast.ExtSlice)):
            if isinstance(node, ast.Index):
                sub_slice.append(MemberAccessCollector.SubscriptData.Index(node))
            elif isinstance(node, ast.Slice):
                sub_slice.append(MemberAccessCollector.SubscriptData.Slice(node.lower, node.upper, node.step))
            elif isinstance(node, ast.ExtSlice):  # 3.9 -> ast.Tuple
                for dim in node.dims:
                    process_slice(dim)
            else:
                assert False, "Unknown slice: " + str(type(node))

        process_slice(subscript.slice)
        return sub_slice

    @staticmethod
    def is_callable_after_override(node: ast.AST) -> bool:
        return isinstance(node, ast.BoolOp) or isinstance(node, ast.BinOp) or isinstance(node, ast.UnaryOp) or \
               isinstance(node, ast.Compare)


class MemberAccessCollectorIterator:
    def __init__(self, member_access_collector: MemberAccessCollector):
        self.mac = member_access_collector
        self.index = len(self.mac.call_list)

    def __iter__(self):
        self.index = len(self.mac.call_list)
        return self

    def __next__(self):
        if self.index <= 0:
            raise StopIteration
        self.index -= 1
        return self.mac.call_list[self.index]

    def get_current(self):
        return self.mac.call_list[self.index]

    def get_first(self):
        return self.mac.call_list[-1]

    def get_last(self):
        return self.mac.call_list[0]

    def is_iteration_over(self) -> bool:
        return self.index <= 0

    def is_iteration_started(self) -> bool:
        return self.index < len(self.mac.call_list)
