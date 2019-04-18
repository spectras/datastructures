/* Copyright 2019 Julien Hartmann
 */
#ifndef DATASTRUCTURES_RBTREE_H
#define DATASTRUCTURES_RBTREE_H

#include <cassert>
#include <functional>
#include <iterator>
#include <limits>
#include <memory>


namespace datastructure {

/****************************************************************************/

namespace detail {

    //************************************************************************
    // Type-erased node manipulation

    enum class Color : uint8_t { Red, Black };

    struct NodeBase
    {
        NodeBase *  parent = nullptr;
        NodeBase *  left = nullptr;
        NodeBase *  right = nullptr;
        Color       color = Color::Red;
    };

    struct TreeBase
    {
        NodeBase * nil = nullptr;     // owned
        NodeBase * root = nullptr;    // owned, except if nil
    };

    inline bool isLeftChild(const NodeBase * node) { return node == node->parent->left; }
    inline bool isRightChild(const NodeBase * node) { return node == node->parent->right; }
    inline NodeBase * & linkTo(TreeBase & tree, NodeBase * node)
    {
        if (isLeftChild(node)) { return node->parent->left; }
        if (isRightChild(node)) { return node->parent->right; }
        return tree.root;
    }

    inline const NodeBase * allLeft(const NodeBase * node)
    {
        while (node->left != node->left->left) { node = node->left; }
        return node;
    }
    inline NodeBase * allLeft(NodeBase * node)
    {
        return const_cast<NodeBase *>(allLeft(const_cast<const NodeBase *>(node)));
    }

    inline const NodeBase * allRight(const NodeBase * node)
    {
        while (node->right != node->right->right) { node = node->right; }
        return node;
    }
    inline NodeBase * allRight(NodeBase * node)
    {
        return const_cast<NodeBase *>(allRight(const_cast<const NodeBase *>(node)));
    }

    /// In-order backwards traversal - return predecessor, or nullptr if none
    inline const NodeBase * predecessor(const TreeBase & tree, const NodeBase * node)
    {
        if (node == tree.nil) { return allRight(tree.root); }
        if (node->left != tree.nil) { return allRight(node->left); }
        while (node != tree.nil) {
            const auto * parent = node->parent;
            if (node == parent->right) { return parent; }
            node = parent;
        }
        return nullptr;
    }
    inline NodeBase * predecessor(const TreeBase & tree, NodeBase * node)
    {
        return const_cast<NodeBase *>(predecessor(tree, const_cast<const NodeBase *>(node)));
    }

    /// In-order forwards traversal - return successor, or nullptr if none
    inline const NodeBase * successor(const TreeBase & tree, const NodeBase * node)
    {
        if (node == tree.nil) { return allLeft(tree.root); }
        if (node->right != tree.nil) { return allLeft(node->right); }
        while (node != tree.nil) {
            const auto * parent = node->parent;
            if (node == parent->left) { return parent; }
            node = parent;
        }
        return nullptr;
    }
    inline NodeBase * successor(const TreeBase & tree, NodeBase * node)
    {
        return const_cast<NodeBase *>(successor(tree, const_cast<const NodeBase *>(node)));
    }

    inline void leftRotate(TreeBase & tree, NodeBase * x)
    {
        auto y = x->right;
        x->right = y->left;
        x->right->parent = x;
        y->left = x;
        y->parent = x->parent;
        linkTo(tree, x) = y;
        x->parent = y;
    }

    inline void rightRotate(TreeBase & tree, NodeBase * y)
    {
        auto x = y->left;
        y->left = x->right;
        y->left->parent = y;
        x->right = y;
        x->parent = y->parent;
        linkTo(tree, y) = x;
        y->parent = x;
    }

