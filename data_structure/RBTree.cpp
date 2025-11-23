#include <iostream>
#include <memory>
#include <cassert>

// --------------------------
// 红黑树完整实现（含辅助函数）
// --------------------------
enum class Color { RED, BLACK };

template <typename Key, typename Value>
class RBTree {
private:
    struct Node {
        Key key;
        Value value;
        Color color;
        std::shared_ptr<Node> left;
        std::shared_ptr<Node> right;
        std::shared_ptr<Node> parent;
        
        Node(Key k, Value v, Color c = Color::RED)
            : key(k), value(v), color(c), left(nullptr), right(nullptr), parent(nullptr) {}
    };

    using NodePtr = std::shared_ptr<Node>;
    NodePtr root;
    NodePtr nil; // 哨兵节点

    // 左旋
    void leftRotate(NodePtr x) {
        NodePtr y = x->right;
        x->right = y->left;
        
        if (y->left != nil) {
            y->left->parent = x;
        }
        
        y->parent = x->parent;
        
        if (x->parent == nil) {
            root = y;
        } else if (x == x->parent->left) {
            x->parent->left = y;
        } else {
            x->parent->right = y;
        }
        
        y->left = x;
        x->parent = y;
    }

    // 右旋
    void rightRotate(NodePtr y) {
        NodePtr x = y->left;
        y->left = x->right;
        
        if (x->right != nil) {
            x->right->parent = y;
        }
        
        x->parent = y->parent;
        
        if (y->parent == nil) {
            root = x;
        } else if (y == y->parent->left) {
            y->parent->left = x;
        } else {
            y->parent->right = x;
        }
        
        x->right = y;
        y->parent = x;
    }

    // 插入修复
    void insertFixup(NodePtr z) {
        while (z->parent->color == Color::RED) {
            if (z->parent == z->parent->parent->left) {
                NodePtr y = z->parent->parent->right;
                if (y->color == Color::RED) {
                    z->parent->color = Color::BLACK;
                    y->color = Color::BLACK;
                    z->parent->parent->color = Color::RED;
                    z = z->parent->parent;
                } else {
                    if (z == z->parent->right) {
                        z = z->parent;
                        leftRotate(z);
                    }
                    z->parent->color = Color::BLACK;
                    z->parent->parent->color = Color::RED;
                    rightRotate(z->parent->parent);
                }
            } else {
                NodePtr y = z->parent->parent->left;
                if (y->color == Color::RED) {
                    z->parent->color = Color::BLACK;
                    y->color = Color::BLACK;
                    z->parent->parent->color = Color::RED;
                    z = z->parent->parent;
                } else {
                    if (z == z->parent->left) {
                        z = z->parent;
                        rightRotate(z);
                    }
                    z->parent->color = Color::BLACK;
                    z->parent->parent->color = Color::RED;
                    leftRotate(z->parent->parent);
                }
            }
        }
        root->color = Color::BLACK;
    }

    // 查找节点
    NodePtr searchNode(const Key& key) const {
        NodePtr current = root;
        while (current != nil) {
            if (key == current->key) {
                return current;
            } else if (key < current->key) {
                current = current->left;
            } else {
                current = current->right;
            }
        }
        return nullptr;
    }

    // 删除修复
    void deleteFixup(NodePtr x) {
        while (x != root && x->color == Color::BLACK) {
            if (x == x->parent->left) {
                NodePtr w = x->parent->right;
                if (w->color == Color::RED) {
                    w->color = Color::BLACK;
                    x->parent->color = Color::RED;
                    leftRotate(x->parent);
                    w = x->parent->right;
                }
                if (w->left->color == Color::BLACK && w->right->color == Color::BLACK) {
                    w->color = Color::RED;
                    x = x->parent;
                } else {
                    if (w->right->color == Color::BLACK) {
                        w->left->color = Color::BLACK;
                        w->color = Color::RED;
                        rightRotate(w);
                        w = x->parent->right;
                    }
                    w->color = x->parent->color;
                    x->parent->color = Color::BLACK;
                    w->right->color = Color::BLACK;
                    leftRotate(x->parent);
                    x = root;
                }
            } else {
                NodePtr w = x->parent->left;
                if (w->color == Color::RED) {
                    w->color = Color::BLACK;
                    x->parent->color = Color::RED;
                    rightRotate(x->parent);
                    w = x->parent->left;
                }
                if (w->right->color == Color::BLACK && w->left->color == Color::BLACK) {
                    w->color = Color::RED;
                    x = x->parent;
                } else {
                    if (w->left->color == Color::BLACK) {
                        w->right->color = Color::BLACK;
                        w->color = Color::RED;
                        leftRotate(w);
                        w = x->parent->left;
                    }
                    w->color = x->parent->color;
                    x->parent->color = Color::BLACK;
                    w->left->color = Color::BLACK;
                    rightRotate(x->parent);
                    x = root;
                }
            }
        }
        x->color = Color::BLACK;
    }

