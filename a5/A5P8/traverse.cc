#include <iostream>
#include <vector>
#include <memory>

class Node {
	public:
		int value;
		std::vector<std::unique_ptr<Node>> children;

		Node(int v) : value{v} {}
};

// read pre-order traversal of a tree and construct the tree
std::unique_ptr<Node> read() {
	int v, children;
	std::cin >> v >> children;
	auto node = std::make_unique<Node>(v);
	for (int i = 0; i < children; ++i) {
		node->children.push_back(std::move(read()));
	}
	return node;
}

// print out post-order traversal of tree
void print(Node *n) {
	for (std::vector<std::unique_ptr<Node>>::iterator it = n->children.begin(); it != n->children.end(); ++it) {
		print(it->get());
	}
	
	std::cout << n->value << " " << n->children.size() << std::endl;
}

int main() {
	std::unique_ptr<Node> tree = read();
	print(tree.get());
	return 0;
}