    inline void insertFixup(TreeBase & tree, NodeBase * node)
    {
        NodeBase * parent;
        while(parent = node->parent, parent->color == Color::Red) {
            auto uncle = isLeftChild(parent) ? parent->parent->right : parent->parent->left;

            if (uncle->color == Color::Red) {
                parent->color = Color::Black;
                uncle->color = Color::Black;
                parent->parent->color = Color::Red;
                node = parent->parent;

            } else {
                if (isLeftChild(parent)) {
                    if (isRightChild(node)) {
                        node = parent;
                        leftRotate(tree, node);
                        parent = node->parent;
                    }
                    parent->color = Color::Black;
                    parent->parent->color = Color::Red;
                    rightRotate(tree, parent->parent);
                } else {
                    if (isLeftChild(node)) {
                        node = parent;
                        rightRotate(tree, node);
                        parent = node->parent;
                    }
                    parent->color = Color::Black;
                    parent->parent->color = Color::Red;
                    leftRotate(tree, parent->parent);
                }
                // break; implied by loop condition
            }
        }
        tree.root->color = Color::Black;
    }

    inline void extractFixup(TreeBase & tree, NodeBase * node)
    {
        NodeBase * parent;
        while (parent = node->parent, node != tree.root && node->color == Color::Black)
        {
            if (isLeftChild(node)) {
                auto * sibling = parent->right;
                if (sibling->color == Color::Red) {
                    sibling->color = Color::Black;
                    parent->color = Color::Red;
                    leftRotate(tree, parent);
                    sibling = parent->right;
                }
                if (sibling->left->color == Color::Black &&
                    sibling->right->color == Color::Black) {
                    sibling->color = Color::Red;
                    node = parent;
                } else {
                    if (sibling->right->color == Color::Black) {
                        sibling->left->color = Color::Black;
                        sibling->color = Color::Red;
                        rightRotate(tree, sibling);
                        sibling = parent->right;
                    }
                    sibling->color = parent->color;
                    parent->color = Color::Black;
                    sibling->right->color = Color::Black;
                    leftRotate(tree, parent);
                    node = tree.root;
                }
            } else {
                auto * sibling = parent->left;
                if (sibling->color == Color::Red) {
                    sibling->color = Color::Black;
                    parent->color = Color::Red;
                    rightRotate(tree, parent);
                    sibling = parent->left;
                }
                if (sibling->right->color == Color::Black &&
                    sibling->left->color == Color::Black) {
                    sibling->color = Color::Red;
                    node = parent;
                } else {
                    if (sibling->left->color == Color::Black) {
                        sibling->right->color = Color::Black;
                        sibling->color = Color::Red;
                        leftRotate(tree, sibling);
                        sibling = parent->left;
                    }
                    sibling->color = parent->color;
                    parent->color = Color::Black;
                    sibling->left->color = Color::Black;
                    rightRotate(tree, parent);
                    node = tree.root;
                }
            }
        }
        node->color = Color::Black;
    }

    inline void extractNode(TreeBase & tree, NodeBase * node)
    {
        detail::NodeBase * replacement = tree.nil;
        detail::NodeBase * fixupRoot = nullptr;

        if (node->left != tree.nil && node->right != tree.nil) {
            replacement = allLeft(node->right);

            if (replacement == node->right) { // replacement is immediate right child
                node->right = replacement->right;
                node->right->parent = node;
            } else {                        // replacement is deep
                replacement->parent->left = replacement->right;
                replacement->parent->left->parent = replacement->parent;
            }
            fixupRoot = replacement->color == Color::Black ? replacement->right : nullptr;
            replacement->left = node->left;
            replacement->right = node->right;
            replacement->left->parent = replacement->right->parent = replacement;

        } else if (node->left != tree.nil) {        // no right child
            replacement = fixupRoot = node->left;
            fixupRoot = node->color == Color::Black ? replacement : nullptr;
        } else if (node->right != tree.nil) {       // no left child
            replacement = node->right;
            fixupRoot = node->color == Color::Black ? replacement : nullptr;
        } else {
            fixupRoot = node->color == Color::Black ? replacement : nullptr;
        }

        linkTo(tree, node) = replacement;
        replacement->parent = node->parent;
        replacement->color = node->color;

        if (fixupRoot) { extractFixup(tree, fixupRoot); }
    }

    //************************************************************************
    // Full node with data

    template <typename K, typename T>
    struct Node final : public NodeBase
    {
        std::pair<const K, T>   value;

        Node(const K & k) : value(k, T{}) {}
        Node(K && k) : value(std::move(k), T{}) {}
        Node(const std::pair<const K, T> & v) : value(v) {}
        Node(std::pair<const K, T> && v) : value(std::move(v)) {}
    };

