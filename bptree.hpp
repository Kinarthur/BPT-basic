#ifndef BPTREE_HPP_BPTREE2_HPP
#define BPTREE_HPP_BPTREE2_HPP
#include <string>
#include <fstream>
#include "vector.hpp"

using std::string;
using std::fstream;
using std::ifstream;
using std::ofstream;

//the first info is for the head of empty node chain
//the second info is for the number of empty node in the chain
template<class T, int info_len = 2>
class MemoryRiver {
private:
    fstream file;
    string file_name;
    int sizeofT = sizeof(T);
public:
    MemoryRiver() = default;

    MemoryRiver(const string& file_name) : file_name(file_name) {}

    void initialise(string FN = "") {
        if (FN != "") file_name = FN;
        file.open(file_name, std::ios::out);
        int tmp = sizeof(int) * info_len;
        file.write(reinterpret_cast<char*>(&tmp), sizeof(int));
        tmp = 0;
        for (int i = 1; i < info_len; ++i)
            file.write(reinterpret_cast<char*>(&tmp), sizeof(int));
        file.close();
    }

    void get_info(int& tmp, int n) {
        if (n > info_len) return;
        file.open(file_name);
        file.seekg(sizeof(int) * (n - 1));
        file.read(reinterpret_cast<char*>(&tmp), sizeof(int));
        file.close();
    }

    void write_info(int tmp, int n) {
        if (n > info_len) return;
        file.open(file_name);
        file.seekp(sizeof(int) * (n - 1));
        file.write(reinterpret_cast<char*>(&tmp), sizeof(int));
        file.close();
    }

    int write(T& t) {
        file.open(file_name);
        int pos, num, r_index;
        file.seekg(0);
        file.read(reinterpret_cast<char*>(&pos), sizeof(int));
        r_index = pos;
        file.read(reinterpret_cast<char*>(&num), sizeof(int));
        //no empty node exists
        if (num == 0) {
            file.seekp(0);
            pos += sizeofT + sizeof(int);
            file.write(reinterpret_cast<char*>(&pos), sizeof(int));
            file.seekp(pos - sizeofT - sizeof(int));
            file.write(reinterpret_cast<char*>(&num), sizeof(int));
            file.write(reinterpret_cast<char*>(&t), sizeofT);
        }
        else {
            --num;
            file.seekp(pos);
            file.read(reinterpret_cast<char*>(&pos), sizeof(int));
            file.write(reinterpret_cast<char*>(&t), sizeofT);
            file.seekp(0);
            file.write(reinterpret_cast<char*>(&pos), sizeof(int));
            file.write(reinterpret_cast<char*>(&num), sizeof(int));
        }
        file.close();
        return r_index;
    }

    void update(T& t, const int& index) {
        file.open(file_name);
        file.seekp(index + sizeof(int));
        file.write(reinterpret_cast<char*>(&t), sizeofT);
        file.close();
    }

    void read(T& t, const int& index) {
        file.open(file_name);
        file.seekg(index + sizeof(int));
        file.read(reinterpret_cast<char*>(&t), sizeofT);
        file.close();
    }

    void Delete(int index) {
        int a, num;
        file.open(file_name);
        file.read(reinterpret_cast<char*>(&a), sizeof(int));
        file.read(reinterpret_cast<char*>(&num), sizeof(int));
        ++num;
        file.seekg(index);
        file.write(reinterpret_cast<char*>(&a), sizeof(int));
        file.seekp(0);
        file.write(reinterpret_cast<char*>(&index), sizeof(int));
        file.write(reinterpret_cast<char*>(&num), sizeof(int));
        file.close();
    }
};

template<class Key, class T, int M = 100, int L = 100>
class BPTree {
private:
    struct value_array {

