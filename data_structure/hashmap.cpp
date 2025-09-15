#include <iostream>
#include <string>
#include <vector>
#include <list>
#include <functional>
#include <cassert>


template <typename Key, typename Value>
class HashTable {
private:
    struct KeyValuePair{
        Key key;
        Value value;
        KeyValuePair(const Key& k, const Value& v) : key(k), value(v) {}
    };
    using Bucket = std::list<KeyValuePair>;
    std::vector<Bucket> buckets;
    size_t count = 0;//所有元素计数
    double maxLoaderFactor = 0.75;  //最大负载因子

    size_t hashFunction(const Key& key) const {
        return std::hash<Key>{}(key) % buckets.size(); //std::hash<Key<{}相当于创建匿名对象
    }

    void rehash() {
        std::vector<Bucket> newBuckets(buckets.size() * 2);
        for(auto& bucket : buckets) {
            for(auto & kv : bucket){
                size_t index = std::hash<Key>{}(kv.key) % newBuckets.size();
                newBuckets[index].push_back(kv);
            }
        }
        buckets = std::move(newBuckets); // 关键修复：移动新桶到成员变量
    }
public:
    HashTable(size_t initialSize = 16) : buckets(initialSize) {}
    //! 对于相同的键，哈希函数总是返回相同的桶索引​​
    //! 确定性映射​​：同一个键不可能出现在不同的桶中
    void insert(const Key& key, const Value& value) {
        if(static_cast<double>(count + 1) / buckets.size() > maxLoaderFactor) {
            rehash();
            std::cout << "running rehash" << std::endl;
        }
        size_t index = hashFunction(key);
        for(auto& kv : buckets[index]){
            if(kv.key == key){
                kv.value = value;
                return;
            }
        }
        buckets[index].emplace_back(key, value);
        count ++;
    }

    //这里返回Value* 不返回Value是因为避免拷贝；不返回整个kv是为了安全不让用户根据key修改value
    Value* find(const Key& key) {
        size_t index = hashFunction(key);
        for(auto& kv : buckets[index]){
            if(kv.key == key){
                return &kv.value;
            }
        }
        return nullptr;
    }

    bool erase(const Key& key) {
        size_t index = hashFunction(key);
        for(auto it = buckets[index].begin(); it!= buckets[index].end(); ++ it){
            if(it->key == key){
                buckets[index].erase(it);
                count --;
                return true;
            }
        }
        return false;
    }

    void printStats() const {
        std::cout << "Buckets: " << buckets.size() << "\n";
        std::cout << "Elements: " << count << "\n";
        std::cout << "Load Factor: " << static_cast<double>(count) / buckets.size() << "\n";
        
        size_t maxChain = 0;
        size_t emptyBuckets = 0;
        for (const auto& bucket : buckets) {
            if (bucket.empty()) emptyBuckets++;
            if (bucket.size() > maxChain) maxChain = bucket.size();
        }
        
        std::cout << "Longest Chain: " << maxChain << "\n";
        std::cout << "Empty Buckets: " << emptyBuckets << "\n";
    }


};

void testHashMap() {
    std::cout << "===== 开始HashMap测试 =====" << std::endl;
    
    // 测试1: 基本插入和查找
    {
        HashTable<std::string, int> ht;
        ht.insert("apple", 10);
        ht.insert("banana", 20);
        ht.insert("orange", 30);
        
        assert(ht.find("apple") != nullptr);
        assert(*ht.find("apple") == 10);
        assert(ht.find("banana") != nullptr);
        assert(*ht.find("banana") == 20);
        assert(ht.find("orange") != nullptr);
        assert(*ht.find("orange") == 30);
        assert(ht.find("grape") == nullptr);
        
        std::cout << "测试1通过: 基本插入和查找" << std::endl;
    }
    
    // 测试2: 更新值
    {
        HashTable<std::string, int> ht;
        ht.insert("apple", 10);
        ht.insert("apple", 15); // 更新值
        
        assert(*ht.find("apple") == 15);
        std::cout << "测试2通过: 值更新" << std::endl;
    }
    
    // 测试3: 删除操作
    {
        HashTable<std::string, int> ht;
        ht.insert("apple", 10);
        ht.insert("banana", 20);
        
        assert(ht.erase("apple") == true);
        assert(ht.find("apple") == nullptr);
        assert(ht.erase("grape") == false); // 删除不存在的键
        
        std::cout << "测试3通过: 删除操作" << std::endl;
    }
    
    // 测试4: 再哈希和扩容
    {
        HashTable<int, int> ht(4); // 初始桶大小4
        // 插入3个元素 (3/4=0.75，不会触发再哈希)
        ht.insert(1, 100);
        ht.insert(2, 200);
        ht.insert(3, 300);
        
        // 插入第4个元素 (4/4=1.0 > 0.75) 触发再哈希
        ht.insert(4, 400);
        
        // 验证再哈希后数据完整
        assert(*ht.find(1) == 100);
        assert(*ht.find(2) == 200);
        assert(*ht.find(3) == 300);
        assert(*ht.find(4) == 400);
        
        std::cout << "测试4通过: 再哈希和扩容" << std::endl;
        std::cout << "扩容后状态:" << std::endl;
        ht.printStats();
    }
    
    // 测试5: 哈希冲突处理
    {
        // 使用小的桶数组强制产生冲突
        HashTable<int, std::string> ht(2);
        ht.insert(1, "one");
        ht.insert(3, "three"); // 1和3在桶大小为2时会有相同哈希值
        
        assert(*ht.find(1) == "one");
        assert(*ht.find(3) == "three");
        
        std::cout << "测试5通过: 哈希冲突处理" << std::endl;
    }
    
    // 测试6: 性能测试和大数据量
    // {
    //     HashTable<int, int> ht;
    //     const int N = 10000;
        
    //     // 插入大量数据
    //     for (int i = 0; i < N; i++) {
    //         ht.insert(i, i * 10);
    //     }
        
    //     // 验证查找
    //     for (int i = 0; i < N; i++) {
    //         assert(ht.find(i) != nullptr);
    //         assert(*ht.find(i) == i * 10);
    //     }
        
    //     // 验证删除
    //     for (int i = 0; i < N; i += 2) {
    //         assert(ht.erase(i) == true);
    //     }
        
    //     // 验证删除后状态
    //     for (int i = 0; i < N; i++) {
    //         if (i % 2 == 0) {
    //             assert(ht.find(i) == nullptr);
    //         } else {
    //             assert(ht.find(i) != nullptr);
    //             assert(*ht.find(i) == i * 10);
    //         }
    //     }
        
    //     std::cout << "测试6通过: 大数据量测试 (N=" << N << ")" << std::endl;
    //     std::cout << "最终状态:" << std::endl;
    //     ht.printStats();
    // }
    
    // 测试7: 字符串键测试
    {
        HashTable<std::string, std::string> ht;
        ht.insert("name", "Alice");
        ht.insert("job", "Engineer");
        ht.insert("city", "New York");
        
        assert(*ht.find("name") == "Alice");
        assert(*ht.find("job") == "Engineer");
        assert(*ht.find("city") == "New York");
        
        std::cout << "测试7通过: 字符串键测试" << std::endl;
    }
    
    std::cout << "===== 所有测试通过! =====" << std::endl;
}

int main() {
    testHashMap();
    return 0;
}