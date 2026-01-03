#include <iostream>
#include <vector>
#include <algorithm>    

using namespace std;

// heapsize当前堆的有效大小，堆在数组中占用的前heapsize个元素
// i表示以i为跟 要下沉的根节点索引
// 升序 最大堆
void heapify(vector<int>& arr, int heapSize, int i)
{
    int largest = i;
    int l = 2 * i + 1;
    int r = 2 * i + 2;
    if(l < heapSize && arr[l] > arr[largest]) largest = l;
    if(r < heapSize && arr[r] > arr[largest]) largest = r;
    if(largest != i){
        swap(arr[largest], arr[i]);
        heapify(arr, heapSize, largest);
    }
}

void heap_sort(vector<int>& arr)
{
    int n = arr.size();
    for(int i = n / 2 - 1; i >= 0; i --){
        heapify(arr, n, i);
    }
    for(int i = n - 1; i > 0; i --){
        swap(arr[0], arr[i]);
        heapify(arr, i, 0);
    }

}

int main()
{
    vector<int> arr{4, 10, 3, 5, 1, 8, 7, 2, 6};
    heap_sort(arr);
    cout << "升序堆排序" << endl;
    for(int v : arr) cout << v << " " ;
    cout << endl;
    return 0;

}