    /// Find node of a given key, looking from a given root, returning possible insertion point if not found
    template <typename K, typename T, typename Compare>
    const Node<K, T> * findNode(const TreeBase & tree, const K & key, const Compare & cmp)
    {
        using N = const Node<K, T>;
        const NodeBase * node = tree.root;
        const NodeBase * next = node;
        while (next != tree.nil) {
            node = next;
            if (cmp(key, static_cast<N *>(node)->value.first)) {
                next = node->left;
            } else if (cmp(static_cast<N *>(node)->value.first, key)) {
                next = node->right;
            } else {
                next = tree.nil;
            }
        }
        return static_cast<N *>(node);
    }
    template <typename K, typename T, typename Compare>
    Node<K, T> * findNode(TreeBase & tree, const K & key, const Compare & cmp)
    {
        using N = Node<K, T>;
        return const_cast<N *>(findNode<K, T, Compare>(const_cast<const TreeBase &>(tree), key, cmp));
    }

    //************************************************************************
    // Red-black tree data, in its own type for EBO

    template <typename K, typename T, typename Compare, typename NodeAllocator>
    struct RBTreeData final : public Compare, public NodeAllocator, public TreeBase
    {
        RBTreeData() = default;

        explicit RBTreeData(const Compare & cmp) : Compare(cmp) {}

        template <typename AllocatorU>
        explicit RBTreeData(AllocatorU && alloc) : NodeAllocator(std::forward<AllocatorU>(alloc)) {}

        template <typename AllocatorU>
        RBTreeData(const Compare & cmp, AllocatorU && alloc)
         : Compare(cmp), NodeAllocator(std::forward<AllocatorU>(alloc)) {}

        RBTreeData & operator=(const RBTreeData &) = delete;
        RBTreeData & operator=(RBTreeData &&) = delete;

        NodeBase * createNil()
        {
            auto * storage = std::allocator_traits<NodeAllocator>::allocate(this->allocator(), 1);
            nil = new (storage) NodeBase;
            nil->left = nil->right = nil;
            nil->color = Color::Black;
            return nil;
        }

        void destroyNil()
        {
            if (nil) {
                nil->~NodeBase();
                auto ptr = reinterpret_cast<typename std::allocator_traits<NodeAllocator>::pointer>(nil);
                std::allocator_traits<NodeAllocator>::deallocate(this->allocator(), ptr, 1);
            }
        }

        Compare & comparator() noexcept { return *static_cast<Compare *>(this); }
        const Compare & comparator() const noexcept { return *static_cast<const Compare *>(this); }
        NodeAllocator & allocator() noexcept { return *static_cast<NodeAllocator *>(this); }
        const NodeAllocator & allocator() const noexcept { return *static_cast<const NodeAllocator *>(this); }
    };

    template <typename K, typename T, bool Const>
    class IteratorTemplate
    {
    protected:
        using tree_type = std::conditional_t<Const, const TreeBase, TreeBase>;
        using node_type = std::conditional_t<Const, const Node<K, T>, Node<K, T>>;
    public:
        using iterator_category = std::bidirectional_iterator_tag;
        using value_type = std::conditional_t<Const, const decltype(node_type::value), decltype(node_type::value)>;
        using reference = value_type &;
        using pointer = value_type *;
        using difference_type = std::ptrdiff_t;
    public:
        IteratorTemplate() = default;
        explicit IteratorTemplate(tree_type & tree, node_type * node) : m_tree(&tree), m_node(node) {}
        IteratorTemplate(const IteratorTemplate<K, T, Const> &) = default;
        IteratorTemplate & operator=(const IteratorTemplate &) = default;

        reference operator*() { return m_node->value; }
        pointer operator->() { return &m_node->value; }

        IteratorTemplate & operator++()
        {
            m_node = static_cast<node_type *>(successor(*m_tree, m_node));
            return *this;
        }
        const IteratorTemplate operator++(int) const { return IteratorTemplate(successor(*m_tree, m_node)); }

        IteratorTemplate & operator--()
        {
            m_node = static_cast<node_type *>(predecessor(*m_tree, m_node));
            return *this;
        }
        const IteratorTemplate operator--(int) const { return IteratorTemplate(predecessor(*m_tree, m_node)); }