        int size,pre,next;
        T val[1010];
        value_array() :size(0),pre(-1),next(-1) {};
        value_array(const T& _val) :size(1),pre(-1),next(-1) {
            val[0] = _val;
        }
        int lower_bound(const T& _val) {
            int l = 0, r = size;
            while (l < r) {
                int mid = (l + r) >> 1;
                if (val[mid] < _val) l = mid + 1;
                else r = mid;
            }
            return r;
        }
        int insert(const T& _val) {
            if (size == 0) {
                size = 1;
                val[0] = _val;
                return 1;
            }
            int pos = lower_bound(_val);
            if(pos == size && size == 1010 && next != -1){
            }
            if (val[pos] == _val) return 0;
            else {
                for (int i = size; i > pos; i--) {
                    val[i] = val[i - 1];
                }
                val[pos] = _val;
                size++;
                return 1;
            }
        }

        int erase(const T& _val) {
            if (size == 0) return 0;
            int pos = lower_bound(_val);
            if (pos == size || val[pos] != _val) return 0;
            for (int i = pos; i < size - 1; i++) {
                val[i] = val[i + 1];
            }
            size--;
            return 1;
        }
    };

    int insert(value_array &val_arr,int address, const T &_val){
        if (val_arr.size == 0) {
            val_arr.size = 1;
            val_arr.val[0] = _val;
            value_memory_river.update(val_arr,address);
            return 1;
        }
        int pos = val_arr.lower_bound(_val);

        if(pos == val_arr.size && val_arr.next != -1){
            value_array tmp;
            value_memory_river.read(tmp, val_arr.next);
            return insert(tmp, val_arr.next,_val);
        }

        if(pos == val_arr.size ){
            if(val_arr.size == 1010){
                value_array tmp;
                tmp.pre = address;
                val_arr.next = value_memory_river.write(tmp);
                value_memory_river.update(val_arr,address);
                return insert(tmp,val_arr.next,_val);
            }else{
                val_arr.val[val_arr.size ++ ] = _val;
                value_memory_river.update(val_arr,address);
                return 1;
            }
        }

        if(val_arr.val[pos] == _val ) return 0;
        else {
            if(val_arr.size == 1010){
                if(val_arr.next == -1){
                    value_array tmp;
                    tmp.pre = address;
                    val_arr.next = value_memory_river.write(tmp);
                    insert(tmp,val_arr.next,val_arr.val[val_arr.size -1]);
                }else{
                    value_array tmp;
                    value_memory_river.read(tmp,val_arr.next);
                    insert(tmp,val_arr.next,val_arr.val[val_arr.size - 1]);
                }

                for (int i = val_arr.size-1; i > pos; i--) {
                    val_arr.val[i] = val_arr.val[i - 1];
                }
                val_arr.val[pos] = _val;
                value_memory_river.update(val_arr,address);
                return 1;
            }else{
                for (int i = val_arr.size; i > pos; i--) {
                    val_arr.val[i] = val_arr.val[i - 1];
                }
                val_arr.val[pos] = _val;
                val_arr.size++;
                value_memory_river.update(val_arr,address);
                return 1;
            }
        }
    }

    int erase(value_array &val_arr, int address, T _val){
        int pos = val_arr.lower_bound(_val);
        if (pos == val_arr.size){
            if(val_arr.next != 0){
                value_array tmp;
                value_memory_river.read(tmp,val_arr.next);
                return erase(tmp,val_arr.next,_val);
            }
            else return 0;
        }
        if(val_arr.val[pos] != _val) return 0;
        for (int i = pos; i < val_arr.size - 1; i++) {
            val_arr.val[i] = val_arr.val[i + 1];
        }
        val_arr.size--;
        if(val_arr.size != 0) {
            value_memory_river.update(val_arr,address);
            return 1;
        }
        else{
            if(val_arr.pre != -1){
                value_array pre;
                value_memory_river.read(pre,val_arr.pre);
                pre.next = val_arr.next;
                value_memory_river.update(pre,val_arr.pre);
                if(val_arr.next != -1){
                    value_array next;
                    value_memory_river.read(next,val_arr.next);
                    next.pre = val_arr.pre;
                    value_memory_river.update(next,val_arr.next);
                }
                value_memory_river.Delete(address);
                return 1;
            }else{
                if(val_arr.next != -1){
                    value_array next;
                    value_memory_river.read(next,val_arr.next);
                    int tmp_next_add = val_arr.next;
                    //val_arr = next;
                    next.pre = -1;
                    value_memory_river.update(next,address);
                    value_memory_river.Delete(tmp_next_add);
                    return 1;
                }else{
                    value_memory_river.Delete(address);
                    return 2;
                }
            }
        }
    }


