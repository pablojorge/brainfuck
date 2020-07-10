package stack

import (
    "errors"
)

type (
    Stack struct {
        top *Node
    }

    Node struct {
        value uint
        prev *Node
    }
)

func New() *Stack {
    return &Stack{nil}
}

func (this *Stack) Pop() (uint, error) {
    if this.top == nil {
        return 0, errors.New("empty stack")
    }

    node := this.top
    this.top = node.prev

    return node.value, nil
}

func (this *Stack) Push(value uint) {
    node := &Node{value,this.top}
    this.top = node
}

