#ifndef VIRUS_GENEALOGY_H
#define VIRUS_GENEALOGY_H

#include <vector>
#include <map>
#include <memory>
#include <algorithm>
#include <iostream>
#include <set>


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

        std::set<typename Virus::id_type> children;

        std::set<typename Virus::id_type> parents;

        VirusNode(typename Virus::id_type const& id): virus(Virus(id)) {}
    };

    typename Virus::id_type stem_id;

    std::map<typename Virus::id_type, std::shared_ptr<VirusNode>> virus_map;
public:
    VirusGenealogy(const VirusGenealogy& other) = delete;
    VirusGenealogy(const VirusGenealogy&& other) = delete;
    VirusGenealogy& operator=(const VirusGenealogy& other) = delete;
    VirusGenealogy& operator=(VirusGenealogy&& other) = delete;

    VirusGenealogy(typename Virus::id_type const& stem_id) : stem_id(stem_id) {
        virus_map.insert(std::make_pair(stem_id, std::make_shared<VirusNode>(stem_id)));
    }

    typename Virus::id_type get_stem_id() const {
        return stem_id;
    }

    std::vector<typename Virus::id_type> get_children(typename Virus::id_type const& id) const {
        if(!exists(id)) {
            throw VirusNotFound();
        }

        auto virus_node = virus_map.at(id);

        return std::vector<typename Virus::id_type>(virus_node->children.begin(), virus_node->children.end());
    }

    std::vector<typename Virus::id_type> get_parents(typename Virus::id_type const& id) const {
        if(!exists(id)) {
            throw VirusNotFound();
        }

        auto virus_node = virus_map.at(id);

        return std::vector<typename Virus::id_type>(virus_node->parents.begin(), virus_node->parents.end());
    }

    bool exists(typename Virus::id_type const& id) const {
        return virus_map.find(id) != virus_map.end();
    }

    Virus& operator[](typename Virus::id_type const& id) const {
        if(!exists(id)) {
            throw VirusNotFound();
        }

        return virus_map.at(id)->virus;
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

        for(auto& parent_id : parent_ids) {
            if(!exists(parent_id)) {
                throw VirusAlreadyCreated();
            }
        }

        auto new_virus = std::make_shared<VirusNode>(id);
        virus_map.insert(std::make_pair(id, new_virus));

        for(auto& parent_id : parent_ids) {
            auto parent = virus_map.at(parent_id);

            new_virus->parents.insert(parent_id);
            parent->children.insert(id);
        }
    }

    void connect(typename Virus::id_type const& child_id, typename Virus::id_type const& parent_id) {
        if(!exists(child_id) || !exists(parent_id)) {
            throw VirusNotFound();
        }

        auto child = virus_map.at(child_id), parent = virus_map.at(parent_id);

        child->parents.insert(parent_id);
        parent->children.insert(child_id);
    }

    void remove(typename Virus::id_type const& id) {
        if(!exists(id)) {
            throw VirusNotFound();
        }

        if(id == stem_id) {
            throw TriedToRemoveStemVirus();
        }

        auto virus_node = virus_map.at(id);
        
        for(auto& parent : virus_node->parents) {
            auto virus_parent = virus_map.at(parent);
            virus_parent->children.erase(id);
        }

        for(auto& child : virus_node->children) {
            auto virus_child = virus_map.at(child);
            virus_child->parents.erase(id);

            if(virus_child->parents.empty()) {
                remove(child);
            }
        }

        virus_map.erase(id);
    }
};
#endif //VIRUS_GENEALOGY_H