    struct bpt_node {
        bool isleaf;
        int cnt;
        int left_sibling, right_sibling;
        Key key[M+5];
        int child[M+5];
        bpt_node() :isleaf(1), cnt(0), left_sibling(0), right_sibling(0) { left_sibling = right_sibling = 0; };
        ~bpt_node() {};
        int get_pos(const Key& _key) const {
            int l = 0, r = cnt;
            while (l < r) {
                int mid = (l + r) >> 1;
                if (_key < key[mid]) r =  mid;
                else l = mid + 1;
            }
            return l - 1;
        }
    };

    struct result {
        int flag,ind;
        Key new_key;
        explicit result() :flag(0) {};
        result(int _f, int index, Key k) :flag(_f), ind(index), new_key(k) {};
    };


    std::string  filename;
    bpt_node* root;
    int root_pos;
    int m_size;
    MemoryRiver<bpt_node, 4> node_memory_river;
    MemoryRiver<value_array> value_memory_river;
public:
    explicit BPTree(const std::string& name)
            :node_memory_river(name + "_BPT"), value_memory_river(name + "_data") {
        filename = name;
        m_size = 0;
        fstream file;
        file.open(name + "_BPT");
        if (!file) {
            node_memory_river.initialise();
            value_memory_river.initialise();
            root = new bpt_node();
            root_pos = node_memory_river.write(*root);
            node_memory_river.write_info(root_pos, 4);
        }
        else {
            file.close();
            node_memory_river.get_info(m_size, 3);
            node_memory_river.get_info(root_pos, 4);
            root = new bpt_node();
            node_memory_river.read(*root, root_pos);

        }
    }

    ~BPTree() {
        node_memory_river.update(*root, root_pos);
        node_memory_river.write_info(root_pos, 4);
        node_memory_river.write_info(m_size, 3);
        if (root) delete root;
    };

    int size() { return m_size; }

