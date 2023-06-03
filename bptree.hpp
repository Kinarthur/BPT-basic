#ifndef BPTREE_HPP_BPTREE2_HPP
#define BPTREE_HPP_BPTREE2_HPP
#include <fstream>
#include "memoryriver.hpp"
#include "vector.hpp"
template<class Key, class T, int M = 100, int L = 100>
class BPTree {
private:
    struct val_leaf {
        int size=0;
        T val[400];
        int next=0;
        val_leaf()
        {
            size=0;
            next= 0;
        }
        val_leaf(const T &index)
        {
            val[0]=index;
            size=1;
            next= 0;
        }
        int lower(const T &index)
        {
            if (index>val[size-1]) return size;
            int l=0,r=size-1;
            while (l<r)
            {
                int mid=(l+r)>>1;
                if (val[mid]<index) l=mid+1; else r=mid;
            }
            return l;
        }
        void insert(const T &index)
        {
            int pos=lower(index);
            for (int i=size;i>pos;i--) val[i]=val[i-1];
            val[pos]=index;
            ++size;
        }
        int erase(const T &index)
        {
            int pos=lower(index);
            if (size==pos) return -1;
            if (val[pos]!=index) return -2;
            --size;
            for (int i=pos;i<size;i++) val[i]=val[i+1];
            return 0;
        }
    };
    struct node{
        bool leaf;
        int size,next;
        int child[M+1]={0};
        Key key[M+1];
        node()
        {
            leaf=false;
            size=0;
        }
        int upper(const Key &index)
        {
            if (index>=key[size-1]) return size;
            int l=0,r=size-1;
            while (l<r)
            {
                int mid=(l+r)>>1;
                if (index>=key[mid]) l=mid+1; else r=mid;
            }
            return l;
        }
        int lower(const Key &index)
        {
            if (index>key[size-1]) return size;
            int l=0,r=size-1;
            while (l<r)
            {
                int mid=(l+r)>>1;
                if (index>key[mid]) l=mid+1; else r=mid;
            }
            return l;
        }
    };
    int root;
    int sizee;
    MemoryRiver<node,4> node_river;
    MemoryRiver<val_leaf> val_river;
public:
    explicit BPTree(const std::string &name) {
        sizee=0;
        std::fstream file;
        node_river=name+"node";
        val_river=name+"val";
        file.open(name+"node",std::ios::in);
        if (!file)
        {
            node_river.initialise();
            val_river.initialise();
        }
        else
        {
            node_river.get_info(sizee,4);
            node_river.get_info(root,3);
            file.close();
        }
    }

    ~BPTree() {
        node_river.write_info(sizee,4);
        node_river.write_info(root,3);
    };

