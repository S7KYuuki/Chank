#pragma once
#include "BinaryUtils.h"

#include <vector>
#include <ctime>
#include <sys/stat.h>

#define NAME_MAX_LENGTH 25

namespace chank
{
    class Node
    {
    public:
        Node(const int id, const char* name, const bool isDir, const off_t size, const time_t lastModification, Node* parent);
        ~Node();

        /**
         * Adds a new node as child of this one
         * returns: the new node
         */
        Node* AddChild(Node* node);

		Node* FindChild(const char *name) const;
		Node* FindChild(const int id) const;
		Node* UpdateNode(const char *name);
		void RemoveChild(const int id);
		void Save(BinaryOut& file) const;
		static Node* Load(BinaryIn& file);

        /**
         * Sets the current datetime as the lastest modification date of the node
         */
        void UpdateModificationDate();

        // GETTERS | SETTERS
        int GetId() const { return this->id; };
        const char* GetName() const { return this->name; }
		Node* GetParent() const { return this->parent; };
		void SetParent(Node* parent) { this->parent = parent; };
        std::vector<Node*> GetChilds() const { return this->childs; }
        bool IsDir() const { return this->isDir; }
        off_t GetSize() const { return this->size; }
        time_t GetLastModification() const { return this->lastModification; }

    private:
        int id;							// Node unique identifier
        Node* parent;					// Node parent
        char name[NAME_MAX_LENGTH]{};   // Node name
        bool isDir;						// Is directory flag
        std::vector<Node*> childs;		// Nodes that belong to the current one (directory)
        off_t size;						// Node size
        time_t lastModification;		// Last modification date
    };
}