    std::pair<int, Key> insert(const std::pair<Key, T>& val, bpt_node& node, int node_address) {
        if (node.isleaf) {
            if (node.cnt == 0) {
                node.key[0] = val.first;
                value_array tmp(val.second);
                node.child[0] = value_memory_river.write(tmp);
                node.cnt++;
                m_size++;
                node_memory_river.update(node, node_address);
                value_memory_river.update(tmp, node.child[0]); //这句不加，VS里竟然会出错，看着插了，实际没插
                return std::pair<int, Key>(0, val.first);
            }
            else {
                int pos = node.get_pos(val.first);
                if (pos == -1 || node.key[pos] != val.first) {
                    for (int i = node.cnt - 1; i > pos; i--) {
                        node.key[i + 1] = node.key[i];
                        node.child[i + 1] = node.child[i];
                    }
                    node.cnt++;
                    node.key[pos + 1] = val.first;
                    value_array temp(val.second);
                    int temp1 = value_memory_river.write(temp);
                    node.child[pos + 1] = temp1;
                    m_size++;
                    if (node.cnt != L) {
                        node_memory_river.update(node, node_address);
                        value_memory_river.update(temp, node.child[pos + 1]);
                        return std::pair<int, Key>(0, val.first);
                    }
                    else {
                        bpt_node new_node;
                        new_node.isleaf = 1;
                        new_node.cnt = L >> 1;
                        node.cnt = L - (L >> 1);
                        for (int i = 0; i < (L >> 1); i++) {
                            new_node.key[i] = node.key[L - (L >> 1) + i];
                            new_node.child[i] = node.child[L - (L >> 1) + i];
                        }

                        new_node.left_sibling = node_address;
                        new_node.right_sibling = node.right_sibling;
                        node.right_sibling = node_memory_river.write(new_node);
                        if (new_node.right_sibling != 0) {
                            bpt_node right_sibling;
                            node_memory_river.read(right_sibling, new_node.right_sibling);
                            right_sibling.left_sibling = node.right_sibling;
                            node_memory_river.update(right_sibling, new_node.right_sibling);
                        }
                        node_memory_river.update(new_node, node.right_sibling); //不知道为啥，加上罢；
                        node_memory_river.update(node, node_address);
                        return std::pair<int, Key>(node.right_sibling, new_node.key[0]);
                    }
                }
                else {
                    int index = node.child[pos];
                    value_array tmp;
                    value_memory_river.read(tmp, index);
                    int flag = insert(tmp,index,val.second);
                    if (flag == 1) {
                        m_size++;
                    }
                    return std::pair<int, Key>(0, val.first);
                }
            }
        }
        else {
            bool flag = 0;
            int index = node.get_pos(val.first);
            if (index == -1) {
                node.key[0] = val.first;
                index = 0;
                flag = 1;
            }
            int child_pos = node.child[index];
            bpt_node son;
            node_memory_river.read(son, child_pos);
            std::pair<int, Key> ret = insert(val, son, child_pos);
            if (ret.first == 0) {
                if (flag) node_memory_river.update(node, node_address);
                return std::pair<int, Key>(0, val.first);
            }
            else {
                for (int i = node.cnt - 1; i > index; i--) {
                    node.key[i + 1] = node.key[i];
                    node.child[i + 1] = node.child[i];
                }
                node.key[index + 1] = ret.second;
                node.child[index + 1] = ret.first;
                node.cnt++;
                if (node.cnt != M) {
                    node_memory_river.update(node, node_address);
                    return std::pair<int, Key>(0, val.first);
                }
                else {
                    bpt_node new_node;
                    new_node.isleaf = 0;
                    new_node.cnt = M >> 1;
                    node.cnt = M - (M >> 1);
                    for (int i = 0; i < (M >> 1); i++) {
                        new_node.key[i] = node.key[M - (M >> 1) + i];
                        new_node.child[i] = node.child[M - (M >> 1) + i];
                    }
                    new_node.left_sibling = node_address;
                    new_node.right_sibling = node.right_sibling;
                    node.right_sibling = node_memory_river.write(new_node);
                    if (new_node.right_sibling != 0) {
                        bpt_node right_sibling;
                        node_memory_river.read(right_sibling, new_node.right_sibling);
                        right_sibling.left_sibling = node.right_sibling;
                        node_memory_river.update(right_sibling, new_node.right_sibling);
                    }
                    //node_memory_river.update(new_node, node.right_sibling);//同不知道为啥，加上再说
                    node_memory_river.update(node, node_address);
                    return std::pair<int, Key>(node.right_sibling, new_node.key[0]);
                }
            }
        }
    }

    void insert(const std::pair<Key, T>& val) {
        std::pair<int, Key> ret = insert(val, *root, root_pos);
        if (ret.first == 0) return;

        node_memory_river.update(*root, root_pos);
        bpt_node* new_node = new bpt_node;
        new_node->cnt = 2;
        new_node->isleaf = 0;
        new_node->key[0] = root->key[0];
        new_node->key[1] = ret.second;
        new_node->child[0] = root_pos;
        new_node->child[1] = ret.first;
        int new_pos = node_memory_river.write(*new_node);
        root_pos = new_pos;
        //node_memory_river.update(*new_node, new_pos); //不知道有没有影响，加上再说
        delete root;
        root = new_node;
    }