    int size() {
        return sizee;
    }
    void vinsert(node &rt,int position, const T &val)
    {
        val_leaf v;
        val_river.read(v,rt.child[position]);
        if (v.val[v.size-1]>val)
        {
            if (v.size<400)
            {
                v.insert(val);
                val_river.update(v,rt.child[position]);
            } else
            {
                val_leaf nv;
                for (int i=200;i<400;++i) nv.val[i-200]=v.val[i];
                nv.size=200;
                v.size=200;
                if (val>v.val[199]) nv.insert(val); else v.insert(val);
                nv.next=v.next;
                v.next=val_river.write(nv);
                val_river.update(v,rt.child[position]);
            }
            return;
        }
        int pos=rt.child[position];
        while (v.next)
        {
            pos=v.next;
            val_river.read(v,pos);
            if (v.val[v.size-1]>val)
            {
                if (v.size<400)
                {
                    v.insert(val);
                    val_river.update(v,pos);
                } else
                {
                    val_leaf nv;
                    for (int i=200;i<400;i++) nv.val[i-200]=v.val[i];
                    nv.size=200;
                    v.size=200;
                    if (val>v.val[199]) nv.insert(val); else v.insert(val);
                    nv.next=v.next;
                    v.next=val_river.write(nv);
                    val_river.update(v,pos);
                }
                return;
            }
        }
        if (v.size<400)
        {
            v.val[v.size++]=val;
            val_river.update(v,pos);
            return;
        }
        val_leaf nv(val);
        v.next=val_river.write(nv);
        val_river.update(v,pos);
    }
    std::pair<int,Key> insert(const std::pair<Key, T> &val,int &position)
    {
        node rt;
        node_river.read(rt,position);
        Key ind=val.first;
        std::pair<int ,Key> a(0,ind);
        int i;
        if (rt.leaf)
        {
            i=rt.lower(val.first);
            if (i<rt.size && ind==rt.key[i]) vinsert(rt,i,val.second);
            else
            {
                for (int j=rt.size;j>i;j--)
                {
                    rt.key[j]=rt.key[j-1];
                    rt.child[j]=rt.child[j-1];
                }
                rt.key[i]=ind;
                val_leaf v(val.second);
                rt.child[i]=val_river.write(v);
                rt.size++;
                sizee++;
                if (rt.size>M)
                {
                    node new_;
                    new_.leaf=true;
                    int mid=rt.size>>1;
                    for (int j=mid;j<=M;j++)
                    {
                        new_.child[j-mid]=rt.child[j];
                        new_.key[j-mid]=rt.key[j];
                    }
                    new_.size=M-mid+1;
                    rt.size=mid;
                    new_.next=rt.next;
                    a.first=node_river.write(new_);
                    rt.next=a.first;
                    a.second=new_.key[0];
                }
            }
            node_river.update(rt,position);
            return a;
        }
        i=rt.upper(ind);
        std::pair<int,Key> neww= insert(val,rt.child[i]);
        if (neww.first==0) return a;
        for (int j=rt.size;j>i;j--)
        {
            rt.key[j]=rt.key[j-1];
            rt.child[j+1]=rt.child[j];
        }
        rt.child[i+1]=neww.first;
        rt.key[i]=neww.second;
        rt.size++;
        if (rt.size==M)
        {
            node new_;
            int mid=(rt.size+1)>>1;
            for (int j=mid;j<M;j++)
            {
                new_.child[j-mid]=rt.child[j];
                new_.key[j-mid]=rt.key[j];
            }
            new_.size=M-mid;
            rt.size=mid-1;
            new_.child[M-mid]=rt.child[M];
            a.first=node_river.write(new_);
            rt.next=a.first;
            node p;
            node_river.read(p,new_.child[0]);
            a.second=p.key[0];
        }
        node_river.update(rt,position);
        return a;
    }
    void insert(const std::pair<Key, T> &val) {
        if (sizee==0)
        {
            node p;
            p.leaf= true;
            p.size=1;
            val_leaf v(val.second);
            p.key[0]=val.first;
            p.child[0]=val_river.write(v);
            root=node_river.write(p);
            sizee++;
            return;
        }
        std::pair<int,Key> f= insert(val,root);
        if (f.first)
        {
            node p;
            p.leaf=false;
            p.size=1;
            p.key[0]=f.second;
            p.child[0]=root;
            p.child[1]=f.first;
            p.next=0;
            root=node_river.write(p);
        }
    }

    sjtu::vector<T> Find(const Key &key) {
        sjtu::vector<T> a;
        if (sizee==0) return a;
        int pos=root;
        while (1)
        {
            node rt;
            node_river.read(rt,pos);
            if(rt.leaf)
            {
                int low=rt.lower(key);
                if (low<rt.size &&rt.key[low]==key)
                {
                    int now=rt.child[low];
                    while (now)
                    {
                        val_leaf p;
                        val_river.read(p,now);
                        for (int j=0;j<p.size;j++) a.push_back(p.val[j]);
                        now=p.next;
                    }
                }
                return a;
            }
            int up=rt.upper(key);
            pos=rt.child[up];
        }
    }

