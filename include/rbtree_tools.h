/* Copyright 2019 Julien Hartmann
 */
#ifndef DATASTRUCTURES_RBTREE_TOOLS_H
#define DATASTRUCTURES_RBTREE_TOOLS_H

#include <ostream>
#include <queue>
#include <string>

namespace datastructure::tools {

    template <typename K, typename T, typename Compare, typename Allocator>
    void writeDot(std::ostream & out, const RBTree<K, T, Compare, Allocator> & tree,
                  const std::string & name)
    {
        using NodeBase = detail::NodeBase;
        using Tree = RBTree<K, T, Compare, Allocator>;
        using Node = detail::Node<typename Tree::key_type, typename Tree::mapped_type>;

        std::queue<NodeBase *> todo;
        todo.push(detail::root(tree));

        out <<"digraph \"" <<name <<"\" {\n";

        while (!todo.empty()) {
            auto node = static_cast<Node *>(todo.front());
            out <<"    \"" <<node
                <<"\" [color=" <<(node->color == detail::Color::Red ? "red" : "black")
                <<" label=<"
                <<node->value.first
                <<"<BR/><FONT POINT-SIZE=\"10\">"
                <<node->value.second
                <<"</FONT>>];\n";
            if (node->left != detail::nil) {
                out <<"    \"" <<node
                    <<"\" -> \"" <<static_cast<Node *>(node->left) <<"\";\n";
            }
            if (node->right != detail::nil) {
                out <<"    \"" <<node
                    <<"\" -> \"" <<static_cast<Node *>(node->right) <<"\";\n";
            }
            if (node->left != datastructure::detail::nil) { todo.push(node->left); }
            if (node->right != datastructure::detail::nil) { todo.push(node->right); }
            todo.pop();
        }
        out <<"}\n";
    }
}

#endif