    sjtu::vector<T> Find(const Key& key, const bpt_node& node) {
        sjtu::vector<T> ret;
        if (m_size == 0 || key < node.key[0]) return ret;
        int index = node.get_pos(key);
        if (node.isleaf == 0) {
            bpt_node child;
            node_memory_river.read(child, node.child[index]);
            return Find(key, child);
        }
        else {
            if (node.key[index] != key) return ret;
            else {
                value_array val;
                int address = node.child[index];
                do {
                    value_memory_river.read(val, address);
                    for (int i = 0; i < val.size; i++) {
                        ret.push_back(val.val[i]);
                    }
                    address = val.next;
                }while(address != -1);
                return ret;
            }
        }
    }
    sjtu::vector<T> Find(const Key& key) {
        return Find(key, *root);
    }

    void remove(const std::pair<Key, T>& val) {
        if (m_size == 0 || val.first < root->key[0]) return;
        remove(val, *root, root_pos, 0, 1);
        if (root->isleaf == 0 && root->cnt == 1) {
            node_memory_river.Delete(root_pos);
            root_pos = root->child[0];
            node_memory_river.read(*root, root_pos);
        }
        return;
    }

    result remove(const std::pair<Key, T>& val, bpt_node& node, int node_pos, int node_ind, int parent_cnt) {
        if (node.isleaf == 1) {
            int index = node.get_pos(val.first);
            if (node.key[index] != val.first) return result();
            value_array value;
            value_memory_river.read(value, node.child[index]);
            int flag = erase(value,node.child[index],val.second);
            if (flag == 1){
                m_size--;
                return result();
            }else if(flag == 0) return result();
            else {
                for (int i = index; i < node.cnt - 1; i++) {
                    node.key[i] = node.key[i + 1];
                    node.child[i] = node.child[i + 1];
                }
                node.cnt--;
                if (node.cnt < (L >> 1)) {
                    bpt_node l_sib, r_sib;
                    l_sib.cnt = r_sib.cnt = 0;
                    if (node_ind > 0) {
                        node_memory_river.read(l_sib, node.left_sibling);
                        if (l_sib.cnt > (L >> 1)) {
                            for (int i = node.cnt; i > 0; i--) {
                                node.key[i] = node.key[i - 1];
                                node.child[i] = node.child[i - 1];
                            }
                            node.key[0] = l_sib.key[l_sib.cnt - 1];
                            node.child[0] = l_sib.child[l_sib.cnt - 1];
                            l_sib.cnt--;
                            node.cnt++;
                            node_memory_river.update(node, node_pos);
                            node_memory_river.update(l_sib, node.left_sibling);
                            return result(1, node_ind, node.key[0]);
                        }
                    }
                    if (node_ind < parent_cnt - 1) {
                        node_memory_river.read(r_sib, node.right_sibling);
                        if (r_sib.cnt > (L >> 1)) {
                            node.key[node.cnt] = r_sib.key[0];
                            node.child[node.cnt] = r_sib.child[0];
                            for (int i = 0; i < r_sib.cnt - 1; i++) {
                                r_sib.key[i] = r_sib.key[i + 1];
                                r_sib.child[i] = r_sib.child[i + 1];
                            }
                            r_sib.cnt--;
                            node.cnt++;
                            node_memory_river.update(node, node_pos);
                            node_memory_river.update(r_sib, node.right_sibling);
                            return result(1, node_ind+1, r_sib.key[0]);
                        }
                    }
                    if (l_sib.cnt > 0) {
                        for (int i = 0; i < node.cnt; i++) {
                            l_sib.key[l_sib.cnt + i] = node.key[i];
                            l_sib.child[l_sib.cnt + i] = node.child[i];
                        }
                        l_sib.cnt += node.cnt;
                        l_sib.right_sibling = node.right_sibling;
                        node_memory_river.update(l_sib, node.left_sibling);
                        if(node.right_sibling !=0){
                            bpt_node tmp;
                            node_memory_river.read(tmp,node.right_sibling);
                            tmp.left_sibling = node.left_sibling;
                            node_memory_river.update(tmp,node.right_sibling);
                        }
                        node_memory_river.Delete(node_pos);
                        return result(2, node_ind, node.key[0]);
                    }
                    if (r_sib.cnt > 0) {
                        for (int i = 0; i < r_sib.cnt; i++) {
                            node.key[node.cnt + i] = r_sib.key[i];
                            node.child[node.cnt + i] = r_sib.child[i];
                        }
                        node.cnt += r_sib.cnt;
                        node_memory_river.Delete(node.right_sibling);
                        node.right_sibling = r_sib.right_sibling;
                        node_memory_river.update(node, node_pos);
                        if(r_sib.right_sibling !=0){
                            bpt_node tmp;
                            node_memory_river.read(tmp,r_sib.right_sibling);
                            tmp.left_sibling = r_sib.left_sibling;
                            node_memory_river.update(tmp,r_sib.right_sibling);
                        }
                        return result(2, node_ind + 1, r_sib.key[0]);
                    }
                    node_memory_river.update(node, node_pos);
                    return index == 0 ? result(1, node_ind, node.key[0]) : result();
                }
                else {
                    node_memory_river.update(node, node_pos);
                    return index == 0 ? result(1, node_ind, node.key[0]) : result();
                }
            }
        }
        else {
            int index = node.get_pos(val.first);
            bpt_node child;
            node_memory_river.read(child, node.child[index]);
            result ret = remove(val, child, node.child[index], index, node.cnt);
            if (ret.flag == 0) return result();
            else if (ret.flag == 1) {
                node.key[ret.ind] = ret.new_key;
                node_memory_river.update(node, node_pos);
                return ret.ind == 0 ? result(1, node_ind, node.key[0]) : result();
            }
            else {
                for (int i = ret.ind; i < node.cnt - 1; i++) {
                    node.key[i] = node.key[i + 1];
                    node.child[i] = node.child[i + 1];
                }
                node.cnt--;
                if (node.cnt >= (M >> 1)) {
                    node_memory_river.update(node, node_pos);
                    return ret.ind == 0 ? result(1, node_ind, node.key[0]) : result();
                }
                else {
                    bpt_node l_sib, r_sib;
                    if (node_ind > 0) {
                        node_memory_river.read(l_sib, node.left_sibling);
                        if (l_sib.cnt > (M >> 1)) {
                            for (int i = node.cnt; i > 0; i--) {
                                node.key[i] = node.key[i - 1];
                                node.child[i] = node.child[i - 1];
                            }
                            node.key[0] = l_sib.key[l_sib.cnt - 1];
                            node.child[0] = l_sib.child[l_sib.cnt - 1];
                            l_sib.cnt--;
                            node.cnt++;
                            node_memory_river.update(node, node_pos);
                            node_memory_river.update(l_sib, node.left_sibling);
                            return result(1, node_ind, node.key[0]);
                        }
                    }
                    if (node_ind < parent_cnt - 1) {
                        node_memory_river.read(r_sib, node.right_sibling);
                        if (r_sib.cnt > (M >> 1)) {
                            node.key[node.cnt] = r_sib.key[0];
                            node.child[node.cnt] = r_sib.child[0];
                            for (int i = 0; i < r_sib.cnt - 1; i++) {
                                r_sib.key[i] = r_sib.key[i + 1];
                                r_sib.child[i] = r_sib.child[i + 1];
                            }
                            r_sib.cnt--;
                            node.cnt++;
                            node_memory_river.update(node, node_pos);
                            node_memory_river.update(r_sib, node.right_sibling);
                            return result(1, node_ind + 1, r_sib.key[0]);
                        }
                    }
                    if (l_sib.cnt > 0) {
                        for (int i = 0; i < node.cnt; i++) {
                            l_sib.key[l_sib.cnt + i] = node.key[i];
                            l_sib.child[l_sib.cnt + i] = node.child[i];
                        }
                        l_sib.cnt += node.cnt;
                        l_sib.right_sibling = node.right_sibling;
                        node_memory_river.update(l_sib, node.left_sibling);
                        node_memory_river.Delete(node_pos);
                        if(node.right_sibling !=0){
                            bpt_node tmp;
                            node_memory_river.read(tmp,node.right_sibling);
                            tmp.left_sibling = node.left_sibling;
                            node_memory_river.update(tmp,node.right_sibling);
                        }
                        return result(2, node_ind, node.key[0]);
                    }
                    if (r_sib.cnt > 0) {
                        for (int i = 0; i < r_sib.cnt; i++) {
                            node.key[node.cnt + i] = r_sib.key[i];
                            node.child[node.cnt + i] = r_sib.child[i];
                        }
                        node.cnt += r_sib.cnt;
                        node_memory_river.Delete(node.right_sibling);
                        node.right_sibling = r_sib.right_sibling;
                        node_memory_river.update(node, node_pos);
                        if(r_sib.right_sibling !=0){
                            bpt_node tmp;
                            node_memory_river.read(tmp,r_sib.right_sibling);
                            tmp.left_sibling = r_sib.left_sibling;
                            node_memory_river.update(tmp,r_sib.right_sibling);
                        }
                        return result(2, node_ind + 1, r_sib.key[0]);
                    }
                    node_memory_river.update(node, node_pos);
                    return ret.ind == 0 ? result(1, node_ind, node.key[0]) : result();
                }
            }
        }
    }