        friend bool operator==(const IteratorTemplate & lhs, const IteratorTemplate & rhs)
        {
            return lhs.m_node == rhs.m_node;
        }
        friend bool operator!=(const IteratorTemplate & lhs, const IteratorTemplate & rhs) { return !(lhs == rhs); }

        tree_type & _tree() const noexcept { return *m_tree; }
        node_type * _node() const noexcept { return m_node; }
    private:
        tree_type * m_tree = nullptr;
        node_type * m_node = nullptr;
    };

    template <typename Tree> auto nil(const Tree & tree) { return tree.m_data.nil; }
    template <typename Tree> auto root(const Tree & tree) { return tree.m_data.root; }
}

/****************************************************************************/

template <typename K, typename T,
          typename Compare = std::less<K>,
          typename Allocator = std::allocator<T>>
class RBTree final
{
    using Node = detail::Node<K, T>;
    using NodeAllocator = typename std::allocator_traits<Allocator>::template rebind_alloc<Node>;
    using NodeAllocatorTraits = std::allocator_traits<NodeAllocator>;
    using Data = detail::RBTreeData<K, T, Compare, NodeAllocator>;
public:
    using key_type = K;
    using mapped_type = T;
    using value_type = decltype(Node::value);
    using key_compare = Compare;
    using allocator_type = Allocator;
    using reference = T &;
    using const_reference = T const &;
    using size_type = std::size_t;
    using difference_type = std::ptrdiff_t;
    using iterator = detail::IteratorTemplate<K, T, false>;
    using const_iterator = detail::IteratorTemplate<K, T, true>;

public:
    RBTree()
    {
        m_data.root = m_data.createNil();
    }

    explicit RBTree(const Allocator & alloc)
     : m_data(alloc)
    {
        m_data.root = m_data.createNil();
    }

    explicit RBTree(const Compare & cmp, const Allocator & alloc = Allocator())
     : m_data(cmp, alloc)
    {
        m_data.root = m_data.createNil();
    }

    RBTree(const RBTree & other)
     : m_data(other.m_data.comparator(),
              NodeAllocatorTraits::select_on_container_copy_construction(other.m_data.allocator()))
    {
        m_data.root = m_data.createNil();
        this->insert(other.begin(), other.end());
    }

    RBTree(const RBTree & other, const Allocator & alloc)
     : m_data(other.m_data.comparator(), alloc)
    {
        m_data.root = m_data.createNil();
        this->insert(other.begin(), other.end());
    }

    RBTree(RBTree && other) noexcept
     : m_data(std::move(other.m_data.comparator()),
              std::move(other.m_data.allocator()))
    {
        using std::swap;
        swap(m_data.nil, other.m_data.nil);
        swap(m_data.root, other.m_data.root);
        swap(m_size, other.m_size);
    }

    RBTree(RBTree && other, const Allocator & alloc)
     : m_data(std::move(other.m_data.comparator()), alloc)
    {
        using std::swap;
        if (alloc == other.m_data.allocator()) {
            swap(m_data.nil, other.m_data.nil);
            swap(m_data.root, other.m_data.root);
            swap(m_size, other.m_size);
        } else {
            m_data.root = m_data.createNil();
            this->insert(std::make_move_iterator(other.begin()),
                         std::make_move_iterator(other.end()));
        }
    }

    template<typename Iterator>
    RBTree(Iterator from, Iterator to,
           const Compare & comp = Compare(), const Allocator & alloc = Allocator())
     : m_data(comp, alloc)
    {
        m_data.root = m_data.createNil();
        this->insert(from, to);
    }

    RBTree(std::initializer_list<value_type> values,
           const Compare & comp = Compare(), const Allocator & alloc = Allocator())
     : m_data(comp, alloc)
    {
        m_data.root = m_data.createNil();
        this->insert(values.begin(), values.end());
    }

    ~RBTree() {
        if (m_data.root) { this->clear(); }
        m_data.destroyNil();
    }