    // 移植节点
    void transplant(NodePtr u, NodePtr v) {
        if (u->parent == nil) {
            root = v;
        } else if (u == u->parent->left) {
            u->parent->left = v;
        } else {
            u->parent->right = v;
        }
        v->parent = u->parent;
    }

    // 查找最小节点
    NodePtr minimum(NodePtr node) {
        while (node->left != nil) {
            node = node->left;
        }
        return node;
    }

    // --------------------------
    // 红黑树性质验证辅助函数
    // --------------------------
    int getBlackHeight(NodePtr node) const {
        if (node == nil) return 1;
        int leftBlack = getBlackHeight(node->left);
        int rightBlack = getBlackHeight(node->right);
        if (leftBlack == -1 || rightBlack == -1 || leftBlack != rightBlack) {
            return -1;
        }
        return (node->color == Color::BLACK) ? leftBlack + 1 : leftBlack;
    }

    bool isValidRBTreeHelper(NodePtr node, int& blackHeight) const {
        if (node == nil) {
            blackHeight = 1;
            return true;
        }
        if (node->color != Color::RED && node->color != Color::BLACK) {
            std::cerr << "Error: Node color invalid" << std::endl;
            return false;
        }
        if (node->color == Color::RED) {
            if (node->left->color == Color::RED || node->right->color == Color::RED) {
                std::cerr << "Error: Red node has red child (key=" << node->key << ")" << std::endl;
                return false;
            }
        }
        int leftBlack, rightBlack;
        if (!isValidRBTreeHelper(node->left, leftBlack) || !isValidRBTreeHelper(node->right, rightBlack)) {
            return false;
        }
        if (leftBlack != rightBlack) {
            std::cerr << "Error: Black height inconsistent (key=" << node->key << ", left=" << leftBlack << ", right=" << rightBlack << ")" << std::endl;
            return false;
        }
        blackHeight = (node->color == Color::BLACK) ? leftBlack + 1 : leftBlack;
        return true;
    }

public:
    RBTree() {
        nil = std::make_shared<Node>(Key(), Value(), Color::BLACK);
        root = nil;
    }

    // 插入键值对
    void insert(const Key& key, const Value& value) {
        NodePtr z = std::make_shared<Node>(key, value);
        NodePtr y = nil;
        NodePtr x = root;
        
        while (x != nil) {
            y = x;
            if (z->key < x->key) {
                x = x->left;
            } else {
                x = x->right;
            }
        }
        
        z->parent = y;
        if (y == nil) {
            root = z;
        } else if (z->key < y->key) {
            y->left = z;
        } else {
            y->right = z;
        }
        
        z->left = nil;
        z->right = nil;
        z->color = Color::RED;
        
        insertFixup(z);
    }

    // 查找值
    Value* find(const Key& key) {
        NodePtr node = searchNode(key);
        if (node) {
            return &node->value;
        }
        return nullptr;
    }

    // 删除键
    bool erase(const Key& key) {
        NodePtr z = searchNode(key);
        if (!z) return false;
        
        NodePtr y = z;
        NodePtr x;
        Color yOriginalColor = y->color;
        
        if (z->left == nil) {
            x = z->right;
            transplant(z, z->right);
        } else if (z->right == nil) {
            x = z->left;
            transplant(z, z->left);
        } else {
            y = minimum(z->right);
            yOriginalColor = y->color;
            x = y->right;
            
            if (y->parent == z) {
                x->parent = y;
            } else {
                transplant(y, y->right);
                y->right = z->right;
                y->right->parent = y;
            }
            
            transplant(z, y);
            y->left = z->left;
            y->left->parent = y;
            y->color = z->color;
        }
        
        if (yOriginalColor == Color::BLACK) {
            deleteFixup(x);
        }
        
        return true;
    }