    void clear() {
        delete root;
        root = new bpt_node;
        m_size = 0;
        node_memory_river.initialise();
        node_memory_river.write_info(m_size, 3);
        root_pos = node_memory_river.write(*root);
        value_memory_river.initialise();
    }

    void modify(const std::pair<Key, T>& val, T new_val) {
        if (m_size == 0 || val.first < root->key[0]) return;
        modify(val, new_val, *root);
    }

    void modify(const std::pair<Key, T>& val, T new_val, bpt_node& node) {
        if (node.isleaf == 0) {
            int index = node.get_pos(val.first);
            bpt_node child;
            node_memory_river.read(child, node.child[index]);
            modify(val, new_val, child);
            return;
        }
        else {
            int index = node.get_pos(val.first);
            if (node.key[index] != val.first) return;
            value_array value;
            value_memory_river.read(value, node.child[index]);
            int index2 = value.lower_bound(val.second);
            if (value.val[index2] != val.second) return;
            for (int i = index2; i < value.size - 1; i++) value.val[i] = value.val[i + 1];
            value.size--;
            value.insert(new_val);
            value_memory_river.update(value, node.child[index]);
            return;
        }
    }
    std::pair<bool, T> find(const Key& key) {
        if (m_size == 0 || key < root->key[0]) return std::pair<int, T>(0, T());
        return find(key, *root);
    }
    std::pair<bool, T> find(const Key& key, const bpt_node& node) {
        int index = node.get_pos(key);
        if (node.isleaf == 0) {
            bpt_node child;
            node_memory_river.read(child, node.child[index]);
            return find(key, child);
        }
        else {
            if (node.key[index] != key) return std::pair<int, T>(0, T());
            value_array value;
            value_memory_river.read(value, node.child[index]);
            return std::pair<int, T>(1, value.val[0]);
        }
    }

    //输出节点 方便调试
    void show(){
        show(*root,root_pos);
    }
    void  show(bpt_node &node, int node_address){
        static int sum = 0;
        if(node.isleaf == 1){
            printf("leaf addresss: %d size: %d left_sibling : %d right sibing: %d\n",node_address,node.cnt,node.left_sibling,node.right_sibling);
            for(int i = 0; i< node.cnt;i++) printf("%s ",node.key[i]);
            printf("\n\n");
            sum+= node.cnt;
        }else{
            printf("non-leaf addresss: %d size: %d left_sibling : %d right sibing: %d\n",node_address,node.cnt,node.left_sibling,node.right_sibling);
            for(int i = 0; i< node.cnt;i++) printf("%s ",node.key[i]);
            printf("\n\n");
            for(int i = 0; i< node.cnt;i++){
                bpt_node child;
                node_memory_river.read(child,node.child[i]);
                show(child,node.child[i]);
            }
            if(node_address == root_pos) printf("%d\n",sum);
        }
    }
};

#endif //BPTREE_HPP_BPTREE2_HPP
