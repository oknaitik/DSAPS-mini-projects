#include <iostream>
#include <random>
#include <climits>
using namespace std;

class MyClass {
    // this is just a demo implementation; 
    // should be modified
public:
    int var1;
    int var2;

    int comp(const MyClass &obj) const {
        if (var1 > obj.var1) {
            return 1;
        }
        else if (var1 == obj.var1) {
            return 0;
        }
        else {
            return -1;
        }
    }
};

template <typename T>
class Node
{
public:
    T data;
    Node<T> **next; // array of pointers to the next nodes at various levels

    Node(T d, int n)
    {
        data = d;
        next = new Node<T> *[n + 1]; // default value NULL
    }
};

template <typename T>
class SkipList
{
private:
    int MAX_LEVEL;
    int curr_level;
    Node<T> *heads; // pointer to the node whose each level next points to the
                    // 1st node of the linked list; data is zero

public:
    SkipList()
    {
        MAX_LEVEL = 16; // assuming number of elements <= 2^16
        curr_level = 0;
        heads = new Node<T>(-1, MAX_LEVEL);
    }

    int randomLevel()
    {
        int level = 0;
        /*
        random_device rd;  // A random seed source
        mt19937 gen(rd()); // Mersenne Twister 19937 generator
        uniform_real_distribution<double> dis(0, 1);
        while (dis(gen) <= 0.5 && level < MAX_LEVEL) {
            level ++;
        }
        */
        while (rand() % 2 == 0 && level < MAX_LEVEL)
        {
            level++;
        }
        return level;
    }

    void insert(T el)
    {
        Node<T> **update = new Node<T> *[MAX_LEVEL + 1];
        Node<T> *ptr = heads;

        for (int i = curr_level; i >= 0; i--)
        {
            while (ptr->next[i] != NULL && ptr->next[i]->data < el)
            {
                ptr = ptr->next[i];
            }
            update[i] = ptr; // store pointer to the rightmost node before the insertion point
        }

        if (ptr->next[0] == NULL || ptr->next[0]->data >= el)
        {
            int node_level = randomLevel();
            // cout << "l:" << node_level << "\n";
            if (node_level > curr_level)
            {
                for (int i = curr_level + 1; i <= node_level; i++)
                {
                    update[i] = heads;
                }
                curr_level = node_level;
            }

            Node<T> *new_node = new Node<T>(el, node_level);
            for (int i = 0; i <= node_level; i++)
            {
                new_node->next[i] = update[i]->next[i];
                update[i]->next[i] = new_node;
            }
        }
        delete[] update;
    }

    void remove(T el)
    {
        bool flag = 1;

        while (flag)
        {
            Node<T> **update = new Node<T> *[MAX_LEVEL + 1];
            Node<T> *ptr = heads;

            for (int i = curr_level; i >= 0; i--)
            {
                while (ptr->next[i] != NULL && ptr->next[i]->data < el)
                {
                    ptr = ptr->next[i];
                }
                update[i] = ptr;
            }

            // level-0 pointer
            ptr = ptr->next[0];
            if ((ptr != NULL && ptr->data != el) || ptr == NULL)
            {
                // no element found
                flag = 0;
            }
            if (ptr != NULL && ptr->data == el)
            {
                for (int i = 0; i <= curr_level; i++)
                {
                    if (update[i]->next[i] != ptr)
                    {
                        break;
                    }
                    update[i]->next[i] = ptr->next[i];
                }

                // reduce level, if the node removed was the only node to have level number of levels
                while (curr_level > 0 && heads->next[curr_level] == NULL) {
                    curr_level--;
                }

                if ((ptr->next[0] != NULL && ptr->next[0]->data != el) || ptr->next[0] == NULL) {
                    // no more duplicate elements
                    flag = 0;
                }
                delete[] ptr->next;
            }
            delete[] update;
        }
    }

    bool search(T el) {
        Node<T> *ptr = heads;
        for (int i = curr_level; i >= 0; i--) {
            while (ptr->next[i] != NULL && ptr->next[i]->data < el) {
                ptr = ptr->next[i];
            }
        }

        ptr = ptr->next[0];
        if (ptr != NULL && ptr->data == el) {
            return 1;
        }
        return 0;
    }

