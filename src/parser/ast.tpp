#ifndef MINIJAVA_INCLUDED_FROM_PARSER_AST_HPP
#error "Never `#include <parser/ast.tpp>` directly; `#include <parser/ast.hpp>` instead."
#endif

namespace minijava
{
    namespace ast
    {
        void visitor::visit_node(node&)
        {
        }

        void visitor::visit_expression(expression& node)
        {
            visit_node(node);
        }

        void visitor::visit_constant(constant& node)
        {
            visit_expression(node);
        }

        void visitor::visit_block_statement(block_statement& node)
        {
            visit_node(node);
        }

        void visitor::visit_statement(statement& node)
        {
            visit_block_statement(node);
        }
        void visitor::visit(type& node)
        {
            visit_node(node);
        }

        void visitor::visit(var_decl& node)
        {
            visit_node(node);
        }

        void visitor::visit(assignment_expression& node)
        {
            visit_expression(node);
        }

        void visitor::visit(binary_expression& node)
        {
            visit_expression(node);
        }

        void visitor::visit(unary_expression& node)
        {
            visit_expression(node);
        }

        void visitor::visit(object_instantiation& node)
        {
            visit_expression(node);
        }

        void visitor::visit(array_instantiation& node)
        {
            visit_expression(node);
        }

        void visitor::visit(array_access& node)
        {
            visit_expression(node);
        }

        void visitor::visit(variable_access& node)
        {
            visit_expression(node);
        }

        void visitor::visit(method_invocation& node)
        {
            visit_expression(node);
        }

        void visitor::visit(this_ref& node)
        {
            visit_expression(node);
        }

        void visitor::visit(boolean_constant& node)
        {
            visit_constant(node);
        }

        void visitor::visit(integer_constant& node)
        {
            visit_constant(node);
        }

        void visitor::visit(null_constant& node)
        {
            visit_constant(node);
        }

        void visitor::visit(local_variable_statement& node)
        {
            visit_block_statement(node);
        }


        void visitor::visit(expression_statement& node)
        {
            visit_statement(node);
        }

        void visitor::visit(block& node)
        {
            visit_statement(node);
        }

        void visitor::visit(if_statement& node)
        {
            visit_statement(node);
        }

        void visitor::visit(while_statement& node)
        {
            visit_statement(node);
        }

        void visitor::visit(return_statement& node)
        {
            visit_statement(node);
        }

        void visitor::visit(empty_statement& node)
        {
            visit_statement(node);
        }

        void visitor::visit(main_method& node)
        {
            visit_node(node);
        }

        void visitor::visit(method& node)
        {
            visit_node(node);
        }

        void visitor::visit(class_declaration& node)
        {
            visit_node(node);
        }

        void visitor::visit(program& node)
        {
            visit_node(node);
        }
    }
}
