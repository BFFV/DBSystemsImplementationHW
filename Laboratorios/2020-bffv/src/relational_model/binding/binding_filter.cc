#include "binding_filter.h"

#include "relational_model/relational_model.h"
#include "relational_model/graph/relational_graph.h"

using namespace std;

BindingFilter::BindingFilter(Binding& binding, map<string, GraphId>& var2graph_id,
                             map<string, ObjectType>& element_types)
    : binding(binding), var2graph_id(var2graph_id), element_types(element_types) { }


std::string BindingFilter::to_string() const {
    throw std::logic_error("Binding filter only intended to be used by get()");
}


std::shared_ptr<GraphObject> BindingFilter::operator[](const std::string&) {
    throw std::logic_error("Binding filter only intended to be used by get()");
}


std::shared_ptr<GraphObject> BindingFilter::get(const std::string& var, const std::string& key) {
    auto search_var = var + "." + key;
    auto value = binding[search_var];
    if (value != nullptr) {
        return value;
    }
    auto search = cache.find(search_var);
    if (search != cache.end()) {
        return (*search).second;
    }
    else { // no esta en el cache ni el el binding original
        auto graph_id = var2graph_id[var];
        auto element_type = element_types[var];
        auto key_object_id = relational_model.get_string_id(key);
        auto var_value = binding[var];

        unique_ptr<BPlusTree::Iter> it = nullptr;
        if (element_type == ObjectType::node) {
            Node node = static_cast<const Node&>(*var_value);
            it = relational_model.get_node_key_value().get_range(
                Record(node.id, key_object_id, 0),
                Record(node.id, key_object_id, UINT64_MAX)
            );
        }
        else {
            Edge edge = static_cast<const Edge&>(*var_value);
            it = relational_model.get_edge_key_value().get_range(
                Record(edge.id, key_object_id, 0),
                Record(edge.id, key_object_id, UINT64_MAX)
            );
        }

        auto res = it->next();
        if (res != nullptr) {
            auto value_obj_id = ObjectId(res->ids[2]);
            auto res = relational_model.get_graph_object(value_obj_id);
            cache.insert({ search_var, res });
            return res;
        }
        else {
            cache.insert({ search_var, nullptr });
            return nullptr;
        }
    }
}