    // 中序遍历打印
    void inorderPrint() const {
        inorderPrint(root);
        std::cout << "\n";
    }

private:
    void inorderPrint(NodePtr node) const {
        if (node == nil) return;
        inorderPrint(node->left);
        std::cout << "(" << node->key << ":" << node->value << ":" 
                  << (node->color == Color::RED ? "R" : "B") << ") ";
        inorderPrint(node->right);
    }

public:
    // 红黑树性质验证接口
    bool isValidRBTree() const {
        if (root == nil) return true;
        if (root->color != Color::BLACK) {
            std::cerr << "Error: Root is not black" << std::endl;
            return false;
        }
        int blackHeight;
        return isValidRBTreeHelper(root, blackHeight);
    }

    // 检查是否为空树
    bool empty() const {
        return root == nil;
    }
};

// --------------------------
// 测试用例实现
// --------------------------
void test1_EmptyTree() {
    std::cout << "===== Test 1: Empty Tree =====" << std::endl;
    RBTree<int, std::string> tree;
    
    // 验证空树性质
    assert(tree.empty() == true);
    assert(tree.isValidRBTree() == true);
    assert(tree.find(10) == nullptr);       // 查找不存在的键
    assert(tree.erase(10) == false);        // 删除不存在的键
    
    std::cout << "Empty tree test passed!\n" << std::endl;
}

void test2_InsertSingleNode() {
    std::cout << "===== Test 2: Insert Single Node =====" << std::endl;
    RBTree<int, std::string> tree;
    
    // 插入单个节点
    tree.insert(10, "apple");
    tree.inorderPrint();  // 预期：(10:apple:B) 
    
    // 验证性质
    assert(tree.empty() == false);
    assert(tree.isValidRBTree() == true);  // 根节点为黑色，性质均满足
    assert(*tree.find(10) == "apple");     // 查找成功
    assert(tree.find(20) == nullptr);      // 查找失败
    
    std::cout << "Single node insert test passed!\n" << std::endl;
}

void test3_InsertMultipleNodes() {
    std::cout << "===== Test 3: Insert Multiple Nodes =====" << std::endl;
    RBTree<int, std::string> tree;
    int keys[] = {10, 20, 5, 15, 25, 3, 7};
    std::string vals[] = {"a", "b", "c", "d", "e", "f", "g"};
    
    // 插入多个节点
    for (int i = 0; i < 7; i++) {
        tree.insert(keys[i], vals[i]);
    }
    
    // 中序遍历（升序，验证BST性质）
    std::cout << "Inorder Print: ";
    tree.inorderPrint();  // 预期：(3:f:R) (5:c:B) (7:g:R) (10:a:B) (15:d:R) (20:b:B) (25:e:R) 
    
    // 验证红黑树性质
    assert(tree.isValidRBTree() == true);
    // 验证查找
    assert(*tree.find(15) == "d");
    assert(*tree.find(3) == "f");
    
    std::cout << "Multiple nodes insert test passed!\n" << std::endl;
}

void test4_InsertSortedData() {
    std::cout << "===== Test 4: Insert Sorted Data (Worst Case) =====" << std::endl;
    RBTree<int, std::string> tree;
    
    // 插入升序数据（易导致BST退化，验证红黑树自平衡）
    for (int i = 1; i <= 5; i++) {
        tree.insert(i, std::string(1, 'a' + i - 1));
    }
    
    std::cout << "Inorder Print: ";
    tree.inorderPrint();  // 升序，且颜色符合红黑树规则
    
    // 核心验证：插入升序数据后仍满足红黑树性质（未退化）
    assert(tree.isValidRBTree() == true);
    
    std::cout << "Sorted data insert test passed!\n" << std::endl;
}

void test5_DeleteLeafNode() {
    std::cout << "===== Test 5: Delete Leaf Node =====" << std::endl;
    RBTree<int, std::string> tree;
    tree.insert(10, "a");
    tree.insert(20, "b");
    tree.insert(5, "c");
    tree.insert(3, "d");  // 叶子节点（红色）
    
    std::cout << "Before Delete: ";
    tree.inorderPrint();  // (3:d:R) (5:c:B) (10:a:B) (20:b:R) 
    
    // 删除叶子节点3（红色）
    assert(tree.erase(3) == true);
    
    std::cout << "After Delete: ";
    tree.inorderPrint();  // (5:c:B) (10:a:B) (20:b:R) 
    assert(tree.isValidRBTree() == true);  // 性质仍满足
    assert(tree.find(3) == nullptr);       // 已删除
    
    std::cout << "Leaf node delete test passed!\n" << std::endl;
}

