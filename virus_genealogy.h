#ifndef VIRUS_GENEALOGY_H
#define VIRUS_GENEALOGY_H

#include <vector>
#include <unordered_map>
#include <memory>
#include <algorithm>
#include <iostream>


class VirusAlreadyCreated : public std::exception {
    const char* what() const throw() {
        return "VirusAlreadyCreated";
    }
};

class VirusNotFound : public std::exception {
    const char* what() const throw() {
        return "VirusNotFound";
    }
};

class TriedToRemoveStemVirus : public std::exception {
    const char* what() const throw() {
        return "TriedToRemoveStemVirus";
    }
};

template <class Virus>
struct VirusGenealogy {
private:
    struct VirusNode {
        Virus virus;

        std::vector<typename Virus::id_type> children;

        std::vector<typename Virus::id_type> parents;

        VirusNode(typename Virus::id_type const& id): virus(Virus(id)) {}
    };

    typename Virus::id_type stem_id;

    std::unordered_map<typename Virus::id_type, std::shared_ptr<VirusNode>> virusMap;
public:
    VirusGenealogy(const VirusGenealogy& other) = delete;
    VirusGenealogy(const VirusGenealogy&& other) = delete;
    VirusGenealogy& operator=(const VirusGenealogy& other) = delete;
    VirusGenealogy& operator=(VirusGenealogy&& other) = delete;

    VirusGenealogy(typename Virus::id_type const& stem_id) : stem_id(stem_id) {
        virusMap.insert(std::make_pair(stem_id, std::make_shared<VirusNode>(stem_id)));
    }

    typename Virus::id_type get_stem_id() const {
        return stem_id;
    }

    std::vector<typename Virus::id_type> get_children(typename Virus::id_type const& id) const {
        if(!exists(id)) {
            throw VirusNotFound();
        }

        return virusMap.at(id)->children;
    }

    std::vector<typename Virus::id_type> get_parents(typename Virus::id_type const& id) const {
        if(!exists(id)) {
            throw VirusNotFound();
        }

        return virusMap.at(id)->parents;
    }

    bool exists(typename Virus::id_type const& id) const {
        return virusMap.find(id) != virusMap.end();
    }

    Virus& operator[](typename Virus::id_type const& id) const {
        auto virusNode = virusMap.find(id);

        if(virusNode == virusMap.end()) {
            throw VirusNotFound();
        }

        return virusNode->second->virus;
    }

    void create(typename Virus::id_type const& id, typename Virus::id_type const& parent_id) {
        std::vector<typename Virus::id_type> v;
        v.push_back(parent_id);
        create(id, v);
    }

    void create(typename Virus::id_type const& id, std::vector<typename Virus::id_type> const& parent_ids) {
        if(exists(id)) {
            throw VirusAlreadyCreated();
        }

        auto virus_node = std::make_pair(id, std::make_shared<VirusNode>(id));
        virusMap.insert(virus_node);

        for(auto& parent : parent_ids) {
            connect(id, parent);
        }
    }

    void connect(typename Virus::id_type const& child_id, typename Virus::id_type const& parent_id) {
        if(!exists(child_id) || !exists(parent_id)) {
            throw VirusNotFound();
        }

        auto child = virusMap.at(child_id), parent = virusMap.at(parent_id);

        child->parents.push_back(parent_id);
        parent->children.push_back(child_id);
    }

    void remove(typename Virus::id_type const& id) {
        if(!exists(id)) {
            throw VirusNotFound();
        }

        if(id == stem_id) {
            throw TriedToRemoveStemVirus();
        }

        auto virus_node = virusMap.at(id);
        for(auto& parent : virus_node->parents) {
            auto virusParent = virusMap.at(parent);
            auto index = std::find(virusParent->children.begin(), virusParent->children.end(), id);
            virusParent->children.erase(index);
        }

        for(auto& child : virus_node->children) {
            auto virusChild = virusMap.at(child);
            auto index = std::find(virusChild->parents.begin(), virusChild->parents.end(), id);
            virusChild->parents.erase(index);

            if(virusChild->parents.empty()) {
                remove(child);
            }
        }

        virusMap.erase(id);
    }
};
#endif //VIRUS_GENEALOGY_H
