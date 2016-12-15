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
    using id_type = typename Virus::id_type;

    struct VirusNode {
        Virus virus;

        std::set<id_type> children;

        std::set<id_type> parents;

        VirusNode(id_type const& id): virus(Virus(id)) {}
    };

    using VirusNodePtr = std::shared_ptr<VirusNode>;

    using VirusMap = std::map<id_type, VirusNodePtr>;

    id_type stem_id;

    VirusMap virus_map;

public:
    VirusGenealogy(const VirusGenealogy& other) = delete;

    VirusGenealogy& operator=(const VirusGenealogy& other) = delete;

    VirusGenealogy(id_type const& stem_id) : stem_id(stem_id) {
        virus_map.insert(std::make_pair(stem_id, std::make_shared<VirusNode>(stem_id)));
    }

    id_type get_stem_id() const noexcept {
        return stem_id;
    }

    std::vector<id_type> get_children(id_type const& id) const {
        if(!exists(id)) {
            throw VirusNotFound();
        }

        VirusNodePtr virus_node = virus_map.at(id);

        return std::vector<id_type>(virus_node->children.begin(), virus_node->children.end());
    }

    std::vector<id_type> get_parents(id_type const& id) const {
        if(!exists(id)) {
            throw VirusNotFound();
        }

        VirusNodePtr virus_node = virus_map.at(id);

        return std::vector<id_type>(virus_node->parents.begin(), virus_node->parents.end());
    }

    bool exists(id_type const& id) const {
        return virus_map.find(id) != virus_map.end();
    }

    Virus& operator[](id_type const& id) const {
        if(!exists(id)) {
            throw VirusNotFound();
        }

        return virus_map.at(id)->virus;
    }

    void create(id_type const& id, id_type const& parent_id) {
        std::vector<typename Virus::id_type> v;
        v.push_back(parent_id);
        create(id, v);
    }

    void create(id_type const& id, std::vector<id_type> const& parent_ids) {
        if(parent_ids.empty()) {
            throw VirusNotFound();
        }

        if (exists(id)) {
            throw VirusAlreadyCreated();
        }

        for (auto &parent_id : parent_ids) {
            if (!exists(parent_id)) {
                throw VirusAlreadyCreated();
            }
        }

        VirusNodePtr new_virus = std::make_shared<VirusNode>(id);
        VirusMap temp_map = virus_map;

        try {
            virus_map.insert(std::make_pair(id, new_virus));

            for (auto &parent_id : parent_ids) {
                auto parent = virus_map.at(parent_id);

                new_virus->parents.insert(parent_id);
                parent->children.insert(id);
            }
        } catch (...) {
            virus_map.swap(temp_map);
            throw;
        }
    }

    void connect(id_type const& child_id, id_type const& parent_id) {
        if(!exists(child_id) || !exists(parent_id)) {
            throw VirusNotFound();
        }

        VirusNodePtr child = virus_map.at(child_id), parent = virus_map.at(parent_id);
        VirusMap temp_map = virus_map;

        try {
            child->parents.insert(parent_id);
            parent->children.insert(child_id);
        } catch(...) {
            virus_map.swap(temp_map);
            throw;
        }
    }

    void remove(id_type const& id) {
        if(!exists(id)) {
            throw VirusNotFound();
        }

        if(id == stem_id) {
            throw TriedToRemoveStemVirus();
        }

        VirusNodePtr virus_node = virus_map.at(id);
        VirusMap temp_map = virus_map;

        try {
            for (auto &parent : virus_node->parents) {
                auto virus_parent = virus_map.at(parent);
                virus_parent->children.erase(id);
            }

            for (auto &child : virus_node->children) {
                auto virus_child = virus_map.at(child);
                virus_child->parents.erase(id);

                if (virus_child->parents.empty()) {
                    remove(child);
                }
            }

            virus_map.erase(id);
        } catch(...) {
            virus_map.swap(temp_map);
            throw;
        }
    }
};
#endif //VIRUS_GENEALOGY_H