    RBTree & operator=(const RBTree & rhs)
    {
        if (&rhs == this) { return *this; }
        this->clear();
        m_data.comparator() = rhs.m_data.comparator();
        if constexpr (NodeAllocatorTraits::propagate_on_container_copy_assignment::value) {
            m_data.destroyNil();
            m_data.allocator() = rhs.m_data.allocator();
            m_data.root = m_data.createNil();
        }
        this->insert(rhs.begin(), rhs.end());   //TODO bisect-based construction
        return *this;
    }

    RBTree & operator=(RBTree && rhs)
    {
        this->clear();
        m_data.comparator() = std::move(rhs.m_data.comparator());
        if constexpr (NodeAllocatorTraits::propagate_on_container_move_assignment::value) {
            m_data.allocator() = std::move(rhs.m_data.allocator());
        } else {
            assert(m_data.allocator() == rhs.m_data.allocator()); // UB
        }
        swap(m_data.nil, rhs.m_data.nil);
        swap(m_data.root, rhs.m_data.root);
        swap(m_size, rhs.m_size);
        return *this;
    }

    [[nodiscard]] key_compare key_comp() const { return m_data.comparator(); }
    [[nodiscard]] allocator_type get_allocator() const { return m_data.allocator(); }

    reference at(const K & key)
    {
        return const_cast<reference>(const_cast<const RBTree *>(this)->at(key));
    }

    const_reference at(const K & key) const
    {
        auto * node = detail::findNode<K, T, Compare>(m_data, key, m_data.comparator());
        if (node != m_data.nil && key == node->value.first) { return node->value.second; }
        throw std::out_of_range("key does not exist");
    }

    reference operator[](const K & key)
    {
        auto * node = detail::findNode<K, T, Compare>(m_data, key, m_data.comparator());
        if (node != m_data.nil && key == node->value.first) { return node->value.second; }

        auto * newNode = buildNode(key);
        newNode->parent = node;
        if (node == m_data.nil) {
            m_data.root = newNode;
        } else {
            if (m_data.comparator()(newNode->value.first, node->value.first)) {
                node->left = newNode;
            } else {
                node->right = newNode;
            }
        }
        insertFixup(m_data, newNode);
        m_size += 1;
        return newNode->value.second;
    }

    reference operator[](K && key)
    {
        auto * node = detail::findNode<K, T, Compare>(m_data, key, m_data.comparator());
        if (node != m_data.nil && key == node->value.first) { return node->value.second; }

        auto * newNode = buildNode(std::move(key));
        newNode->parent = node;
        if (node == m_data.nil) {
            m_data.root = newNode;
        } else {
            if (m_data.comparator()(newNode->value.first, node->value.first)) {
                node->left = newNode;
            } else {
                node->right = newNode;
            }
        }
        insertFixup(m_data, newNode);
        m_size += 1;
        return newNode->value.second;
    }

    void clear()
    {
        auto * node = allLeft(m_data.root);
        while (node != m_data.nil) {
            while (node->right != m_data.nil) { node = allLeft(node->right); }

            detail::NodeBase * leaf;
            do {
                leaf = node;
                node = node->parent;
                destroyNode(static_cast<Node *>(leaf));
            } while (leaf == node->right);
        }
        m_data.root = m_data.nil;
        m_size = 0;
    }

    std::pair<iterator, bool> insert(const value_type & value)
    {
        return this->insert(value_type(value));
    }

    std::pair<iterator, bool> insert(value_type && value)
    {
        auto * node = detail::findNode<K, T, Compare>(m_data, value.first, m_data.comparator());
        if (node != m_data.nil && value.first == node->value.first) {
            return {iterator(m_data, node), false};
        }

        auto * newNode = buildNode(std::forward<value_type>(value));
        newNode->parent = node;
        if (node == m_data.nil) {
            m_data.root = newNode;
        } else {
            if (m_data.comparator()(value.first, node->value.first)) { node->left = newNode; }
                                                                else { node->right = newNode; }
        }
        insertFixup(m_data, newNode);
        m_size += 1;
        return {iterator(m_data, newNode), true};
    }

    void insert(std::initializer_list<value_type> values)
    {
        this->insert(std::begin(values), std::end(values));
    }

    template<typename Iterator> void insert(Iterator first, Iterator last)
    {
        while (first != last) {
            this->insert(*first);
            ++first;
        }
    }

