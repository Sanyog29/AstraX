#include <bits/stdc++.h>
using namespace std;

class node{
public:
int data;
node* next;

node(int value){
    data = value;
    next = nullptr;
    }
};

class ll{
public :
node *head;
void push(int num){
    node* newNode = new node(num);
    if(head == nullptr){
    head = newNode;
    return;    
    }
    node* temp = head;
    while(temp->next != nullptr) temp = temp->next;
    temp->next = newNode;
}

void disp(){
    node* curr = head;
    while(curr != nullptr){
        cout<<curr->data<<" ";
        curr = curr->next;
    }
}
};

int main(){
cout<<"LL Program : "<<endl;
ll* linkedl = new ll();
linkedl->push(1);
linkedl->push(2);
linkedl->push(3);
linkedl->push(4);
linkedl->disp();
cout<<"\nLinkedList Program End";
return 0;
}