    void deletenode( const std::pair<Key,T> &val,node &f,int &position)
    {
        Key ind=val.first;
        int i=f.upper(ind);
        node son;
        node_river.read(son,f.child[i]);
        if (son.leaf)
        {
            int j=son.lower(ind);
            if (j==son.size ||son.key[j]!=ind) return;
            int now=son.child[j],pre;
            val_leaf prev,s;
            while (now)
            {
                val_river.read(s,now);
                int p=s.erase(val.second);
                if(p==0)
                {
                    if (s.size==0)
                    {
                        val_river.Delete(now);
                        if (now==son.child[j])
                        {
                            son.child[j]=s.next;
                            if (!s.next)
                            {
                                son.size--;
                                sizee--;
                                if (son.size<(L+1)>>1)
                                {
                                    node nextn,pren;
                                    if (i!=f.size) node_river.read(nextn,f.child[i+1]);
                                    if (i!=0) node_river.read(pren,f.child[i-1]);
                                    if (i!=f.size && nextn.size>(L+1)>>1)
                                    {
                                        for (int k=j;k<son.size;k++)
                                        {
                                            son.key[k]=son.key[k+1];
                                            son.child[k]=son.child[k+1];
                                        }
                                        son.key[son.size]=nextn.key[0];
                                        son.child[son.size]=nextn.child[0];
                                        nextn.size--;
                                        for (int k=0;k<nextn.size;k++)
                                        {
                                            nextn.key[k]=nextn.key[k+1];
                                            nextn.child[k]=nextn.child[k+1];
                                        }
                                        son.size++;
                                        f.key[i]=nextn.key[0];
                                        node_river.update(nextn,f.child[i+1]);
                                        node_river.update(son,f.child[i]);
                                    } else if (i!=0 && pren.size>(L+1)>>1)
                                    {
                                        for (int k=j;k>0;k--)
                                        {
                                            son.key[k]=son.key[k-1];
                                            son.child[k]=son.child[k-1];
                                        }
                                        son.key[0]=pren.key[--pren.size];
                                        son.child[0]=pren.child[pren.size];
                                        son.size++;
                                        f.key[i-1]=son.key[0];
                                        node_river.update(pren,f.child[i-1]);
                                        node_river.update(son,f.child[i]);
                                    } else if (i!=f.size)
                                    {
                                        for (int k=j;k<son.size;k++)
                                        {
                                            son.key[k]=son.key[k+1];
                                            son.child[k]=son.child[k+1];
                                        }
                                        for (int k=0;k<nextn.size;k++)
                                        {
                                            son.key[son.size+k]=nextn.key[k];
                                            son.child[son.size+k]=nextn.child[k];
                                        }
                                        son.size+=nextn.size;
                                        node_river.Delete(f.child[i+1]);
                                        f.size--;
                                        f.key[i]=f.key[i+1];
                                        for (int k=i+1;k<f.size;k++)
                                        {
                                            f.key[k]=f.key[k+1];
                                            f.child[k]=f.child[k+1];
                                        }
                                        f.child[f.size]=f.child[f.size+1];
                                        node_river.update(son,f.child[i]);
                                    } else
                                    {
                                        for (int k=0;k<j;k++)
                                        {
                                            pren.key[pren.size+k]=son.key[k];
                                            pren.child[pren.size+k]=son.child[k];
                                        }
                                        for (int k=j;k<son.size;k++)
                                        {
                                            pren.key[pren.size+k]=son.key[k+1];
                                            pren.child[pren.size+k]=son.child[k+1];
                                        }
                                        pren.size+=son.size;
                                        node_river.Delete(f.child[i]);
                                        f.size--;
                                        f.key[i-1]=f.key[i];
                                        for (int k=i;k<f.size;k++)
                                        {
                                            f.key[k]=f.key[k+1];
                                            f.child[k]=f.child[k+1];
                                        }
                                        f.key[f.size]=f.key[f.size+1];
                                        node_river.update(pren,f.child[i-1]);
                                    }
                                    if (position==root) node_river.update(f,position);
                                    return;
                                } else
                                {
                                    for (int k=j;k<son.size;k++)
                                    {
                                        son.key[k]=son.key[k+1];
                                        son.child[k]=son.child[k+1];
                                    }
                                    node_river.update(son,f.child[i]);
                                }
                                return;
                            }
                            node_river.update(son,f.child[i]);
                            return;
                        } else
                        {
                            prev.next=s.next;
                            val_river.update(prev,pre);
                            return;
                        }
                    } else val_river.update(s,now);
                    return;
                } else if (p==-1)
                {
                    pre=now;
                    prev=s;
                    now=s.next;
                } else break;
            }
            return;
        }
        deletenode(val,son,f.child[i]);
        if (position==root)
        {
            if (son.size<((M+1)>>1)-1) {
                node nextn, pren;
                if (i != f.size) node_river.read(nextn, f.child[i + 1]);
                if (i != 0) node_river.read(pren, f.child[i - 1]);
                if (i != f.size && nextn.size > ((M + 1) >> 1)-  1) {
                    node ea;
                    node_river.read(ea, nextn.child[0]);
                    son.key[son.size] = ea.key[0];
                    son.child[++son.size] = nextn.child[0];
                    f.key[i] = nextn.key[0];
                    nextn.size--;
                    for (int k = 0; k < nextn.size; k++) {
                        nextn.key[k] = nextn.key[k + 1];
                        nextn.child[k] = nextn.child[k + 1];
                    }
                    nextn.child[nextn.size] = nextn.child[nextn.size + 1];
                    node_river.update(nextn, f.child[i + 1]);
                    node_river.update(son, f.child[i]);
                } else if (i != 0 && pren.size > ((M + 1) >> 1 )- 1) {
                    node ea;
                    node_river.read(ea, son.child[0]);
                    son.child[son.size + 1] = son.child[son.size];
                    for (int k = son.size++; k > 0; k--) {
                        son.key[k] = son.key[k - 1];
                        son.child[k] = son.child[k - 1];
                    }
                    son.key[0] = ea.key[0];
                    son.child[0] = pren.child[pren.size];
                    f.key[i - 1] = pren.key[pren.size--];
                    node_river.update(pren, f.child[i - 1]);
                    node_river.update(son, f.child[i]);
                } else if (i != f.size) {
                    node ea;
                    node_river.read(ea, nextn.child[0]);
                    son.key[son.size++] = ea.key[0];
                    for (int k = 0; k < nextn.size; k++) {
                        son.key[son.size + k] = nextn.key[k];
                        son.child[son.size + k] = nextn.child[k];
                    }
                    son.size += nextn.size;
                    son.child[son.size] = nextn.child[nextn.size];
                    node_river.Delete(f.child[i + 1]);
                    f.size--;
                    f.key[i] = f.key[i + 1];
                    for (int k = i + 1; k < f.size; k++) {
                        f.key[k] = f.key[k + 1];
                        f.child[k] = f.child[k + 1];
                    }
                    f.child[f.size] = f.child[f.size + 1];
                    node_river.update(son, f.child[i]);
                } else {
                    node ea;
                    node_river.read(ea, son.child[0]);
                    pren.key[pren.size++] = ea.key[0];
                    for (int k = 0; k < son.size; k++) {
                        pren.key[pren.size + k] = son.key[k];
                        pren.child[pren.size + k] = son.child[k];
                    }
                    pren.size += son.size;
                    pren.child[pren.size] = son.child[son.size];
                    node_river.Delete(f.child[i]);
                    f.size--;
                    f.key[i - 1] = f.key[i];
                    for (int k = i; k < f.size; k++) {
                        f.key[k] = f.key[k + 1];
                        f.child[k] = f.child[k + 1];
                    }
                    f.key[f.size] = f.key[f.size + 1];
                    node_river.update(pren, f.child[i - 1]);
                }
                node_river.update(f, position);
                return;
            }
            node_river.update(son,f.child[i]);
            return;
        }
        if (son.size<((M+1)>>1)-1) {
            node nextn, pren;
            if (i != f.size) node_river.read(nextn, f.child[i + 1]);
            if (i != 0) node_river.read(pren, f.child[i - 1]);
            if (i != f.size && nextn.size > ((M + 1) >> 1) - 1) {
                node ea;
                node_river.read(ea, nextn.child[0]);
                son.key[son.size] = ea.key[0];
                son.child[++son.size] = nextn.child[0];
                f.key[i] = nextn.key[0];
                nextn.size--;
                for (int k = 0; k < nextn.size; k++) {
                    nextn.key[k] = nextn.key[k + 1];
                    nextn.child[k] = nextn.child[k + 1];
                }
                nextn.child[nextn.size] = nextn.child[nextn.size + 1];
                node_river.update(nextn, f.child[i + 1]);
                node_river.update(son, f.child[i]);
            } else if (i != 0 && pren.size > ((M + 1) >> 1) - 1) {
                node ea;
                node_river.read(ea, son.child[0]);
                son.child[son.size + 1] = son.child[son.size];
                for (int k = son.size++; k > 0; k--) {
                    son.key[k] = son.key[k - 1];
                    son.child[k] = son.child[k - 1];
                }
                son.key[0] = ea.key[0];
                son.child[0] = pren.child[pren.size];
                f.key[i - 1] = pren.key[pren.size--];
                node_river.update(pren, f.child[i - 1]);
                node_river.update(son, f.child[i]);
            } else if (i != f.size) {
                node ea;
                node_river.read(ea, nextn.child[0]);
                son.key[son.size++] = ea.key[0];
                for (int k = 0; k < nextn.size; k++) {
                    son.key[son.size + k] = nextn.key[k];
                    son.child[son.size + k] = nextn.child[k];
                }
                son.size += nextn.size;
                son.child[son.size] = nextn.child[nextn.size];
                node_river.Delete(f.child[i + 1]);
                f.size--;
                f.key[i] = f.key[i + 1];
                for (int k = i + 1; k < f.size; k++) {
                    f.key[k] = f.key[k + 1];
                    f.child[k] = f.child[k + 1];
                }
                f.child[f.size] = f.child[f.size + 1];
                node_river.update(son, f.child[i]);
            } else {
                node ea;
                node_river.read(ea, son.child[0]);
                pren.key[pren.size++] = ea.key[0];
                for (int k = 0; k < son.size; k++) {
                    pren.key[pren.size + k] = son.key[k];
                    pren.child[pren.size + k] = son.child[k];
                }
                pren.size += son.size;
                pren.child[pren.size] = son.child[son.size];
                node_river.Delete(f.child[i]);
                f.size--;
                f.key[i - 1] = f.key[i];
                for (int k = i; k < f.size; k++) {
                    f.key[k] = f.key[k + 1];
                    f.child[k] = f.child[k + 1];
                }
                f.key[f.size] = f.key[f.size + 1];
                node_river.update(pren, f.child[i - 1]);
            }
            return;
        }
        node_river.update(son,f.child[i]);
    }
    void remove(const std::pair<Key, T> &val) {
        if (sizee==0) return;
        node rt;
        node_river.read(rt,root);
        if (rt.leaf)
        {
            int i=rt.lower(val.first);
            if (i==rt.size ||rt.key[i]!=val.first) return;
            int now=rt.child[i],pre;
            val_leaf s,prev;
            while (now)
            {
                val_river.read(s,now);
                int p=s.erase(val.second);
                if (p==0)
                {
                    if (s.size==0)
                    {
                        val_river.Delete(now);
                        if (now==rt.child[i]) {
                            rt.child[i] = s.next;
                            if (!s.next) {
                                rt.size--;
                                sizee--;
                                if (!sizee) {
                                    node_river.initialise();
                                    val_river.initialise();
                                    return;
                                }
                                for (int j = i; j < rt.size; j++) {
                                    rt.key[j] = rt.key[j + 1];
                                    rt.child[j] = rt.child[j + 1];
                                }
                                node_river.update(rt, root);
                                return;
                            }
                            node_river.update(rt, root);
                            return;
                        }else
                        {
                            prev.next=s.next;
                            val_river.update(prev,pre);
                            return;
                        }
                    } else val_river.update(s,now);
                    return;
                } else if (p==-1)
                {
                    pre=now;
                    prev=s;
                    now=s.next;
                }else return;
            }
            return;
        }
        deletenode(val,rt,root);
        if (rt.size==0)
        {
            node_river.Delete(root);
            root=rt.child[0];
        }
    }