    void erase(iterator it)
    {
        auto * node = it._node();
        extractNode(m_data, node);
        destroyNode(node);
        m_size -= 1;
    }

    void erase(const_iterator it)
    {
        auto * node = const_cast<Node *>(it._node());
        extractNode(m_data, node);
        destroyNode(node);
        m_size -= 1;
    }

    void erase(const K & key)
    {
        auto it = this->find(key);
        if (it != this->end()) { this->erase(it); }
    }

    [[nodiscard]] iterator find(const K & key)
    {
        auto it = const_cast<const RBTree *>(this)->find(key);
        return iterator(const_cast<detail::TreeBase &>(it._tree()), const_cast<Node *>(it._node()));
    }

    [[nodiscard]] const_iterator find(const K & key) const
    {
        if (m_data.root == m_data.nil) { return const_iterator(); }
        auto * node = detail::findNode<K, T, Compare>(m_data, key, m_data.comparator());
        if (node == m_data.nil || key != node->value.first) { return const_iterator(m_data, nullptr); }
        return const_iterator(m_data, node);
    }

    [[nodiscard]] iterator begin() {
        return iterator(m_data, static_cast<Node *>(allLeft(m_data.root)));
    }
    [[nodiscard]] const_iterator begin() const {
        return const_iterator(m_data, static_cast<const Node *>(allLeft(m_data.root)));
    }
    [[nodiscard]] const_iterator cbegin() const {
        return const_iterator(m_data, static_cast<const Node *>(allLeft(m_data.root)));
    }
    [[nodiscard]] iterator end() { return iterator(m_data, nullptr); }
    [[nodiscard]] const_iterator end() const { return const_iterator(m_data, nullptr); }
    [[nodiscard]] const_iterator cend() const { return const_iterator(m_data, nullptr); }

    void swap(RBTree & rhs) noexcept
    {
        using std::swap;
        if constexpr (NodeAllocatorTraits::propagate_on_container_swap::value) {
            swap(m_data.allocator(), rhs.m_data.allocator());
        } else {
            assert(m_data.allocator() == rhs.m_data.allocator());
        }
        swap(m_data.comparator(), rhs.m_data.comparator());
        swap(m_data.nil, rhs.m_data.nil);
        swap(m_data.root, rhs.m_data.root);
        swap(m_size, rhs.m_size);
    }

    [[nodiscard]] size_type size() const noexcept
    {
        return m_size;
    }

    [[nodiscard]] size_type max_size() const noexcept
    {
        return std::numeric_limits<size_type>::max() / sizeof(Node);
    }

    [[nodiscard]] bool empty() const noexcept
    {
        return m_size == 0;
    }

    friend bool operator==(const RBTree & lhs, const RBTree & rhs)
    {
        return lhs.m_data.allocator() == rhs.m_data.allocator() &&
            lhs.m_size == rhs.m_size &&
            std::equal(lhs.cbegin(), lhs.cend(), rhs.cbegin());
    }

    friend bool operator!=(const RBTree & lhs, const RBTree & rhs) { return !(lhs == rhs); }

private:
    template <typename ... Args>
    Node * buildNode(Args && ... args)
    {
        Node * node;
        auto * storage = NodeAllocatorTraits::allocate(m_data.allocator(), 1);
        try {
            node = new (storage) Node(std::forward<Args>(args)...);
        } catch (...) {
            NodeAllocatorTraits::deallocate(m_data.allocator(), storage, 1);
            throw;
        }
        node->left = node->right = m_data.nil;
        return node;
    }

    void destroyNode(Node * node)
    {
        node->~Node();
        NodeAllocatorTraits::deallocate(m_data.allocator(), node, 1);
    }

    friend auto detail::root<RBTree>(const RBTree &); // used by rbtree_tools to access internals

private:
    Data        m_data;
    size_type   m_size = 0;
};

template<typename K, typename T, typename Compare, typename Allocator>
void swap(RBTree<K, T, Compare, Allocator> & lhs, RBTree<K, T, Compare, Allocator> & rhs) { lhs.swap(rhs); }

/****************************************************************************/

} // namespace containers

#endif