void test6_DeleteNonLeafNode() {
    std::cout << "===== Test 6: Delete Non-Leaf Node =====" << std::endl;
    RBTree<int, std::string> tree;
    tree.insert(10, "a");
    tree.insert(20, "b");
    tree.insert(5, "c");
    tree.insert(3, "d");
    tree.insert(7, "e");  // 非叶子节点（有左子树nil，右子树nil？不，7是5的右子，叶子节点；换10为非叶子）
    
    // 重新构造：10为根，有左右子树
    RBTree<int, std::string> tree2;
    tree2.insert(10, "a");  // 根（黑）
    tree2.insert(5, "b");   // 左子（红）
    tree2.insert(15, "c");  // 右子（红）
    tree2.insert(3, "d");   // 5的左子（黑）
    tree2.insert(7, "e");   // 5的右子（黑）
    
    std::cout << "Before Delete (key=5): ";
    tree2.inorderPrint();  // (3:d:B) (5:b:R) (7:e:B) (10:a:B) (15:c:R) 
    
    // 删除非叶子节点5（有两个子节点）
    assert(tree2.erase(5) == true);
    
    std::cout << "After Delete (key=5): ";
    tree2.inorderPrint();  // (3:d:B) (7:e:R) (10:a:B) (15:c:R) 
    assert(tree2.isValidRBTree() == true);
    assert(tree2.find(5) == nullptr);
    
    std::cout << "Non-leaf node delete test passed!\n" << std::endl;
}

void test7_DeleteRootNode() {
    std::cout << "===== Test 7: Delete Root Node =====" << std::endl;
    RBTree<int, std::string> tree;
    tree.insert(10, "a");  // 根（黑）
    tree.insert(5, "b");   // 左子（红）
    tree.insert(15, "c");  // 右子（红）
    
    std::cout << "Before Delete (root=10): ";
    tree.inorderPrint();  // (5:b:R) (10:a:B) (15:c:R) 
    
    // 删除根节点10
    assert(tree.erase(10) == true);
    
    std::cout << "After Delete (root=10): ";
    tree.inorderPrint();  // 新根为5或15，且为黑色（例如：(5:b:B) (15:c:R)）
    assert(tree.isValidRBTree() == true);  // 新根必为黑色
    assert(tree.find(10) == nullptr);
    
    std::cout << "Root node delete test passed!\n" << std::endl;
}

void test8_ComprehensiveScenario() {
    std::cout << "===== Test 8: Comprehensive Scenario (Insert-Delete-Find) =====" << std::endl;
    RBTree<std::string, int> tree;  // 键为string，值为int
    
    // 1. 插入
    tree.insert("banana", 20);
    tree.insert("apple", 10);
    tree.insert("cherry", 30);
    tree.insert("date", 40);
    tree.insert("elderberry", 50);
    
    std::cout << "After Insert: ";
    tree.inorderPrint();  // 按string字典序：apple(10) banana(20) cherry(30) date(40) elderberry(50)
    assert(tree.isValidRBTree() == true);
    
    // 2. 查找
    assert(*tree.find("cherry") == 30);
    assert(tree.find("fig") == nullptr);
    
    // 3. 删除中间节点
    assert(tree.erase("cherry") == true);
    std::cout << "After Delete 'cherry': ";
    tree.inorderPrint();  // 移除cherry，其余有序
    assert(tree.isValidRBTree() == true);
    
    // 4. 插入重复键（原代码允许重复插入，会形成右子树）
    tree.insert("banana", 25);  // 键重复，插入到右子树
    std::cout << "After Insert Duplicate 'banana': ";
    tree.inorderPrint();  // banana出现两次（原代码未去重）
    assert(tree.isValidRBTree() == true);
    
    std::cout << "Comprehensive scenario test passed!\n" << std::endl;
}

int main() {
    // 执行所有测试用例
    test1_EmptyTree();
    test2_InsertSingleNode();
    test3_InsertMultipleNodes();
    test4_InsertSortedData();
    test5_DeleteLeafNode();
    test6_DeleteNonLeafNode();
    test7_DeleteRootNode();
    test8_ComprehensiveScenario();

    std::cout << "All tests passed! 🎉" << std::endl;
    return 0;
}