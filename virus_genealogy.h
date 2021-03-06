#ifndef VIRUS_GENEALOGY_H
#define VIRUS_GENEALOGY_H

#include <vector>
#include <map>
#include <memory>
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

    using VirusPtr = std::shared_ptr<Virus>;

    struct VirusNode {
        using VirusSet = std::set<id_type>;

        VirusPtr virus;

        VirusSet children;

        VirusSet parents;

        VirusNode(id_type const& id) {
            virus = std::make_shared<Virus>(id);
        }

        VirusNode(VirusNode const& other) {
            virus = other.virus;
            children = other.children;
            parents = other.parents;
        }
    };

    using VirusNodePtr = std::shared_ptr<VirusNode>;

    using VirusMap = std::map<id_type, VirusNodePtr>;

    id_type stem_id;

    VirusMap virus_map;

    void remove_node(id_type const& id) {
        VirusNodePtr virus_node = virus_map.at(id);

        for (auto &parent : virus_node->parents) {
            auto virus_parent = virus_map.at(parent);
            virus_parent->children.erase(id);
        }

        for (auto &child : virus_node->children) {
            auto virus_child = virus_map.at(child);
            virus_child->parents.erase(id);

            if (virus_child->parents.empty()) {
                remove_node(child);
            }
        }

        virus_map.erase(id);
    }

    // silna gwarancja
    // nie wprowadza zmian
    VirusMap backup(){
        VirusMap copy_map;

        for (auto& p: virus_map){
            copy_map.insert(std::make_pair(p.first,
                                           std::make_shared<VirusNode>(*p.second)));
        }

        return copy_map;
    }

public:
    VirusGenealogy(VirusGenealogy const& other) = delete;

    VirusGenealogy& operator=(VirusGenealogy const& other) = delete;

    VirusGenealogy& operator=(VirusGenealogy&& other) = delete;

    VirusGenealogy(id_type const& stem_id) : stem_id(stem_id) {
        virus_map.insert(std::make_pair(stem_id, std::make_shared<VirusNode>(stem_id)));
    }

    id_type get_stem_id() const noexcept {
        return stem_id;
    }

    // silna gwarancja
    // nie wprowadza zmian
    std::vector<id_type> get_children(id_type const& id) const {
        if(!exists(id)) {
            throw VirusNotFound();
        }

        VirusNodePtr virus_node = virus_map.at(id);

        return std::vector<id_type>(virus_node->children.begin(), virus_node->children.end());
    }

    // silna gwarancja
    // nie wprowadza zmian
    std::vector<id_type> get_parents(id_type const& id) const {
        if(!exists(id)) {
            throw VirusNotFound();
        }

        VirusNodePtr virus_node = virus_map.at(id);

        return std::vector<id_type>(virus_node->parents.begin(), virus_node->parents.end());
    }

    // silna gwarancja
    // nie wprowadza zmian
    bool exists(id_type const& id) const {
        return virus_map.find(id) != virus_map.end();
    }

    // silna gwarancja
    // nie wprowadza zmian
    Virus& operator[](id_type const& id) const {
        if(!exists(id)) {
            throw VirusNotFound();
        }

        return *virus_map.at(id)->virus;
    }

    // silna gwarancja
    // patrz create(id_type const& id, std::vector<id_type> const& parent_ids)
    void create(id_type const& id, id_type const& parent_id) {
        std::vector<typename Virus::id_type> v;
        v.push_back(parent_id);
        create(id, v);
    }

    // silna gwarancja
    // w przypadku wyjątku, cofa kontener do stanu pierwotnego
    // swap daje silną gwarancje
    void create(id_type const& id, std::vector<id_type> const& parent_ids) {
        if(parent_ids.empty()) {
            throw VirusNotFound();
        }

        if (exists(id)) {
            throw VirusAlreadyCreated();
        }

        for (auto &parent_id : parent_ids) {
            if (!exists(parent_id)) {
                throw VirusNotFound();
            }
        }

        VirusNodePtr new_virus = std::make_shared<VirusNode>(id);
        VirusMap temp_map = backup();

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

    // silna gwarancja
    // w przypadku wyjątku, cofa kontener do stanu pierwotnego
    // swap daje silną gwarancje
    void connect(id_type const& child_id, id_type const& parent_id) {
        if(!exists(child_id) || !exists(parent_id)) {
            throw VirusNotFound();
        }

        VirusNodePtr child = virus_map.at(child_id), parent = virus_map.at(parent_id);
        typename VirusNode::VirusSet temp_map = child->parents;

        try {
            child->parents.insert(parent_id);
            parent->children.insert(child_id);
        } catch(...) {
            child->parents.swap(temp_map);
            throw;
        }
    }

    // silna gwarancja
    // w przypadku wyjątku, cofa kontener do stanu pierwotnego
    // swap daje silną gwarancje
    void remove(id_type const& id) {
        if(!exists(id)) {
            throw VirusNotFound();
        }

        if(id == stem_id) {
            throw TriedToRemoveStemVirus();
        }

        VirusMap temp_map = backup();

        try {
            remove_node(id);
        } catch(...) {
            virus_map.swap(temp_map);
            throw;
        }
    }
};
#endif //VIRUS_GENEALOGY_H