    void printList() {
        // for (int i = curr_level; i >= 0; i --) {
        //     Node<T> *ptr = heads;
        //     cout << "Level " << i << ": ";
        //     while (ptr->next[i] != NULL) {
        //         cout << ptr->next[i]->data << " ";
        //         ptr = ptr->next[i];
        //     }
        //     cout << "\n";
        // }
        Node<T> *ptr = heads;
        while (ptr->next[0] != NULL) {
            cout << ptr->next[0]->data << " ";
            ptr = ptr->next[0];
        }
        cout << "\n";
    }

    int count_occurence(T el) {
        int cnt = 0;
        Node<T> *ptr = heads;
        for (int i = curr_level; i >= 0; i--) {
            while (ptr->next[i] != NULL && ptr->next[i]->data < el) {
                ptr = ptr->next[i];
            }
        }

        ptr = ptr->next[0];
        while (ptr != NULL && ptr->data == el) {
            cnt++;
            ptr = ptr->next[0];
        }
        return cnt;
    }

    T lower_bound(T el) {
        Node<T> *ptr = heads;
        for (int i = curr_level; i >= 0; i--) {
            while (ptr->next[i] != NULL && ptr->next[i]->data < el) {
                ptr = ptr->next[i];
            }
        }

        ptr = ptr->next[0];
        if (ptr != NULL) {
            return ptr->data;
        }
        return T();
    }

    T upper_bound(T el)
    {
        Node<T> *ptr = heads;
        for (int i = curr_level; i >= 0; i--) {
            while (ptr->next[i] != NULL && ptr->next[i]->data < el) {
                ptr = ptr->next[i];
            }
        }

        ptr = ptr->next[0];
        while (ptr != NULL) {
            if (ptr->data > el) {
                return ptr->data;
            }
            else {
                ptr = ptr->next[0];
            }
        }
        return T();
    }

    // disable when data type is non-numeric
    T closest_element(T el) {
        Node<T> *ptr = heads;
        T min_diff;
        T closest_el;
        for (int i = curr_level; i >= 0; i--) {
            while (ptr->next[i] != NULL && ptr->next[i]->data < el) {
                // with each node, the difference keeps reducing anyways
                min_diff = el - ptr->next[i]->data;
                closest_el = ptr->next[i]->data;
                ptr = ptr->next[i];
            }
        }

        // ptr = ptr->next[0];
        if (ptr->next[0] == NULL) {
            if (ptr == heads) {
                // list is empty
                return T();
            }
            // reached the last element of level-0 linked list
            return ptr->data;
        }
        ptr = ptr->next[0];
        if (ptr != NULL)
        {
            if (ptr == heads->next[0]) {
                // el is less than the smallest element
                return ptr->data;
            }
            if (ptr->data > el) {
                if (ptr->data - el < min_diff) {
                    return ptr->data;
                }
                else {
                    return closest_el;
                }
            }
            else {
                // exact match
                return ptr->data;
            }
        }
        return T();
    }

    void releaseMemory() {
        // release all the nodes by traversing level-0 linked list
        Node<T> *ptr = heads;
        while (ptr != NULL) {
            Node<T> *next_ptr = ptr->next[0];
            delete[] ptr->next;
            ptr = next_ptr;
        }
    }
};

int main()
{
    srand(time(0));
    SkipList<int> sl;

    int choice = -1;
    while (choice)
    {
        cin >> choice;
        int el;
        switch (choice)
        {
            case 1: {
                cin >> el;
                sl.insert(el);
                // sl.printList();
            }
            break;

            case 2: {
                cin >> el;
                sl.remove(el);
                // sl.printList();
            }
            break;

            case 3: {
                cin >> el;
                cout << sl.search(el) << "\n";
                // sl.printList();
            }
            break;

            case 4: {
                cin >> el;
                cout << sl.count_occurence(el) << "\n";
                // sl.printList();
            }
            break;

            case 5: {
                cin >> el;
                cout << sl.lower_bound(el) << "\n";
                // sl.printList();
            }
            break;

            case 6: {
                cin >> el;
                cout << sl.upper_bound(el) << "\n";
                // sl.printList();
            }
            break;

            case 7: {
                // disable when data type is non-numeric
                // if (sl.hasNumericData()) {
                cin >> el;
                cout << sl.closest_element(el) << "\n";
                // sl.printList();
                // }
            }
            break;

            default:
            break;
        }
    }

    sl.releaseMemory();
}