    void clear(){
        sizee=0;
        node_river.initialise();
        val_river.initialise();
    }

    void modify(const std::pair<Key, T> &val, T new_val) {
        if (sizee==0) return;
        node rt;
        node_river.read(rt,root);
        while (!rt->leaf)
        {
            int i= rt.lower(val.first);
            if (i==rt.size ||rt.key[i]!=val.first) return;
            node_river.read(rt,rt.child[i]);
        }
        int i=rt.lower(val.first);
        if (i==rt.size ||rt.key[i]!=val.first) return;
        int now=rt.child[i];
        val_leaf s;
        while (now)
        {
            val_river.read(s,now);
            if (val.second>s.val[s.size-1]) now=s.next;
            else
            {
                int pos=s.lower(val.second);
                if (pos==s.size ||s.val[pos]!=val.second) return;
                s.val[pos]=new_val;
                val_river.update(s,now);
                return;
            }
        }
    }

    std::pair<bool, T> find(const Key &key) {
        if (sizee==0) return std::make_pair(false,T());
        node rt;
        node_river.read(rt,root);
        while (!rt.leaf)
        {
            int low=rt.lower(key);
            if (low==rt.size || rt.key[low]!=key) return std::make_pair(false,T());
            node_river.read(rt,rt.child[low]);
        }
        int low=rt.lower(key);
        if (low==rt.size || rt.key[low]!=key) return std::make_pair(false,T());
        int now=rt.child[low];
        val_leaf s;
        val_river.read(s,now);
        return std::make_pair(true,s.val[0]);
    }

    void show() {
        std::cout<<sizee<<std::endl;
        printf("sum: %d\n",show_node(root));
    }

    int show_node(int pos){
        static int ans = 0;
        node rt;
        node_river.read(rt,pos);

        if(rt.leaf) {
            std::cout << "leaf: \n";
            std::cout << "pos: " << pos << " next: " << rt.next << " now_size: " << rt.size << std::endl;
            for(int i = 0; i < rt.size; i++){
                std::cout << rt.key[i] << " " <<rt.child[i]<<std::endl;
            }
            ans+=rt.size;
            std::cout << std::endl;
            return ans;
        }
        std::cout << "pos: " << pos <<  " next: " << rt.next << " now_size: " << rt.size << std::endl;
        for(int i = 0; i < rt.size; i++){
            std::cout << rt.key[i] << " " ;
        }
        std::cout << std::endl;
        for(int i = 0; i <= rt.size; i++){
            if(rt.child[i]) show_node(rt.child[i]);
        }
        return ans;
    }
};

#endif //BPTREE_HPP_BPTREE2_HPP
